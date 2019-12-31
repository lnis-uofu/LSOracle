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
class decompose_unmapped_LUTs
{
public:
    decompose_unmapped_LUTs_impl( NtkSource const& ntk )
        : ntk( ntk )
    {
    }

    NtkDest run()
    {
        NtkDest dest;
        nlohmann::json json_library; //LUT -> verilog database
        mockturtle::node_map<mockturtle::signal<NtkDest>, NtkSource> node_to_signal( ntk ); //i/o for original klut network
        std::ifstream library("../../NPN_complete_noZero.json"); //make this generic once it's working
        library >> json_library; //this will be huge if we go above LUT4.  May need to memoize.
        std::cout << "NUM LATCHES: " << ntk.num_latches() << "\n";
        //not going to worry about inputs and outputs just yet.  Just putting the trappings here for now.
        ntk.foreach_pi( [&]( auto const& pi, auto index ) {
            //auto signal = ntk.make_signal( pi );
            if(index < ntk.num_pis() - ntk.num_latches()){
            
            } else {
                if(ntk.num_latches()>0){

                }
            }
        } );

        ntk.foreach_node( [&]( auto const n ) {
            if ( ntk.is_constant( n ) || ntk.is_pi( n ) ){
                return;
            }
            auto const func = ntk.node_function( n );
            auto NPNconfig = kitty::exact_npn_canonization(func);
            std::string tempstr = kitty::to_hex(std::get<0>(NPNconfig));
            std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper );
            std::string json_lookup = fmt::format("out_{}", tempstr);
            ntk.foreach_fanin( n, [&]( auto fanin ) {
                cell_children.push_back( node_to_signal[fanin] );
            } );
            try {
                //if this succeeds, we're just copying the nodes from the old network to the new.
                std::vector<std::string> node_gates = json_library[json_lookup]["gates"];
            } catch (const std::exception& ex){
                //Not in json file; need to decompose function.
                node_to_signal[n] = mockturtle::shannon_decomposition( dest, func, cell_children);  
            }

        } );



}