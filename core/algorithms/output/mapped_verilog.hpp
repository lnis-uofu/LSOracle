 /* LSOracle
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
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
example verilog file

module ex ( 
a, b, c, d,
F  );
input  a, b, c, d;
output F;
//wire [...];
NOR4xp25_ASAP7_75t_R  g0(.A(d), .B(c), .C(b), .D(a), .Y(F));
endmodule
*/

#pragma once

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#include <fmt/format.h>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/utils/string_utils.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <nlohmann/json.hpp>


namespace oracle
{


template<class Ntk>
void write_techmapped_verilog( Ntk const& ntk, std::ostream& os, std::unordered_map<int, std::string> cell_names, std::string top_name )
{
    int netlistcount;
    std::string input_list;
    std::string output_list;
    std::string wire_list;
    std::string po_assignments;
    auto first = true; 
    ntk.foreach_pi( [&]( auto const& n ) {
            if (first)
                first = false;
            else
                input_list += ", ";
            input_list += fmt::format("n{}", ntk.node_to_index( n ) );

    });

    first = true;
    ntk.foreach_po( [&]( auto const& n, auto i ) {
            if (first)
                first = false;
            else
                output_list += ", ";
            output_list += fmt::format("po{}", i);
            if ( ntk.is_constant( ntk.get_node( n ) ) ){
                po_assignments += fmt::format( "\tassign po{} = {};\n", i, ntk.is_complemented( n ) ? "1'b1" : "1'b0" );
            } else {
                po_assignments += fmt::format("\tassign po{} = n{};\n", i, n);
            }
    });
    
    first = true;
    ntk.foreach_node ( [&](auto const& n){
        if (ntk.is_pi(n) || ntk.is_constant( n)){
            return;
        }
        if(cell_names.find(n) != cell_names.end()){
            if (first)
                first = false;
            else
                wire_list += ", ";
            wire_list += fmt::format("n{}", n);
        }
    });

    os << "module " << top_name <<"("<<input_list <<", " << output_list<< ");\n";
    os << "\tinput " << input_list<< ";\n";
    os << "\toutput " << output_list << ";\n";
    os << "\twire "<< wire_list << ";\n\n";
//body

    ntk.foreach_node( [&]( auto const& n ) {
                int flag = 0;

        if (cell_names.find(n) != cell_names.end()){
            std::vector <std::string> children;
            std::vector <std::string> port_names;
            if(regex_match(cell_names.at(n), std::regex("[AOIx123]{6}.+"))){
                std::string working_name = cell_names.at(n);
                working_name.erase(std::remove_if(working_name.begin(), working_name.end(), [](char c) { return !std::isdigit(c);}), working_name.end());
                if (working_name.at(0) == '2'){
                    port_names.push_back("A1");
                    port_names.push_back("A2");
                } else if (working_name.at(0) == '3'){
                    port_names.push_back("A1");
                    port_names.push_back("A2");
                    port_names.push_back("A3");
                }
                if (working_name.at(1)  == '1'){
                    port_names.push_back("B");
                } else if (working_name.at(1) == '2'){
                    port_names.push_back("B1");
                    port_names.push_back("B2");
                } else if (working_name.at(1)  == '3'){
                    port_names.push_back("B1");
                    port_names.push_back("B2");
                    port_names.push_back("B3");
                }
                if (working_name.at(2)  == '1'){
                    port_names.push_back("C");
                }
            } else {
                port_names.push_back("A");
                port_names.push_back("B");
                port_names.push_back("C");
                port_names.push_back("D");
            }
            // populate children, which will be function inputs
            ntk.foreach_fanin( n, [&]( auto fanin ) {
                //handle constants in fanin
                if (fanin == 0){
                    children.push_back("1'b0");
                } else if (fanin == 1){
                    children.push_back("1'b1");
                } else {
                    children.push_back(fmt::format( "n{}",fanin ));
                }
            } );
            os << "\t" << cell_names.at(n) << " "; 
                if(children.size() > 0){       
                    os << fmt::format("g{}", n) << "(." << port_names.at(0) << "(" << children.at(0) << ")";
                }
                if (children.size() > 1){
                    os << ", ." <<port_names.at(1)<<"(" << children.at(1) << ")";
                }
                if (children.size() > 2){
                    os << ", ."<<port_names.at(2)<<"(" << children.at(2) << ")";
                }
                if (children.size() > 3){
                    os << ", ."<<port_names.at(3)<<"(" << children.at(3) << ")";
                }
                if (children.size() > 4){
                    os << ", ."<<port_names.at(4)<<"(" << children.at(4) << ")";
                }
                if (children.size() > 5){
                    os << ", ."<<port_names.at(5)<<"(" << children.at(5) << ")";
                }
                os << ", .Y(" << fmt::format("n{}",n) << ") );\n";
     }
    } );
    os << po_assignments;
    os << "endmodule\n";
}

//file version
template<class Ntk>
void write_techmapped_verilog( Ntk const& ntk, std::string const& filename, std::unordered_map<int, std::string> cell_names, std::string top_name )
{
      std::ofstream os( filename.c_str(), std::ofstream::out );
      write_techmapped_verilog( ntk, os, cell_names, top_name );
      os.close();
}

}