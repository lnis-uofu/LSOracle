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

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>
#include <mockturtle/utils/string_utils.hpp>


#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>

#include <fmt/format.h>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <nlohmann/json.hpp>


namespace oracle
{


template<class Ntk>
void write_techmapped_verilog( Ntk const& ntk, std::ostream& os, std::string top_name )
{
    //Right now, as a first effort, this will only work after LUT mapping, so ntk will always be klut
    //  and it will not support sequential circuits, because as far as I can tell, the klut network doesn't either yet.
    
    static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
    static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
    static_assert( mockturtle::has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
    static_assert( mockturtle::has_is_complemented_v<Ntk>, "Ntk does not implement the is_complemented method" );
    static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
    static_assert( mockturtle::has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
    static_assert( mockturtle::has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );
    static_assert( mockturtle::has_node_function_v<Ntk>, "Ntk does not implement the node_function method" );
    

    int netlistcount = 0;
    nlohmann::json json_library;
    regex in_match("\(F\)");
    regex wire_match("new");
    regex g_count("g[0-9]+");
    regex o1("\(a\)");
    regex o2("\(b\)");
    regex o3("\(c\)");
    regex o4("\(d\)");
    
    double area_total = 0;
    
    std::cout << "Loading json library ";
    std::ifstream library("../../NPN_LUT4.json"); //make this generic once it's working
    library >> json_library; //might need to do something more sophisticated here as we get to larger function classes.  For LUT4s, the file is only 150K; but as we go to 5, 6, 8 inputs, this will be huge
    std::cout << "done. Library size: " << json_library.size() << "\n";

    //populate pis and pos
    //this needs to be fixed.  I don't have a good way of knowing which node number is a PI or PO for now when I write out the netlist.  Better to just use the node index.
    auto digitsIn  = std::to_string(ntk.num_pis()/* -ntk.num_latches()*/).length();
    auto digitsOut = std::to_string(ntk.num_pos()/*-ntk.num_latches()*/).length();
    const auto xs = mockturtle::map_and_join( ez::make_direct_iterator<decltype( ntk.num_pis() )>( 0 ),
                                      ez::make_direct_iterator( ntk.num_pis() ), //these had ntk.num_latches too. if/when you add that to klut, fix this
                                      [&digitsIn]( auto i ) { return fmt::format( "pi{0:0{1}}", i, digitsIn ); }, ", "s );
    const auto ys = mockturtle::map_and_join( ez::make_direct_iterator<decltype( ntk.num_pis() )>( 0 ),
                                      ez::make_direct_iterator( ntk.num_pos()  ), //v.s.
                                      [&digitsOut]( auto i ) { return fmt::format( "po{0:0{1}}", i, digitsOut ); }, ", "s );
            
    
  /*      os << fmt::format( "module top({}, {});\n", xs, ys )
        << "\t" << fmt::format( "  input {};\n", xs )
        << "\t" << fmt::format( "  output {};\n", ys );
  */
    os << "\t" << "input ";
    auto first = true; 
    ntk.foreach_pi( [&]( auto const& n ) {
            if (first)
                first = false;
            else
                os << ", ";
            os << fmt::format("n{}", ntk.node_to_index(n));
    });
    first = true;
    os << ";\n" << "\t" << "output ";
    ntk.foreach_po( [&]( auto const& n ) {
            if (first)
                first = false;
            else
                os << ", ";
            os << fmt::format("n{}", ntk.node_to_index(n));
    });
    os <<";\n";
    /*
    wire x, y, z, etc; 
    */
    //Pretty sure that it's basically right, but the internal wires for each cell aren't handled; I'll need to add them after the fact
    //think this is now fixed.  Testing before I delete the reminder.

    if ( ntk.num_gates() > 0 ) //should this be gate or node?
    {
        os << "\t" << "  wire ";
        first = true;
        ntk.foreach_gate( [&]( auto const& n ) {
            auto index = ntk.node_to_index( n );
            if(index > ntk.num_pis()) {
                if (first)
                    first = false;
                else
                    os << ", ";
                os << fmt::format("n{}", ntk.node_to_index(n));
            }
        } );
        os << ";\n";
    }
//do the actual mapping/substitution
    ntk.foreach_node( [&]( auto const& n ) {
        if ( ntk.is_constant( n ) || ntk.is_pi( n ) )
            return; /* continue */

        auto func = ntk.node_function( n );
        std::vector <std::string> children;
       // populate children, which will be function inputs
        ntk.foreach_fanin( n, [&]( auto const& c, auto i ) {
            if ( ntk.is_complemented( c ) )
            {
                kitty::flip_inplace( func, i );
            }
           
            children.push_back(fmt::format( "n{}", ntk.node_to_index( ntk.get_node( c ) ) ));
        } );
    
    //lookup the function in the json database
        std::string tempstr = kitty::to_hex(std::get<0>(kitty::exact_npn_canonization(func)));
        std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper );
        tempstr.insert(tempstr.begin(), 4 - tempstr.length(), '0');
        std::string json_lookup = fmt::format("out_{}", tempstr);
        os << "// techmapping TT: " << json_lookup << "\n";
        
        std::string cell_out = fmt::format("n{}", ntk.node_to_index(n));
        
        //handle the internal wires
        try{
            std::string cell_wires = json_library[json_lookup]["wires"];
            //it would be nicer to have all wire declarations at the top, but this will do.
            cell_wires = regex_replace(cell_wires, wire_match, cell_out);
            os << "\t" << "wire "<< cell_wires <<";\n";

        } catch(nlohmann::json::type_error& er){
            //cell does not exist (caught below), or has no internal wires, which is fine.
        }
        
        //replace '(F)' with cell_out, '(a)' with '(<children[0]>)', '(b)' with '(<children[1]>)', and so on
        try{
            std::vector<std::string> test = json_library[json_lookup]["gates"];
            for (auto i : test){
                netlistcount++;
                i = regex_replace(i, in_match, cell_out);
                i = regex_replace(i, wire_match, cell_out);
                i = regex_replace(i, g_count, fmt::format("g{}", netlistcount));
                if(children.size() == 4){
                    i = regex_replace(i, o1, children.at(0));
                    i = regex_replace(i, o2, children.at(1));
                    i = regex_replace(i, o3, children.at(2));
                    i = regex_replace(i, o4, children.at(3));
                } else {
                    std::cout << "Attempting to call a 4 input function with fewer than 4 inputs\n";
                }
                os << "\t" << i << "\n";
            }
        } catch(nlohmann::json::type_error& er){
                     std::cout << "\nmessage: " << er.what() << '\n'<< "exception id: " << er.id << std::endl;
                     std::cout << "Perhaps truth table "<<json_lookup<<" is not found in the techmapping library?\n\n";
        }
        
        //get area of each LUT equivalent set of gates
        try{
            std::string cell_area = json_library[json_lookup]["area"];
            area_total += std::stod(cell_area, nullptr);
        } catch(nlohmann::json::type_error& er){
            std::cout << "Truth table " <<json_lookup<<" has no defined area.\n";
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

     os << "endmodule\n";
     os << "// design area: " << area_total <<"\n";
}

//file version
template<class Ntk>
void write_techmapped_verilog( Ntk const& ntk, std::string const& filename, std::string top_name )
{
      std::ofstream os( filename.c_str(), std::ofstream::out );
      write_techmapped_verilog( ntk, os, top_name );
      os.close();
}

}