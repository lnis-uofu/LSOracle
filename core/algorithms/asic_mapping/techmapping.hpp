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
*   techmapping.hpp
*   Author: Scott Temple
*   Reads a collapsed klut mapped network, writes
*   a standard cell mapped network by reading the
*   truth table for each LUT, retrieving the corresponding
*   standard cells from a json database built with external tools
*   (see included NPN_LUT4.json), and creating a new cell in the network
*   for each standard cell in the json database
*   returns a tuple of the klut (now standard cell) network
*   and a map of node number and the string identifying the standard cell
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
    nlohmann::json json_library; //LUT -> verilog database
    mockturtle::node_map<mockturtle::signal<NtkDest>, NtkSource> node_to_signal( ntk ); //i/o for original klut network
    int netlistcount = 0; //node count.  Used as index in cell_names
    int new_count = 0;
    std::unordered_map <int, std::string> cell_names; //which network node is which standard cell.  Returned in tuple for printing.  Doing it this way to avoid changing mockturtle    
    std::regex gate_inputs("\\.([ABCDY123]+)\\((.+?)\\)");  //regex to handle signals
    std::ifstream library("../../NPN_complete_noZero.json"); //make this generic once it's working
    library >> json_library; //this will be huge if we go above LUT4.  May need to memoize.

    /* primary inputs */
    ntk.foreach_pi( [&]( auto n ) {
        node_to_signal[n] = dest.create_pi();
    } );

    /* LUT nodes */
    std::cout << "Rewriting LUTs\n";
    ntk.foreach_node( [&]( auto const n ) {
        if ( ntk.is_constant( n ) || ntk.is_pi( n ) ){
          ++netlistcount;
          return;
        }
        auto func = ntk.node_function( n );
        // std::cout << "LUT Function before NPN canonization: " << kitty::to_hex(func) << "\t";
        auto NPNconfig = kitty::exact_npn_canonization(func);
        std::string tempstr = kitty::to_hex(std::get<0>(NPNconfig));
        std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper );
        std::string json_lookup = fmt::format("out_{}", tempstr);
        std::vector<mockturtle::signal<NtkDest>> cell_children;
        // std::cout << "NPN Class: " << tempstr << "\n";

        ntk.foreach_fanin( n, [&]( auto fanin ) {
          cell_children.push_back( node_to_signal[fanin] );
        } );

        //Handling special cases.  NOT LUTs and Constants
        if (cell_children.size() == 1){
          if (json_lookup == "out_1"){
              int before = dest.size();
              std::vector <mockturtle::signal<NtkDest>> NegVec;
              NegVec.push_back(cell_children.at(0));
              node_to_signal[n] = dest.create_node(NegVec, make_truth_table(dest, NegVec, "INV" ));
              int after = dest.size();
              if (before != after){
                cell_names.insert({netlistcount, "INVx2_ASAP7_75t_R"});
                ++netlistcount;
                ++new_count;
              } else {
                // std::cout << "ERROR: Not placing 1 input function, INVx2_ASAP7_75t_R.  Equivalent already exists.\n";
              }
            } else if (json_lookup == "out_0" || json_lookup == "out_00"){
              node_to_signal[n] = dest.get_constant(false);
            }
          return;
        }

        //input negation
        for (int j = 0; j< cell_children.size(); ++j){
          if ( (std::get<1>(NPNconfig) >> j) & 1){
              int before = dest.size();
              std::vector <mockturtle::signal<NtkDest>> NegVec;
              NegVec.push_back(cell_children.at(j));
              mockturtle::signal<NtkDest> tmpsig = dest.create_node(NegVec, make_truth_table(dest, NegVec, "INVx2_ASAP7_75t_R" ));
              cell_children.at(j) = tmpsig;
              int after = dest.size();
              if (before != after){
                cell_names.insert({netlistcount, "INVx2_ASAP7_75t_R"});
                ++netlistcount;
                ++new_count;
              } else {
                // std::cout << "equivalent node already exists.  Not placing. (input negation)\n";
              }
          }
        }

        //input permutation
        std::vector<mockturtle::signal<NtkDest>> temp_cell_children(cell_children.size());
        for (int j = 0; j < cell_children.size(); ++j){
          int temp_index = std::get<2>(NPNconfig)[j];
          temp_cell_children[j] = cell_children[temp_index];
        }
        cell_children = temp_cell_children;
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
                      //can be input from the LUT (a, b, c, d for LUT4) or internal wire
                      //the inputs should always appear in the json file in alphabetical order (A, (optionally B, C, D,) then Y)
                      //so I'm using push_back instead of trying to preserve order.
                      if (arg_match[1] != "Y"){
                        if (arg_match[2] == "a"){
                          gate_children.push_back(cell_children.at(0));
                        } else if (arg_match[2] == "b"){
                          gate_children.push_back(cell_children.at(1));
                        } else if (arg_match[2] == "c"){
                          gate_children.push_back(cell_children.at(2));
                        } else if (arg_match[2] == "d"){
                          gate_children.push_back(cell_children.at(3));
                        } else if (arg_match[2] == "e"){
                          gate_children.push_back(cell_children.at(4));
                        } else if (arg_match[2] == "f"){
                          gate_children.push_back(cell_children.at(5));
                        } else { //otherwise it's an internal wire which should have been an output from an earlier standard cell
                          try {
                            auto int_wire = gate_tmp_outputs.at(arg_match[2]);
                            gate_children.push_back(int_wire);
                          } catch (const std::exception& e){
                            std::cout << "Techmapping failed.  No signal corresponding to wire name " << arg_match[2] << " in NPN class " << json_lookup <<"\n";
                            std::cout << "Guru Meditation: " << e.what() <<"\n";
                          }
                        }

                      //outputs and populating network
                      } else if (arg_match[1] == "Y"){
                        if (arg_match[2] == "F"){ 
                          //the output of the last standard cell becomes the node_to_signal of the parent LUT so that the fanin of the next LUT is correct
                          //need to check if original function in LUT before NPN canonization had a negated output and if so add a NOT node.
                          int before = dest.size();
                          node_to_signal[n] = dest.create_node(gate_children, make_truth_table(dest, gate_children, node_gates.at(i).substr(0, node_gates.at(i).find(" "))));
                          int after = dest.size();
                          if (before != after){
                            std::string stcell = node_gates.at(i).substr(0, node_gates.at(i).find(" "));
                            cell_names.insert({netlistcount, stcell});
                            ++netlistcount;
                            ++new_count;
                          } else {
                            std::string stcell = node_gates.at(i).substr(0, node_gates.at(i).find(" "));
                            // std::cout << "Equivalent cell already exists at LUT end:\t";
                            // std::cout << "node: " << i << " in parent klut node " << n <<"\n";
                          }
                          if ( ( ( std::get<1>(NPNconfig) >> cell_children.size() ) & 1 )){
                            int before = dest.size();
                            std::vector <mockturtle::signal<NtkDest>> NegVec;
                            NegVec.push_back(node_to_signal[n]);
                            node_to_signal[n] = dest.create_node(NegVec, make_truth_table(dest, NegVec, "INV" ));
                            int after = dest.size();
                            if (before != after){
                              cell_names.insert({netlistcount, "INVx2_ASAP7_75t_R"});
                              ++netlistcount;
                              ++new_count;
                            } else {
                              // std::cout << "Gate equivalent to negated klut output already exists.  Not placing not gate.\n";
                            }
                          }
                        } else {
                          //otherwise it goes to an internal map to link standard cells within a LUT
                          int before = dest.size();
                          mockturtle::signal<NtkDest> LUT_member_node = dest.create_node(gate_children, make_truth_table(dest, gate_children, node_gates.at(i).substr(0, node_gates.at(i).find(" "))));
                          gate_tmp_outputs.insert({arg_match[2], LUT_member_node});
                          int after = dest.size();
                          if (before != after){
                            std::string stcell = node_gates.at(i).substr(0, node_gates.at(i).find(" "));
                            cell_names.insert({netlistcount, stcell});
                            ++netlistcount;
                            ++new_count;
                          } else {
                            // std::cout << "Reusing equivalent cell" << "\n";
                          }
                        }
                      /***************************/
                      //error handling
                      } else {
                        // std::cout << "Error: assignment to standard cell must be A, B, C, D, or Y. Techmapper was passed: " << arg_match[1] << "\n";
                        break;
                      } //done with i/o mapping
                    } else {
                      // std::cout << "Internal error during techmapping. Attempted to access regex result when not ready.\n";
                      break;
                    } //arg_match.ready()
                  } else {
                    // std::cout << "Bad token in NPN-class json file!\n";
                    break;
                  } //regex_search
                } //regex_token_iterator
            } // node_gates for loop
            } catch(nlohmann::json::type_error& er){
                     // std::cout << "\n" << er.what() << '\n'<< "guru meditation: " << er.id << std::endl;
                     // std::cout << "Perhaps truth table "<<json_lookup<<" is not found in the techmapping library?\n\n";
            } catch (std::out_of_range exc){
              // std::cout << "Attempting to create a 4 input standard cell with a fanin less  than 4\n";
            } catch (const std::exception& ex){
              // std::cout << "caught other exception: "<< ex.what() << "\n";
            }
           
      } ); //foreach node

    ntk.foreach_po( [&]( auto const& f ) {
        dest.create_po( node_to_signal[f] );
    } );
    
    // std::cout << "###################\nLOOPING THROUGH DEST NTK (DEBUG)\n\n"; 
    // dest.foreach_node( [&]( auto const& n ) {
    //   if (cell_names.find(n) != cell_names.end()){
    //     std::cout << "\t" << n << "\t" << cell_names.at(n) << " \n";
    //   } else {
    //     std::cout << "\tno cell found for " << n << "\n";
    //   }
    //   });

    int dup_count = 0;

    std::cout << "Performing post techmapping optimization\n";
    dest.foreach_node( [&](auto const& n){
      if ( dest.is_constant( n ) || dest.is_pi( n ) ){
          return;
      }
      auto func = dest.node_function( n );
      std::vector<mockturtle::signal<NtkDest>> check_node_children;
      std::vector<mockturtle::signal<NtkDest>> check_node_grandchildren;
        
      if (kitty::to_hex(func) == "1"){
        dest.foreach_fanin( n, [&]( auto fanin ) {
          check_node_children.push_back(fanin);
        } );
        if (check_node_children.size() == 1){
          if (kitty::to_hex(dest.node_function(check_node_children.at(0))) == "1"){
            dest.foreach_fanin( check_node_children.at(0), [&]( auto fanin2 ) {
              check_node_grandchildren.push_back(fanin2);
            } );
            if (check_node_grandchildren.size() == 1){
              dup_count += 2;
              // std::cout << "Duplicated NOT gates.  Signal: " << n << " child: " << check_node_children.at(0) << " original signal before double inversion: " << check_node_grandchildren.at(0) << "\n";
              if(dest.fanout_size(check_node_children.at(0)) == 1 )
              {
                cell_names.erase(check_node_children.at(0));
              }
              cell_names.erase(n);
              dest.substitute_node(n, check_node_grandchildren.at(0));

              //dest.take_out_node(n); //this is not implemented for KLUT networks
            }
          }
        }
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
    kitty::dynamic_truth_table result (tt_vec.size());
    if (func.substr(0,3) == "INV"){
       kitty::create_from_hex_string(result, "1");
    } else if (func.substr(0,3) == "AND"){
      if (tt_vec.size() == 2)
        kitty::create_from_hex_string(result, "8");
      if (tt_vec.size() == 3)
        kitty::create_from_hex_string(result, "80");
      if (tt_vec.size() == 4)
        kitty::create_from_hex_string(result, "8000");
    } else if (func.substr(0,3) == "NOR"){
      if (tt_vec.size() == 2)
        kitty::create_from_hex_string(result, "1");
      if (tt_vec.size() == 3)
        kitty::create_from_hex_string(result, "01");
      if (tt_vec.size() == 4)
        kitty::create_from_hex_string(result, "0001");
    } else if (func.substr(0,3) == "NAN"){
      if (tt_vec.size() == 2)
        kitty::create_from_hex_string(result, "7");
      if (tt_vec.size() == 3)
        kitty::create_from_hex_string(result, "7F");
      if (tt_vec.size() == 4)
        kitty::create_from_hex_string(result, "7FFF");
    } else if (func.substr(0,3) == "XOR"){
      if (tt_vec.size() == 2)
        kitty::create_from_hex_string(result, "6");
      if (tt_vec.size() == 3)
        kitty::create_from_hex_string(result, "96");
      if (tt_vec.size() == 4)
        kitty::create_from_hex_string(result, "6996");
    } else if (func.substr(0,3) == "XNO"){
      if (tt_vec.size() == 2)
        kitty::create_from_hex_string(result, "9");
      if (tt_vec.size() == 3)
        kitty::create_from_hex_string(result, "69");
      if (tt_vec.size() == 4)
        kitty::create_from_hex_string(result, "9669");
    } else if (func.substr(0,2) == "OR"){
      if (tt_vec.size() == 2)
        kitty::create_from_hex_string(result, "E");
      if (tt_vec.size() == 3)
        kitty::create_from_hex_string(result, "FE");
      if (tt_vec.size() == 4)
        kitty::create_from_hex_string(result, "FFFE");
    } else if (func.substr(0,3) == "MAJ"){
       kitty::create_from_hex_string(result, "E8");
    }else if (func.substr(0,6) == "AOI21x"){
    //  (!A1 * !B) + (!A2 * !B)
      kitty::create_from_chain(result, {"x4 = x1 !| x3", "x5 = x2 !| x3", "x6 = x4 | x5"});
    } else if (func.substr(0,6) == "AOI211"){
    //  (!A1 * !B * !C) + (!A2 * !B * !C)
      //kitty::create_from_expression(result, "{(!a!c!d)(!b!c!d)}");
      kitty::create_from_hex_string(result, "0111");
    } else if (func.substr(0,6) == "AOI31x"){
    //  (!A1 * !B) + (!A2 * !B) + (!A3 * !B)
      kitty::create_from_chain(result, {"x5 = x1 !| x4", "x6 = x2 !| x4", "x7 = x3 !| x4", "x8 = x5 | x6", "x9 = x7 | x8"});
    } else if (func.substr(0,6) == "AOI311"){
    //  (!A1 * !B * !C) + (!A2 * !B * !C) + (!A3 * !B * !C)
      //kitty::create_from_expression(result, "{(!a!d!e)(!b!d!e)(!c!d!e)}");
      kitty::create_from_hex_string(result, "01111111");
    } else if (func.substr(0,6) == "AOI22x"){
    //  (!A1 * !B1) + (!A1 * !B2) + (!A2 * !B1) + (!A2 * !B2)
       kitty::create_from_chain(result, {"x5 = x1 !| x3", "x6 = x1 !| x4", "x7 = x2 !| x3", "x8 = x2 !| x4", "x9 = x5 | x6", "x10 = x7 | x8", "x11 = X9 | x10"});
    } else if (func.substr(0,6) == "AOI221"){
    //  (!A1 * !B1 * !C) + (!A1 * !B2 * !C) + (!A2 * !B1 * !C) + (!A2 * !B2 * !C)
      //kitty::create_from_expression(result, "{(!a!c!e)(!a!d!e)(!b!c!e)(!b!d!e)}");
      kitty::create_from_hex_string(result, "00000111"); //this is a place holder
    } else if (func.substr(0,6) == "OAI21x"){
      //inverse of AOI
      kitty::create_from_chain(result, {"x4 = x1 !| x3", "x5 = x2 !| x3", "x6 = x4 !| x5"});
    } else if (func.substr(0,6) == "OAI211"){
      kitty::create_from_hex_string(result, "1000");
    } else if (func.substr(0,6) == "OAI22x"){
       kitty::create_from_chain(result, {"x5 = x1 !| x3", "x6 = x1 !| x4", "x7 = x2 !| x3", "x8 = x2 !| x4", "x9 = x5 | x6", "x10 = x7 | x8", "x11 = X9 !| x10"});
    } else if (func.substr(0,6) == "OAI311"){
      kitty::create_from_hex_string(result, "10000000");
    } else if (func.substr(0,6) == "OAI32x"){
      //function : "(!A1 * !A2 * !A3) + (!B1 * !B2)";
      kitty::create_from_hex_string(result, "1111111F");
    } else if (func.substr(0,6) == "OAI31x"){
      kitty::create_from_chain(result, {"x5 = x1 !| x4", "x6 = x2 !| x4", "x7 = x3 !| x4", "x8 = x5 | x6", "x9 = x7 !| x8"});
    } else if (func.substr(0,6) == "OA21x2"){
      kitty::create_from_chain(result, {"x4 = x1 & x3", "x5 = x2 & x3", "x6 = x4 | x5"});
    } else if (func.substr(0,6) == "OA22x2"){
      // function : "(A1 * B1) + (A1 * B2) + (A2 * B1) + (A2 * B2)";
      kitty::create_from_chain(result, {"x5 = x1 & x3", "x6 = x1 & x4", "x7 = x2 & x3", "x8 = x2 & x4", "x9 = x5 | x6", "x10 = x7 | x8", "x11 = x9 | x10"});
    } else if (func.substr(0,6) == "OA31x2"){
      // function : "(A1 * B1) + (A2 * B1) + (A3 * B1)";
      kitty::create_from_chain(result, {"x5 = x1 & x4", "x6 = x2 & x4", "x7 = x3 & x4", "x8 = x5 | x6", "x9 = x8 | x7"});
    } else if (func.substr(0,6) == "AO211x"){
      //      function : "(A1 * A2) + (B) + (C)";
      kitty::create_from_chain(result, {"x5 = x1 & x2", "x6 = x5 | x3", "x7 = x6 | x4"});
    } else if (func.substr(0,6) == "AO21x2"){
      //      function : "(A1 * A2) + (B)";
      kitty::create_from_chain(result, {"x4 = x1 & x2", "x5 = x4 | x3"});
    } else if (func.substr(0,6) == "AO22x1"){
      //      function : "(A1 * A2) + (B1 * B2)";
      kitty::create_from_chain(result, {"x5 = x1 & x2", "x6 = x3 & x4", "x7 = x5 | x6"});
    } else if (func.substr(0,6) == "AO31x2"){
      //       function : "(A1 * A2 * A3) + (B)";
      kitty::create_from_chain(result, {"x5 = x1 & x2", "x6 = x5 & x3", "x7 = x6 | x4"});
    }else {
      result = kitty::dynamic_truth_table(8);
    };
    return result;
  }

};


template<class NtkDest, class NtkSource>
std::tuple<NtkDest, std::unordered_map<int, std::string>> techmap_mapped_network( NtkSource const& ntk )
{
  
  static_assert( mockturtle::is_network_type_v<NtkSource>, "NtkSource is not a network type" );
  static_assert( mockturtle::is_network_type_v<NtkDest>, "NtkDest is not a network type" );
  static_assert( mockturtle::has_get_constant_v<NtkSource>, "NtkSource does not implement the get_constant method" );
  static_assert( mockturtle::has_get_node_v<NtkSource>, "NtkSource does not implement the get_node method" );
  static_assert( mockturtle::has_foreach_pi_v<NtkSource>, "NtkSource does not implement the foreach_pi method" );
  static_assert( mockturtle::has_foreach_po_v<NtkSource>, "NtkSource does not implement the foreach_po method" );
  static_assert( mockturtle::has_foreach_node_v<NtkSource>, "NtkSource does not implement the foreach_node method" );
  static_assert( mockturtle::has_is_constant_v<NtkSource>, "NtkSource does not implement the is_constant method" );
  static_assert( mockturtle::has_is_pi_v<NtkSource>, "NtkSource does not implement the is_pi method" );
  static_assert( mockturtle::has_get_constant_v<NtkDest>, "NtkDest does not implement the get_constant method" );
  static_assert( mockturtle::has_create_pi_v<NtkDest>, "NtkDest does not implement the create_pi method" );
  static_assert( mockturtle::has_create_node_v<NtkDest>, "NtkDest does not implement the create_node method" );

  collapse_techmap_impl<NtkDest, NtkSource> p( ntk );
  return p.run();
}

} /* namespace mockturtle */
