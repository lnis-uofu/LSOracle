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
    /*
    static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
    static_assert( has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
    static_assert( has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
    static_assert( has_is_complemented_v<Ntk>, "Ntk does not implement the is_complemented method" );
    static_assert( has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
    static_assert( has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
    static_assert( has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );
    static_assert( has_node_function_v<Ntk>, "Ntk does not implement the node_function method" );
    */

    std::cout << "Loading json library ";
    std::ifstream library("../../NPN_LUT4.json");
    nlohmann::json json_library;
    library >> json_library; //might need to do something more sophisticated here as we get to larger function classes.  For LUT4s, the file is only 150K; but as we go to 5, 6, 8 inputs, this will be huge
    std::cout << "testing:" << json_library.size();
    std::vector <std::string> inputs;
    std::vector <std::string> pos;
    auto digitsIn  = std::to_string(ntk.num_pis()/* -ntk.num_latches()*/).length();
    auto digitsOut = std::to_string(ntk.num_pos()/*-ntk.num_latches()*/).length();
    const auto xs = mockturtle::map_and_join( ez::make_direct_iterator<decltype( ntk.num_pis() )>( 0 ),
                                      ez::make_direct_iterator( ntk.num_pis() ), //these had ntk.num_latches too. if/when you add that to klut, fix this
                                      [&digitsIn]( auto i ) { return fmt::format( "pi{0:0{1}}", i, digitsIn ); }, ", "s );
    const auto ys = mockturtle::map_and_join( ez::make_direct_iterator<decltype( ntk.num_pis() )>( 0 ),
                                      ez::make_direct_iterator( ntk.num_pos()  ), //v.s.
                                      [&digitsOut]( auto i ) { return fmt::format( "po{0:0{1}}", i, digitsOut ); }, ", "s );
            
    
    
    os << fmt::format( "module top({}, {});\n", xs, ys )
        << fmt::format( "  input {};\n", xs )
        << fmt::format( "  output {};\n", ys );
    /*
    wire x, y, z, etc; 
    */
    //not sure if this is handled correctly; this is taken from the verilog writer, but I'm pretty sure I should handle wires as I go in the next section. 
    //maybe somehow collect the wires that I need and then insert them near the header again when I'm done
    if ( ntk.num_gates() > 0 )
    {
        os << "  wire ";
        auto first = true;
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
//do the actual 
    ntk.foreach_node( [&]( auto const& n ) {
        if ( ntk.is_constant( n ) || ntk.is_pi( n ) )
            return; /* continue */

        auto func = ntk.node_function( n );
        std::string children;
        auto first = true;
        ntk.foreach_fanin( n, [&]( auto const& c, auto i ) {
            if ( ntk.is_complemented( c ) )
            {
                kitty::flip_inplace( func, i );
            }
            if ( first )
            {
                first = false;
            }
            else
            {
                children += ", ";
            }
            children += fmt::format( "n{}", ntk.node_to_index( ntk.get_node( c ) ) );
        } );
    //this is where I need to write the lookup
    /*  Bench format looked like this:
        os << fmt::format( "n{} = LUT 0x{} ({})\n",
                       ntk.node_to_index( n ),
                      kitty::to_hex(std::get<0>(kitty::exact_npn_canonization( func ))), children);
        
        In my case I want n{} to be mapped to the Y() pin, and the children to A, B, C, and D if applicable
     */
        std::string tempstr = kitty::to_hex(std::get<0>(kitty::exact_npn_canonization(func)));
        std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper );
        tempstr.insert(tempstr.begin(), 4 - tempstr.length(), '0');
        std::string json_lookup = fmt::format("out_{}", tempstr);
        os << "// techmapping TT: " << json_lookup << "\n";
        try{
            std::vector<std::string> test = json_library[json_lookup]["gates"];
            for (auto i : test){
                os << i <<"\n";
            }
        } catch(nlohmann::json::type_error& er){
                     std::cout << "\nmessage: " << er.what() << '\n'<< "exception id: " << er.id << std::endl;
                     std::cout << "Perhaps truth table "<<json_lookup<<" is not found in the techmapping library?\n\n";
        }


    } );

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