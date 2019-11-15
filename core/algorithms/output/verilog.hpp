/* mockturtle: C++ logic network library
 * Copyright (C) 2018  EPFL
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

#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <string>

#include <ez/direct_iterator.hpp>
#include <fmt/format.h>

namespace oracle
{

  template<class Ntk>    
  void write_toplevel_verilog( Ntk ntk, oracle::partition_manager<Ntk> pm,  std::string filename, mockturtle::node_map<std::string, Ntk> node_names, mockturtle::node_map<std::string, Ntk> input_names, std::string modulename = "toplevel")
  {
    static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( mockturtle::has_num_latches_v<Ntk>, "Ntk does not implement the has_latches method" );
    static_assert( mockturtle::has_num_pis_v<Ntk>, "Ntk does not implement the num_pis method" );
    static_assert( mockturtle::has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
    static_assert( mockturtle::has_foreach_pi_v<Ntk>, "Ntk does not implement the foreach_pi method" );
    static_assert( mockturtle::has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
    static_assert( mockturtle::has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );
    static_assert( mockturtle::has_foreach_ri_v<Ntk>, "Ntk does not implement the foreach_ri method" );
    static_assert( mockturtle::has_foreach_ro_v<Ntk>, "Ntk does not implement the foreach_ro method" );
    static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
    static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
    static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
    static_assert( mockturtle::has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
    static_assert( mockturtle::has_is_and_v<Ntk>, "Ntk does not implement the is_and method" );
    static_assert( mockturtle::has_is_or_v<Ntk>, "Ntk does not implement the is_or method" );
    static_assert( mockturtle::has_is_xor_v<Ntk>, "Ntk does not implement the is_xor method" );
    static_assert( mockturtle::has_is_xor3_v<Ntk>, "Ntk does not implement the is_xor3 method" );
    static_assert( mockturtle::has_is_maj_v<Ntk>, "Ntk does not implement the is_maj method" );
    static_assert( mockturtle::has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );
    
    int num_parts = pm.get_part_num();
    std::ofstream os( filename.c_str(), std::ofstream::out );
    auto digitsIn  = std::to_string(ntk.num_pis()).length();
    auto digitsOut = std::to_string(ntk.num_pos()).length();

    const auto xs = mockturtle::map_and_join( ez::make_direct_iterator<decltype( ntk.num_pis() )>( 0 ),
                                  ez::make_direct_iterator( ntk.num_pis() - ntk.num_latches() ),
                                  [&digitsIn]( auto i ) { return fmt::format( "pi{0:0{1}}", i, digitsIn); }, ", "s );
    const auto ys = mockturtle::map_and_join( ez::make_direct_iterator<decltype( ntk.num_pis() )>( 0 ),
                                  ez::make_direct_iterator( ntk.num_pos() - ntk.num_latches() ),
                                  [&digitsOut]( auto i ) { return fmt::format( "po{0:0{1}}", i, digitsOut); }, ", "s );


    os << fmt::format( "module {}({}, {});\n", modulename, xs, ys)
       << fmt::format( "  input {};\n", xs )
       << fmt::format( "  output {};\n", ys );

    // Initialize 
    ntk.foreach_node( [&](auto n) {
      input_names[n] = "1'b0";
      node_names[n] = "1'b0";
    });
           
    std::vector<int> vector_output;

    ntk.foreach_pi( [&]( auto const& n, auto i ) {
      input_names[n] = fmt::format( "pi{0:0{1}}", i, digitsIn);
    } );
    
    // Store names of inputs and outpus
    auto node_list = pm.get_part_inputs(0);
    auto output_list = pm.get_part_outputs(0);

    for (int i = 0; i < num_parts; i++)
    {
      auto temp_i = pm.get_part_inputs(i);
      node_list.insert(temp_i.begin(), temp_i.end());

      auto temp_o = pm.get_part_outputs(i);
      node_list.insert(temp_o.begin(), temp_o.end());

      output_list.insert(temp_o.begin(), temp_o.end());
    }
    
    
    auto first = true;
    int count = 0;
    for (auto const &n: node_list )
    {
      if (!ntk.is_pi(n) && !ntk.is_po(n))
      {
        if (first) {
          first = false;
          os << "  wire ";
        }
        else
          os << ", ";

        node_names[n] = fmt::format( "n{}", count);
        os << fmt::format("n{}", count);
        count++;
      }
    }

    ntk.foreach_po( [&]( auto const& n, auto i ) {
      if ( node_names[n] == "1'b0" ) {
        node_names[n] = fmt::format( "tpo{0:0{1}}", i, digitsOut);
        vector_output.push_back( -1 );
        if (first) {
          first = false;
          os << "  wire ";
        }
        else
          os << ", ";

        os << node_names[n];
      }   
      else {
        vector_output.push_back( ntk._storage->outputs.at(i).index );
      }
    } );

    if (!first)
      os << ";\n";
           
    for ( int i = 0; i < ntk.num_pos(); i++ ) {
      if ( vector_output[ i ] != -1 )
        os << fmt::format( "  assign po{0:0{1}} = {2}{3};\n", i, digitsOut, ntk.is_complemented( ntk._storage->outputs.at(i) ) ? "~" : "", node_names[ vector_output[i] ] );
      else
        os << fmt::format( "  assign po{0:0{1}} = {2}tpo{0:0{1}};\n", i, digitsOut, ntk.is_complemented( ntk._storage->outputs.at(i) ) ? "~" : "" );
    }

    os.close();
  }
  
  
  template<class Ntk>    
  void call_submodule( Ntk const& ntk, Ntk const& part_ntk, std::string const& filename, std::string const& modulename, int part_num, oracle::partition_view<Ntk> const& part, mockturtle::node_map<std::string, Ntk> node_names, mockturtle::node_map<std::string, Ntk> input_names )
  {
    static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( mockturtle::has_num_latches_v<Ntk>, "Ntk does not implement the has_latches method" );
    static_assert( mockturtle::has_num_pis_v<Ntk>, "Ntk does not implement the num_pis method" );
    static_assert( mockturtle::has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
    static_assert( mockturtle::has_foreach_pi_v<Ntk>, "Ntk does not implement the foreach_pi method" );
    static_assert( mockturtle::has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
    static_assert( mockturtle::has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );
    static_assert( mockturtle::has_foreach_ri_v<Ntk>, "Ntk does not implement the foreach_ri method" );
    static_assert( mockturtle::has_foreach_ro_v<Ntk>, "Ntk does not implement the foreach_ro method" );
    static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
    static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
    static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
    static_assert( mockturtle::has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
    static_assert( mockturtle::has_is_and_v<Ntk>, "Ntk does not implement the is_and method" );
    static_assert( mockturtle::has_is_or_v<Ntk>, "Ntk does not implement the is_or method" );
    static_assert( mockturtle::has_is_xor_v<Ntk>, "Ntk does not implement the is_xor method" );
    static_assert( mockturtle::has_is_xor3_v<Ntk>, "Ntk does not implement the is_xor3 method" );
    static_assert( mockturtle::has_is_maj_v<Ntk>, "Ntk does not implement the is_maj method" );
    static_assert( mockturtle::has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );
    
    std::ofstream os( filename.c_str(), std::ofstream::app );
    os << fmt::format("  {} U{} ( ", modulename, part_num);
    
    auto digitsIn  = std::to_string(part_ntk.num_pis()-part_ntk.num_latches()).length();
    auto digitsOut = std::to_string(part_ntk.num_pos()-part_ntk.num_latches()).length();

    auto first = true;
    part_ntk.foreach_pi( [&]( auto const& n, auto i ) {
      auto idx = part_ntk.node_to_index( n );
      auto node_id = part.index_to_node( idx );
      if (first)
        first = false;
      else
        os << ", ";

      if ( ntk.is_pi( node_id ) )
        os << fmt::format(".pi{0:0{1}}( ", i, digitsIn) << input_names[node_id] << " )";
      else
        os << fmt::format(".pi{0:0{1}}( ", i, digitsIn) << node_names[node_id] << " )";
        
    } );

    part_ntk.foreach_po( [&]( auto const& n, auto i ) {
      auto idx = part_ntk._storage->outputs.at(i).index;
      auto node_id = part.index_to_node( idx );
      if (first)
        first = false;
      else
        os << ", ";

      if ( node_names[node_id] == "1'b0" )
        os << fmt::format(".po{0:0{1}}(  )", i, digitsOut);
      else
        os << fmt::format(".po{0:0{1}}( ", i, digitsOut) << node_names[node_id] << " )";           
    } );

    os << " );\n";
    os.close();
  }

} /* namespace mockturtle */