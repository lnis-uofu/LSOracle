/* LSOracle: A learning based Oracle for Logic Synthesis
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
*   collapse_techmap.hpp
*   Based on collapse_mapped.hpp from mockturtle
*   Now no longer meaningfully based on that.
*
*   Author: Scott Temple
*   Reads a collapsed klut mapped network, writes
*   a standard cell mapped network by reading the
*   truth table for each LUT, retrieving the corresponding
*   standard cells from a json database built with external tools
*   (see included NPN_LUT4.json), and creating a new cell in the network
*   for each standard cell in the json database
*
*/


#pragma once

#include <unordered_map>
#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include <kitty/operators.hpp>

namespace oracle
{
template<class NtkDest, class NtkSource>
class collapse_techmap_impl
{
public:
  collapse_techmap_impl( NtkSource const& ntk )
      : ntk( ntk )
  {
  }

  std::tuple <NtkDest, std::unordered_map<int, std::string>>  run()
  {
    NtkDest dest;
    nlohmann::json json_library; //LUT -> verilog

    mockturtle::node_map<mockturtle::signal<NtkDest>, NtkSource> node_to_signal( ntk );
    std::unordered_map <int, std::string> cell_names; //for printing verilog

    int netlistcount = 0;
    
    //regular expressions needed to handle signals between nodes in a cell and format cell_names
    //regex in_match("(\S+)\s+g\d+\(");
    std::regex wire_match("new");
    std::regex g_count("g[0-9]+");
    std::regex gate_inputs("\\.([ABCDY])\\((.+?)\\)");

    std::ifstream library("../../NPN_LUT4.json"); //make this generic once it's working
    library >> json_library; //this will be huge if we go above LUT4.  May need to memoize.

    /* primary inputs */
    ntk.foreach_pi( [&]( auto n ) {
        //not sure if I should handle complemented inputs/outputs here, or at the truth table level.
        //I'm concerned that I'll invert twice. 
        node_to_signal[n] = dest.create_pi();
    } );

    /* nodes */
    ntk.foreach_node( [&]( auto const n ) {
        if ( ntk.is_constant( n ) || ntk.is_pi( n ) ){
             ++netlistcount;
            return;
        }
        auto func = ntk.node_function( n );
        std::string tempstr = kitty::to_hex(std::get<0>(kitty::exact_npn_canonization(func)));
        std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper );
        tempstr.insert(tempstr.begin(), 4 - tempstr.length(), '0');
        std::string json_lookup = fmt::format("out_{}", tempstr);
        std::vector<mockturtle::signal<NtkDest>> cell_children;
        
        ntk.foreach_fanin( n, [&]( auto fanin ) {
          cell_children.push_back( node_to_signal[fanin] );
        } );
        
        //get array of standard cells here from json file
        try{
            std::vector<std::string> node_gates = json_library[json_lookup]["gates"];
            std::unordered_map<std::string, mockturtle::signal<NtkDest>> gate_tmp_outputs;
            for (int i = 0; i < node_gates.size(); i++){
                std::vector<mockturtle::signal<NtkDest>> gate_children;
                const std::sregex_token_iterator end;

                //iterates through input and output statements in the verilog, e.g. .A(a) or .Y(new_n6_)
                //captures the i/o wire name, e.g. A or Y, and whatever is in parentheses
                for (std::sregex_token_iterator it(node_gates.at(i).begin(), node_gates.at(i).end(), gate_inputs); it != end; it++){
                  std::string current_token = *it;
                  std::smatch arg_match;
                  if (std::regex_search(current_token, arg_match, gate_inputs)){
                    if (arg_match.ready()){
                      //handle standard cell inputs
                      if (arg_match[1] == "A" || arg_match[1] == "B" || arg_match[1] == "C" || arg_match[1] == "D"){
                        //if the standard cell input corresponds to one of the inputs from the LUT, get that.
                        //the inputs should always appear in the json file in alphabetical order (A, (optionally B, C, D,) then Y)
                        //so I'm using push_back instead of trying to preserve order.
                        if (arg_match[2] == "a"){
                          gate_children.push_back(cell_children.at(0));
                        } else if (arg_match[2] == "b"){
                          gate_children.push_back(cell_children.at(1));
                        } else if (arg_match[2] == "c"){
                          gate_children.push_back(cell_children.at(2));
                        } else if (arg_match[2] == "d"){
                          gate_children.push_back(cell_children.at(3));
                        } else { //otherwise it's an internal wire which should have been an output from an earlier standard cell
                          try {
                            auto int_wire = gate_tmp_outputs.at(arg_match[2]);
                            gate_children.push_back(int_wire);
                          } catch (const std::exception& e){
                            std::cout << "Techmapping failed.  No signal corresponding to wire name " << arg_match[2] << " in NPN class " << json_lookup <<"\n";
                            std::cout << "Guru Meditation: " << e.what() <<"\n";
                          }
                        }
                      } else if (arg_match[1] == "Y"){ //once we get to the output we have what we need to create the node
                        std::cout << "emplacing Y  ";
                        if (arg_match[2] == "F"){ 
                          //the output of the last standard cell becomes the node_to_signal of the parent LUT so that the fanin of the next LUT is correct
                          node_to_signal[n] = dest.create_node(gate_children, make_truth_table(dest, gate_children, node_gates.at(i).substr(0, 3)));
                          std::cout << "making node " << node_to_signal[n] << " with " << gate_children.size() << " children\n";
                          std::string stcell = node_gates.at(i).substr(0, node_gates.at(i).find(" "));
                          cell_names.insert({netlistcount, stcell});
                          std::cout << "index " << netlistcount << "  " << stcell << "\n\n";
                        } else {
                          //otherwise it goes to an internal map to link standard cells within a LUT
                          mockturtle::signal<NtkDest> LUT_member_node = dest.create_node(gate_children, make_truth_table(dest, gate_children, node_gates.at(i).substr(0, 3)));
                          std::cout << "making node " << dest.get_node(LUT_member_node) << " with " << gate_children.size() << " children\n";
                          gate_tmp_outputs.insert({arg_match[2], LUT_member_node});
                          std::string stcell = node_gates.at(i).substr(0, node_gates.at(i).find(" "));
                          cell_names.insert({netlistcount, stcell});
                          std::cout << "index " << netlistcount<< "  " << stcell << "\n\n";
                        }
                      } else {
                        std::cout << "Error: assignment to standard cell must be A, B, C, D, or Y. Techmapper was passed: " << arg_match[1] << "\n";
                        break;
                      } //done with i/o mapping
                    } else {
                      std::cout << "Internal error during techmapping. Attempted to access regex result when not ready.\n";
                      break;
                    } //arg_match.ready()
                  } else {
                    std::cout << "Bad token in NPN-class json file!\n";
                    break;
                  } //regex_search
                } //regex_token_iterator
                ++netlistcount;
            }
            std::cout << "done with for loop.  netlistcount: " << netlistcount << "\n";
            } catch(nlohmann::json::type_error& er){
                     std::cout << "\n" << er.what() << '\n'<< "guru meditation: " << er.id << std::endl;
                     std::cout << "Perhaps truth table "<<json_lookup<<" is not found in the techmapping library?\n\n";
            } catch (std::out_of_range exc){
              std::cout << "Attempting to create a 4 input standard cell with a fanin less  than 4\n";
            } catch (const std::exception& ex){
              std::cout << "caught other exception: "<< ex.what() << "\n";
            }
          dest.foreach_fanin(n, [&](auto fanin){

              std::cout << "node " << n << " has fanin node " << fanin << "\n";
          });
           
      } ); //foreach node

    //ignoring constants at the moment.  Not sure if this is correct long term.
  
    std::cout<<"POs\n";
    ntk.foreach_po( [&]( auto const& f ) {
       // std::cout << std::to_string(f);
        //V.S. for not handling inversion yet.
        dest.create_po( node_to_signal[f] );
    } );
    
    //test
    //std::cout << "\n\n\n";
    dest.foreach_node( [&]( auto const n ) {
     if (cell_names.find(n) != cell_names.end()){
       dest.foreach_fanin( n, [&]( auto fanin ){
         std::cout << "TWO: node " << n << " has fanin node " << fanin << "\n";
       } );
     }
    });
    return std::tuple<NtkDest, std::unordered_map<int, std::string>> (dest, cell_names);
  }

private:
  NtkSource const& ntk;

  kitty::dynamic_truth_table make_truth_table (NtkSource const& dest, std::vector <mockturtle::signal <NtkDest> > children, std::string func){
    std::vector <kitty::dynamic_truth_table> tt_vec;
    for (auto child : children){
      tt_vec.push_back(dest.node_function(dest.get_node(child)));
    }
    kitty::dynamic_truth_table result;
    if (func == "NOT"){
      result = ~dest.node_function(dest.get_node(children[0]));
    } else if (func == "AND"){
      result = std::accumulate( tt_vec.begin(), tt_vec.end(), tt_vec.at(0), [](kitty::dynamic_truth_table a, kitty::dynamic_truth_table d) { return a & d; });
    } else if (func == "NOR"){
      result = ~(std::accumulate( tt_vec.begin(), tt_vec.end(), tt_vec.at(0), [](kitty::dynamic_truth_table a, kitty::dynamic_truth_table d) { return (a | d); }));
    } else if (func == "NAN"){
      result = ~(std::accumulate( tt_vec.begin(), tt_vec.end(), tt_vec.at(0), [](kitty::dynamic_truth_table a, kitty::dynamic_truth_table d) { return (a & d); }));
    } else if (func == "XOR"){
      result = std::accumulate( tt_vec.begin(), tt_vec.end(), tt_vec.at(0), [](kitty::dynamic_truth_table a, kitty::dynamic_truth_table d) { return a ^ d; });  
    } else if (func == "XNO"){
      result = ~(std::accumulate( tt_vec.begin(), tt_vec.end(), tt_vec.at(0), [](kitty::dynamic_truth_table a, kitty::dynamic_truth_table d) { return a ^ d; }));  
    } else if (func.substr(0,2) == "OR"){
      result = std::accumulate( tt_vec.begin(), tt_vec.end(), tt_vec.at(0), [](kitty::dynamic_truth_table a, kitty::dynamic_truth_table d) { return a | d; });  
    } else if (func == "MAJ"){
      result =  (tt_vec.at(0) & tt_vec.at(1)) | (tt_vec.at(0) & tt_vec.at(2)) | (tt_vec.at(1) & tt_vec.at(2));
    }else {
      result = kitty::dynamic_truth_table(8);
    };
    return result;
  }

};


template<class NtkDest, class NtkSource>
std::tuple<NtkDest, std::unordered_map<int, std::string>> techmap_mapped_network( NtkSource const& ntk )
{
  /*
  static_assert( is_network_type_v<NtkSource>, "NtkSource is not a network type" );
  static_assert( is_network_type_v<NtkDest>, "NtkDest is not a network type" );

  static_assert( has_has_mapping_v<NtkSource>, "NtkSource does not implement the has_mapping method" );
  static_assert( has_num_gates_v<NtkSource>, "NtkSource does not implement the num_gates method" );
  static_assert( has_get_constant_v<NtkSource>, "NtkSource does not implement the get_constant method" );
  static_assert( has_get_node_v<NtkSource>, "NtkSource does not implement the get_node method" );
  static_assert( has_foreach_pi_v<NtkSource>, "NtkSource does not implement the foreach_pi method" );
  static_assert( has_foreach_po_v<NtkSource>, "NtkSource does not implement the foreach_po method" );
  static_assert( has_foreach_node_v<NtkSource>, "NtkSource does not implement the foreach_node method" );
  static_assert( has_foreach_cell_fanin_v<NtkSource>, "NtkSource does not implement the foreach_cell_fanin method" );
  static_assert( has_is_constant_v<NtkSource>, "NtkSource does not implement the is_constant method" );
  static_assert( has_is_pi_v<NtkSource>, "NtkSource does not implement the is_pi method" );
  static_assert( has_is_cell_root_v<NtkSource>, "NtkSource does not implement the is_cell_root method" );
  static_assert( has_cell_function_v<NtkSource>, "NtkSource does not implement the cell_function method" );
  static_assert( has_is_complemented_v<NtkSource>, "NtkSource does not implement the is_complemented method" );

  static_assert( has_get_constant_v<NtkDest>, "NtkDest does not implement the get_constant method" );
  static_assert( has_create_pi_v<NtkDest>, "NtkDest does not implement the create_pi method" );
  static_assert( has_create_node_v<NtkDest>, "NtkDest does not implement the create_node method" );
  static_assert( has_create_not_v<NtkDest>, "NtkDest does not implement the create_not method" );
*/

  collapse_techmap_impl<NtkDest, NtkSource> p( ntk );
  return p.run();
}

} /* namespace mockturtle */
