/* mockturtle: C++ logic network library
 * Copyright (C) 2018-2019  EPFL
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

/*!
  \file write_bench.hpp
  \brief Write networks to BENCH format

  \author Mathias Soeken
*/

#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include <fmt/format.h>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>

#include "../traits.hpp"

namespace mockturtle
{

/*! \brief Writes network in BENCH format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required network functions:**
 * - `is_constant`
 * - `is_pi`
 * - `is_complemented`
 * - `get_node`
 * - `num_pos`
 * - `node_to_index`
 * - `node_function`
 *
 * \param ntk Network
 * \param os Output stream
 */
template<class Ntk>
void write_bench( Ntk const& ntk, std::ostream& os )
{
  static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
  static_assert( has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
  static_assert( has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
  static_assert( has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
  static_assert( has_is_complemented_v<Ntk>, "Ntk does not implement the is_complemented method" );
  static_assert( has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
  static_assert( has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
  static_assert( has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );
  static_assert( has_node_function_v<Ntk>, "Ntk does not implement the node_function method" );

  node_map<std::string, Ntk> node_names( ntk );
  std::map<uint32_t, std::string> output_names;

  ntk.foreach_pi( [&]( auto const& pi, auto index ) {
    auto signal = ntk.make_signal( pi );
    if(index < ntk.num_pis() - ntk.num_latches()){
      if constexpr ( has_has_name_v<Ntk> && has_get_name_v<Ntk> )
      {  
        node_names[signal] = ntk.has_name( signal ) ? ntk.get_name( signal ) : fmt::format( "pi{}", index );
        // if(node_names[signal].find('[') != std::string::npos || node_names[signal].find('[') != std::string::npos)
        //   node_names[signal].insert(node_names[signal].begin(),'\\');
      }
      else
      {
        node_names[signal] = fmt::format( "pi{}", index);
      }
    }
    else
    {
      std::cout << "latch outputs not supported yet for bench writer\n";
    }
  } );

  ntk.foreach_po( [&]( auto const& po, auto index ) {
    if(index < ntk.num_pos() - ntk.num_latches())
    {
      if constexpr ( has_has_output_name_v<Ntk> && has_get_output_name_v<Ntk> )
      {  
        output_names[index] = ntk.has_output_name( index ) ? ntk.get_output_name( index ) : fmt::format( "po{}", index );
        // if(output_names[index].find('[') != std::string::npos || output_names[index].find('[') != std::string::npos)
        //   output_names[index].insert(output_names[index].begin(),'\\');
      }
      else
      {
        output_names[index] = fmt::format( "po{}", index );
      }
    }
    else
    {
      std::cout << "latch inputs not supported yet for bench writer\n";
    }
  } );

  ntk.foreach_pi( [&]( auto const& n ) {
    os << fmt::format( "INPUT({})\n", node_names[ntk.make_signal(n)] );
  } );

  ntk.foreach_po( [&]( auto const& po, auto index ) {
    os << fmt::format( "OUTPUT({})\n", output_names[index] );
  } );

  os << fmt::format( "new_n{} = gnd\n", ntk.node_to_index( ntk.get_node( ntk.get_constant( false ) ) ) );
  if ( ntk.get_node( ntk.get_constant( false ) ) != ntk.get_node( ntk.get_constant( true ) ) )
  {
    os << fmt::format( "new_n{} = vdd\n", ntk.node_to_index( ntk.get_node( ntk.get_constant( true ) ) ) );
  }

  ntk.foreach_node( [&]( auto const& n ) {
    if ( ntk.is_constant( n ) || ntk.is_ci( n ) )
      return; /* continue */

    auto func = ntk.node_function( n );
    std::string children;
    auto first = true;
    ntk.foreach_fanin( n, [&]( auto const& c, auto i ) {
      if constexpr ( has_has_name_v<Ntk> && has_get_name_v<Ntk> )
      {
        signal<Ntk> const s = ntk.make_signal( ntk.node_to_index( ntk.get_node( c ) ) );
        std::string const name = ntk.has_name( s ) ? ntk.get_name( s ) : fmt::format( "new_n{}", ntk.get_node( s ) );
        node_names[c] = name;
      }
      else
      {
        node_names[c] = fmt::format( "new_n{} ", ntk.node_to_index( ntk.get_node( c ) ) );
      }
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

      children += fmt::format( "{}", node_names[c] );
    } );

    os << fmt::format( "new_n{} = LUT 0x{} ({})\n",
                       ntk.node_to_index( n ),
                       kitty::to_hex( func ), children );
  } );

  /* outputs */
  ntk.foreach_po( [&]( auto const& s, auto i ) {
    if ( ntk.is_constant( ntk.get_node( s ) ) )
    {
      os << fmt::format( "{} = {}\n",
                         output_names[i],
                         ( ntk.constant_value( ntk.get_node( s ) ) ^ ntk.is_complemented( s ) ) ? "vdd" : "gnd" );
    }
    else
    {
      os << fmt::format( "{} = LUT 0x{} (new_n{})\n",
                         output_names[i],
                         ntk.is_complemented( s ) ? 1 : 2,
                         ntk.node_to_index( ntk.get_node( s ) ) );
    }
  } );

  os << std::flush;
}

/*! \brief Writes network in BENCH format into a file
 *
 * **Required network functions:**
 * - `is_constant`
 * - `is_pi`
 * - `is_complemented`
 * - `get_node`
 * - `num_pos`
 * - `node_to_index`
 * - `node_function`
 *
 * \param ntk Network
 * \param filename Filename
 */
template<class Ntk>
void write_bench( Ntk const& ntk, std::string const& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_bench( ntk, os );
  os.close();
}

} /* namespace mockturtle */
