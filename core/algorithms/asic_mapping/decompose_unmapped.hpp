/******************************************************************************
 * LSOracle: A learning based Oracle for Logic Synthesis
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
 *
 * decompose_unmapped_LUTs:
 * This function reads an LUT network and a database of techmapped LUTs.
 * If it encounters an LUT whose NPN canonical form is not in the database,
 * it decomposes the function into smaller functions, which should be mapped.
 * The database contains all 2, 3, and 4 input functions, and a subset of 
 * 5 and 6 input functions.  Functions larger than 6 inputs will be 
 * added soon, so this implementation should be general. Latches need to be
 * preserved, because we are adding sequential support to the techmapper.
 *****************************************************************************/

#pragma once

#include <unordered_map>
#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include <kitty/operators.hpp>

namespace oracle
{
template<class NtkDest, class NtkSource>
class decompose_unmapped_impl
{
public:
    decompose_unmapped_impl( NtkSource const& ntk )
        : ntk( ntk )
    {
    }

    NtkDest run()
    {
        NtkDest dest;
        nlohmann::json json_library; //LUT -> verilog database
        mockturtle::node_map<mockturtle::signal<NtkDest>, NtkSource> node_to_signal( ntk ); //i/o for original klut network
        std::ifstream library("/Users/srt/code/LSOracle/11FEB_ASAP7.json"); //make this generic once it's working
        library >> json_library; //this will be huge if we go above LUT4.  May need to memoize.
        std::cout << "NUM LATCHES: " << ntk.num_latches() << "\n";
        //not going to worry about inputs and outputs just yet.  Just putting the trappings here for now.
        ntk.foreach_pi( [&]( auto const& pi, auto index ) {
            //auto signal = ntk.make_signal( pi );
            if(index < ntk.num_pis() - ntk.num_latches()){
                node_to_signal[pi] = dest.create_pi();
                //std::cout << "index: " << index << "PI: "<< pi << "NTS: " << node_to_signal[index] << "\t";
            } else {
                if(ntk.num_latches()>0){

                }
            }
        } );

        ntk.foreach_node( [&]( auto const n ) {
            if ( ntk.is_constant( n ) || ntk.is_pi( n ) ){
                return;
            }
            std::vector<mockturtle::signal<NtkDest>> cell_children;
            auto const func = ntk.node_function( n );
            auto NPNconfig = kitty::exact_npn_canonization(func);
            std::string tempstr = kitty::to_hex(std::get<0>(NPNconfig));
            std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::tolower );
            std::string json_lookup = fmt::format("out_{}", tempstr);
            ntk.foreach_fanin( n, [&]( auto fanin ) {
                std::cout << "node: " << n << " fanin: " << fanin << " ";
                cell_children.push_back( node_to_signal[fanin] );
                //std::cout << "nts: " << node_to_signal[fanin] << "\n";
            } );
            try {
                //if this succeeds, we're just copying the nodes from the old network to the new.
                std::vector<std::string> node_gates = json_library[json_lookup]["gates"];
                node_to_signal[n] = dest.create_node(cell_children, func);
                std::cout << dest.size() << "\n";
            } catch (const std::exception& ex){
                //Not in json file; need to decompose function.
                std::cout << "Decomposing: " << kitty::to_hex(func) << "\n";
                node_to_signal[n] = mockturtle::shannon_decomposition( dest, func, cell_children);
                std::cout << "******** NTS: " << node_to_signal[n] << " ***********\n";
            }

        } );

        ntk.foreach_po( [&]( auto const& f ) {
            dest.create_po( node_to_signal[f] );
        } );

        return dest;
    }
private:
  NtkSource const& ntk;
};

template<class NtkDest, class NtkSource>
NtkDest decompose_unmapped ( NtkSource const& ntk )
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

  decompose_unmapped_impl<NtkDest, NtkSource> p( ntk );
  return p.run();
}

} /* namespace mockturtle */