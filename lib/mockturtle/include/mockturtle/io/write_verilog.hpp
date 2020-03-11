/* mockturtle: C++ logic network library
 * Copyright (C) 2018-2019  EPFL EPFL
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
  \file write_verilog.hpp
  \brief Write networks to structural Verilog format

  \author Mathias Soeken
*/

#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <string>

#include <lorina/verilog.hpp>
#include <ez/direct_iterator.hpp>
#include <fmt/format.h>

#include "../traits.hpp"
#include "../utils/node_map.hpp"
#include "../utils/string_utils.hpp"
#include "../views/topo_view.hpp"

namespace mockturtle
{

using namespace std::string_literals;

namespace detail
{

template<class Ntk>
std::vector<std::pair<bool, std::string>>
format_fanin( Ntk const& ntk, node<Ntk> const& n, node_map<std::string, Ntk>& node_names )
{
  std::vector<std::pair<bool, std::string>> children;
  ntk.foreach_fanin( n, [&]( auto const& f ) {
      children.emplace_back( std::make_pair( ntk.is_complemented( f ), node_names[f] ) );
    });
  return children;
}

} // namespace detail

struct write_verilog_params
{
  std::string module_name = "top";
  std::vector<std::pair<std::string, uint32_t>> input_names;
  std::vector<std::pair<std::string, uint32_t>> output_names;
  uint32_t skip_feedthrough = 0u;
};

/*! \brief Writes network in structural Verilog format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required network functions:**
 * - `num_pis`
 * - `num_pos`
 * - `foreach_pi`
 * - `foreach_node`
 * - `foreach_fanin`
 * - `get_node`
 * - `get_constant`
 * - `is_constant`
 * - `is_pi`
 * - `is_and`
 * - `is_or`
 * - `is_xor`
 * - `is_xor3`
 * - `is_maj`
 * - `node_to_index`
 *
 * \param ntk Network
 * \param os Output stream
 */
template<class Ntk>
void write_verilog( Ntk const& ntk, std::ostream& os, write_verilog_params const& ps = {} )
{
  static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
  static_assert( has_num_latches_v<Ntk>, "Ntk does not implement the num_latches method" );
  static_assert( has_num_pis_v<Ntk>, "Ntk does not implement the num_pis method" );
  static_assert( has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
  static_assert( has_foreach_pi_v<Ntk>, "Ntk does not implement the foreach_pi method" );
  static_assert( has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
  static_assert( has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );
  static_assert( has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
  static_assert( has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
  static_assert( has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
  static_assert( has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
  static_assert( has_is_and_v<Ntk>, "Ntk does not implement the is_and method" );
  static_assert( has_is_or_v<Ntk>, "Ntk does not implement the is_or method" );
  static_assert( has_is_xor_v<Ntk>, "Ntk does not implement the is_xor method" );
  static_assert( has_is_xor3_v<Ntk>, "Ntk does not implement the is_xor3 method" );
  static_assert( has_is_maj_v<Ntk>, "Ntk does not implement the is_maj method" );
  static_assert( has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );

  node_map<std::string, Ntk> node_names( ntk );
  std::vector<std::string> xs, inputs, ros;

  if ( !ps.input_names.empty() )
  {
    uint32_t ctr{0u};
    for ( auto const& [name, width] : ps.input_names )
    {
      inputs.emplace_back( name );
      ctr += width;
      for ( auto i = 0u; i < width; ++i )
      {
        xs.emplace_back( fmt::format( "{}[{}]", name, i ) );
      }
    }
    if ( ctr != ntk.num_cis() )
    {
      std::cerr << "[e] input names do not partition all inputs\n";
    }
  }
  else
  {
    auto latch_index = 1u;
    ntk.foreach_ci( [&]( auto const& n, auto index ) {
      auto signal = ntk.make_signal( n );
      if(index < ntk.num_cis() - ntk.num_latches()){
        if constexpr ( has_has_name_v<Ntk> && has_get_name_v<Ntk> )
        {  
          node_names[n] = ntk.has_name( signal ) ? ntk.get_name( signal ) : fmt::format( "pi{}", index );
          if( node_names[n].find( '[' ) != std::string::npos || node_names[n].find('[') != std::string::npos )
            node_names[n].insert( node_names[n].begin(), '\\' );
        }
        else
        {
          node_names[n] = fmt::format( "pi{}", index );
        }
        xs.push_back( node_names[n] );
      }
      else
      {
        if(ntk.num_latches()>0) 
        {
          if constexpr ( has_has_name_v<Ntk> && has_get_name_v<Ntk> )
          {  
            node_names[n] = ntk.has_name( signal ) ? ntk.get_name( signal ) : fmt::format( "lo{}", latch_index );
            if( node_names[n].find( '[' ) != std::string::npos || node_names[n].find('[') != std::string::npos )
              node_names[n].insert( node_names[n].begin(), '\\' );
          }
          else
          {
            node_names[n] = fmt::format( "lo{}", latch_index );
          }
        }
        latch_index++;
        ros.push_back( node_names[n] );
      }
    } );
    inputs = xs;
  }

  std::vector<std::string> ys, outputs;
  if( !ps.output_names.empty() )
  {
    uint32_t ctr{0u};
    for ( auto const& [name, width] : ps.output_names )
    {
      outputs.emplace_back( name );
      ctr += width;
      for ( auto i = 0u; i < width; ++i )
      {
        ys.emplace_back( fmt::format( "{}[{}]", name, i ) );
      }
    }
    if ( ctr != ntk.num_pos() )
    {
      std::cerr << "[e] output names do not partition all outputs\n";
    }
  }
  else
  {
    auto latch_index = 1u;
    ntk.foreach_po( [&]( auto const& n, auto index ) {
      std::string output_name;
      if constexpr ( has_has_output_name_v<Ntk> && has_get_output_name_v<Ntk> )
      {  
        output_name = ntk.has_output_name( index ) ? ntk.get_output_name( index ) : fmt::format( "po{}", index );
        if( output_name.find( '[' ) != std::string::npos || output_name.find( '[' ) != std::string::npos )
          output_name.insert( output_name.begin(), '\\' );
      }
      else
      {
        output_name = fmt::format( "po{}", index );
      }
      ys.push_back( output_name );
      
    } );
    outputs = ys;
  }
  
  std::vector<std::string> ws;
  ntk.foreach_gate( [&]( auto const& n ) {
    ws.emplace_back( fmt::format( "new_n{}", ntk.node_to_index( n ) ) );
  } );

  lorina::verilog_writer writer( os );
  writer.on_module_begin( ps.module_name, inputs, outputs );
  if ( !ps.input_names.empty() )
  {
    for ( auto const& [name, width] : ps.input_names )
    {
      writer.on_input( width, name );
    }
  }
  else
  {
    writer.on_input( xs );
  }
  
  if ( !ps.output_names.empty() )
  {
    for ( auto const& [name, width] : ps.output_names )
    {
      writer.on_output( width, name );
    }
  }
  else
  {
    writer.on_output( ys );
  }
  if ( !ros.empty() )
  {
    writer.on_reg( ros );
  }
  if ( !ws.empty() )
  {
    writer.on_wire( ws );
  }

  node_names[ntk.get_constant( false )] = "1'b0";
  if ( ntk.get_node( ntk.get_constant( false ) ) != ntk.get_node( ntk.get_constant( true ) ) )
    node_names[ntk.get_constant( true )] = "1'b1";

  ntk.foreach_pi( [&]( auto const& n, auto i ) {
    node_names[n] = xs[i];
  } );
  
  topo_view ntk_topo{ntk};

  ntk_topo.foreach_node( [&]( auto const& n ) {
    if ( ntk.is_constant( n ) || ntk.is_ci( n ) )
      return true;

    /* assign a name */
    node_names[n] = fmt::format( "new_n{}", ntk.node_to_index( n ) );

    if ( ntk.is_and( n ) )
    {
      writer.on_assign( node_names[n], detail::format_fanin<Ntk>( ntk, n, node_names ), "&" );
    }
    else if ( ntk.is_or( n ) )
    {
      writer.on_assign( node_names[n], detail::format_fanin<Ntk>( ntk, n, node_names ), "|" );
    }
    else if ( ntk.is_xor( n ) || ntk.is_xor3( n ) )
    {
      writer.on_assign( node_names[n], detail::format_fanin<Ntk>( ntk, n, node_names ), "^" );
    }
    else if ( ntk.is_maj( n ) )
    {
      std::array<signal<Ntk>, 3> children;
      ntk.foreach_fanin( n, [&]( auto const& f, auto i ) { children[i] = f; } );

      if ( ntk.is_constant( ntk.get_node( children[0u] ) ) )
      {
        std::vector<std::pair<bool, std::string>> vs;
        vs.emplace_back( std::make_pair( ntk.is_complemented( children[1u] ), node_names[ntk.get_node( children[1u] )] ) );
        vs.emplace_back( std::make_pair( ntk.is_complemented( children[2u] ), node_names[ntk.get_node( children[2u] )] ) );

        if ( ntk.is_complemented( children[0u] ) )
        {
          // or
          writer.on_assign( node_names[n], {vs[0u], vs[1u]}, "|" );
        }
        else
        {
          // and
          writer.on_assign( node_names[n], {vs[0u], vs[1u]}, "&" );
        }
      }
      else
      {
        writer.on_assign_maj3( node_names[n], detail::format_fanin<Ntk>( ntk, n, node_names ) );
      }
    }
    else
    {
      writer.on_assign_unknown_gate( node_names[n] );
    }

    return true;
  } );

  ntk.foreach_po( [&]( auto const& f, auto i ) {
    writer.on_assign_po( ys[i], std::make_pair( ntk.is_complemented( f ), node_names[f] ) );
  } );

  std::vector<uint64_t> ro_inits;
  ntk.foreach_ro( [&]( auto const& ro ){
    auto ro_sig = ntk.make_signal( ro );
    auto ri_node = ntk.ro_to_ri( ro_sig );
    mockturtle::latch_info l_info = ntk._storage->latch_information[ro];

    if( l_info.type != "fe" && l_info.type != "re" ){
      std::cout << "WARNING: only positive or negative edge triggered latches are supported. \
          Latch type for " << node_names[ro] << " set to rising edge.\n";
      l_info.type = "re";
    }

    ro_inits.emplace_back( l_info.init );

    writer.on_control_reg( node_names[ro], node_names[ri_node], std::make_pair( l_info.type, l_info.control ) );
  } );

  if( ntk.num_latches() > 0 )
    writer.on_reg_init( ros, ro_inits );

  writer.on_module_end();
}

/*! \brief Writes network in structural Verilog format into a file
 *
 * **Required network functions:**
 * - `num_pis`
 * - `num_pos`
 * - `foreach_pi`
 * - `foreach_node`
 * - `foreach_fanin`
 * - `get_node`
 * - `get_constant`
 * - `is_constant`
 * - `is_pi`
 * - `is_and`
 * - `is_or`
 * - `is_xor`
 * - `is_xor3`
 * - `is_maj`
 * - `node_to_index`
 *
 * \param ntk Network
 * \param filename Filename
 */
template<class Ntk>
void write_verilog( Ntk const& ntk, std::string const& filename, write_verilog_params const& ps = {} )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_verilog( ntk, os, ps );
  os.close();
}

} /* namespace mockturtle */
