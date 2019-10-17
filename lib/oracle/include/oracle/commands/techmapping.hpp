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
    //os << "module " << top_name << "( TODO )\n";
    //input and output lists
    //os << "\t" << "input ";
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
            input_list += fmt::format("n{}", n);

    });

    first = true;
    ntk.foreach_po( [&]( auto const& n, auto i ) {
            if (first)
                first = false;
            else
                output_list += ", ";
            output_list += fmt::format("po{}", i);
            po_assignments += fmt::format("\tassign po{} = n{};\n", i, n);
    });
    
    first = true;
    ntk.foreach_node ( [&](auto const& n){
        if (ntk.is_pi(n) || ntk.is_constant( n)){
            return;
        }
            if (first)
                first = false;
            else
                wire_list += ", ";
            wire_list += fmt::format("n{}", n);
    });

    os << "module " << top_name <<"("<<input_list <<", " << output_list<< ");\n";
    os << "\tinput " << input_list<< ";\n";
    os << "\toutput " << output_list << ";\n";
    os << "\twire "<< wire_list << ";\n\n";
//body

    ntk.foreach_node( [&]( auto const& n ) {
    /*
        if ( ntk.is_constant( n ) || ntk.is_pi( n ) ){
            std::cout << "pi or const: " << n << "\n";
            return;
        }
*/
        if (cell_names.find(n) != cell_names.end()){

            std::vector <std::string> children;
            // populate children, which will be function inputs
            ntk.foreach_fanin( n, [&]( auto fanin ) {
                children.push_back(fmt::format( "n{}",fanin ));
            } );
            os << "\t" << cell_names.at(n) << " ";
            if(children.size() > 0){
            os << fmt::format("g{}", n) << "(.A(" << children.at(0) << ")";
            }
            if (children.size() > 1){
                os << ", .B(" << children.at(1) << ")";
            }
            if (children.size() > 2){
                os << ", .C(" << children.at(2) << ")";
            }
            if (children.size() > 3){
                os << ", .D(" << children.at(3) << ")";
            }
            os << ", .Y(" << fmt::format("n{}",n) << ") );\n";
     }


    } );
   
    /*
    ntk.foreach_po( [&]( auto const& s, auto i ) {
        os << fmt::format( "po{} = LUT 0x{} (n{})\n",
                         i,
                         ntk.is_complemented( s ) ? 1 : 2,
                         ntk.node_to_index( ntk.get_node( s ) ) );
     } );
     */
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