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
    template<int Fanin, class Ntk>
    std::pair<std::array<std::string, Fanin>, std::array<std::string, Fanin>>
    format_fanin( Ntk const& ntk, node<Ntk> const& n, node_map<std::string, Ntk>& node_names )
    {
      std::array<std::string, Fanin> children, inv;
      ntk.foreach_fanin( n, [&]( auto const& f, auto i ) {
        children[i] = node_names[f];
        inv[i] = ntk.is_complemented( f ) ? "~" : "";
      } );
      return {children, inv};
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
 * - `num_latches`
 * - `num_pis`
 * - `num_pos`
 * - `foreach_pi`
 * - `foreach_node`
 * - `foreach_fanin`
 * - `foreach_ri`
 * - `foreach_ro`
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
 * - `ri_index`
 *
 * \param ntk Network
 * \param os Output stream
 */
  template<class Ntk>
  void write_verilog( Ntk const& ntk, std::ostream& os, write_verilog_params const& ps = {} )
  {
    static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( has_num_latches_v<Ntk>, "Ntk does not implement the has_latches method" );
    static_assert( has_num_pis_v<Ntk>, "Ntk does not implement the num_pis method" );
    static_assert( has_num_pos_v<Ntk>, "Ntk does not implement the num_pos method" );
    static_assert( has_foreach_pi_v<Ntk>, "Ntk does not implement the foreach_pi method" );
    static_assert( has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
    static_assert( has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );
    static_assert( has_foreach_ri_v<Ntk>, "Ntk does not implement the foreach_ri method" );
    static_assert( has_foreach_ro_v<Ntk>, "Ntk does not implement the foreach_ro method" );
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
    
    //counting number of digits to add leading 0's
    auto digitsIn  = std::to_string(ntk.num_pis()-ntk.num_latches()).length();
    auto digitsOut = std::to_string(ntk.num_pos()-ntk.num_latches()).length();

    node_map<std::string, Ntk> node_names( ntk );
    // node_map<std::string, Ntk> output_names( ntk );
    std::map<uint32_t, std::string> output_names;
    std::map<uint32_t, std::string> ri_names;
    // node_map<std::string, Ntk> ri_names( ntk );
    std::vector<std::string> xs, ys, ros, ris;
    auto latch_index = 1u;
    ntk.foreach_pi( [&]( auto const& pi, auto index ) {
      auto signal = ntk.make_signal( pi );
      if(index < ntk.num_pis() - ntk.num_latches()){
        if constexpr ( has_has_name_v<Ntk> && has_get_name_v<Ntk> )
        {  
          node_names[signal] = ntk.has_name( signal ) ? ntk.get_name( signal ) : fmt::format( "pi{0:0{1}}", index, digitsIn );
          if(node_names[signal].find('[') != std::string::npos || node_names[signal].find('[') != std::string::npos)
            node_names[signal].insert(node_names[signal].begin(),'\\');
        }
        else
        {
          node_names[signal] = fmt::format( "pi{0:0{1}}", index, digitsIn );
        }
        xs.push_back(node_names[signal]);
      }
      else
      {
        if(ntk.num_latches()>0) 
        {
          if constexpr ( has_has_name_v<Ntk> && has_get_name_v<Ntk> )
          {  
            node_names[signal] = ntk.has_name( signal ) ? ntk.get_name( signal ) : fmt::format( "lo{}", latch_index );
            if(node_names[signal].find('[') != std::string::npos || node_names[signal].find('[') != std::string::npos)
              node_names[signal].insert(node_names[signal].begin(),'\\');
          }
          else
          {
            node_names[signal] = fmt::format( "lo{}", latch_index );
          }
        }
        latch_index++;
        ros.push_back(node_names[signal]);
      }
    } );

    latch_index = 1u;
    ntk.foreach_po( [&]( auto const& po, auto index ) {
      if(index < ntk.num_pos() - ntk.num_latches())
      {
        if constexpr ( has_has_output_name_v<Ntk> && has_get_output_name_v<Ntk> )
        {  
          output_names[index] = ntk.has_output_name( index ) ? ntk.get_output_name( index ) : fmt::format( "po{0:0{1}}", index, digitsOut );
          if(output_names[index].find('[') != std::string::npos || output_names[index].find('[') != std::string::npos)
            output_names[index].insert(output_names[index].begin(),'\\');
        }
        else
        {
          output_names[index] = fmt::format( "po{0:0{1}}", index, digitsOut );
        }
        ys.push_back(output_names[index]);
      }
      else
      {
        if(ntk.num_latches()>0) 
        {
          if constexpr ( has_has_output_name_v<Ntk> && has_get_output_name_v<Ntk> )
          {  
            ri_names[latch_index] = ntk.has_output_name( index ) ? ntk.get_output_name( index ) : fmt::format( "li{}", latch_index );
            if(ri_names[latch_index].find('[') != std::string::npos || ri_names[latch_index].find('[') != std::string::npos)
              ri_names[latch_index].insert(ri_names[latch_index].begin(),'\\');
          }
          else
          {
            ri_names[latch_index] = fmt::format( "li{}", latch_index );
          }
        }
        ris.push_back(ri_names[latch_index]);
        latch_index++;
      }
    } );

    if(ntk.num_latches()>0) {
      std::string clk = "clock";
        os << fmt::format( "module {}({} , {} , {} );\n", ps.module_name, clk, fmt::join(xs, " , "), fmt::join(ys, " , ") )
           << fmt::format( "  input {} ;\n", clk )
           << fmt::format( "  input {} ;\n", fmt::join(xs, " , ") )
           << fmt::format( "  output {} ;\n", fmt::join(ys, " , ") )
           << fmt::format( "  reg {} ;\n", fmt::join(ros, " , ") );
    }

    else {
        os << fmt::format( "module {}({} , {} );\n", ps.module_name, fmt::join(xs, " , "), fmt::join(ys, " , ") )
           << fmt::format( "  input {} ;\n", fmt::join(xs, " , ") )
           << fmt::format( "  output {} ;\n", fmt::join(ys, " , ") );
    }

    
    node_names[ntk.get_constant( false )] = "1'b0";
    if ( ntk.get_node( ntk.get_constant( false ) ) != ntk.get_node( ntk.get_constant( true ) ) )
    {
      node_names[ntk.get_constant( true )] = "1'b1";
    }

    topo_view ntk_topo{ntk};

    /* declare wires */
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
          os << fmt::format("new_n{}", ntk.node_to_index(n));
        }
      } );

      ntk.foreach_ri( [&](auto ri, auto index ) {
        os << ", ";
        os << ri_names[index + 1];
      } );
      os << ";\n";
    }
    ntk_topo.foreach_node( [&]( auto const& n ) {
      if ( ntk.is_constant( n ) || ntk.is_ci( n ) )
        return true;

      if ( ntk.is_and( n ) )
      {
        const auto [children, inv] = detail::format_fanin<2, Ntk>( ntk, n, node_names );
        os << fmt::format( "  assign new_n{} = {}{} & {}{} ;\n", ntk.node_to_index( n ),
                           inv[0], children[0], inv[1], children[1] );
      }
      else if ( ntk.is_or( n ) )
      {
        const auto [children, inv] = detail::format_fanin<2, Ntk>( ntk, n, node_names );
        os << fmt::format( "  assign new_n{} = {}{} | {}{} ;\n", ntk.node_to_index( n ),
                           inv[0], children[0], inv[1], children[1] );
      }
      else if ( ntk.is_xor( n ) )
      {
        const auto [children, inv] = detail::format_fanin<2, Ntk>( ntk, n, node_names );
        os << fmt::format( "  assign new_n{} = {}{} ^ {}{} ;\n", ntk.node_to_index( n ),
                           inv[0], children[0], inv[1], children[1] );
      }
      else if ( ntk.is_xor3( n ) )
      {
        const auto [children, inv] = detail::format_fanin<3, Ntk>( ntk, n, node_names );
        os << fmt::format( "  assign new_n{} = {}{} ^ {}{} ^ {}{} ;\n", ntk.node_to_index( n ),
                           inv[0], children[0], inv[1], children[1], inv[2], children[2] );
      }
      else if ( ntk.is_maj( n ) )
      {   
        signal<Ntk> first_child;
        ntk.foreach_fanin( n, [&]( auto const& f ) { first_child = f; return false; } );

        const auto [children, inv] = detail::format_fanin<3, Ntk>( ntk, n, node_names );
        if ( ntk.is_constant( ntk.get_node( first_child ) ) )
        {
          os << fmt::format( "  assign new_n{0} = {1}{3} {5} {2}{4} ;\n",
                             ntk.node_to_index( n ),
                             inv[1], inv[2], children[1], children[2],
                             ntk.is_complemented( first_child ) ? "|" : "&" );
        }
        else
        {
          os << fmt::format( "  assign new_n{0} = ( {1}{4} & {2}{5} ) | ( {1}{4} & {3}{6} ) | ( {2}{5} & {3}{6} );\n",
                             ntk.node_to_index( n ),
                             inv[0], inv[1], inv[2], children[0], children[1], children[2] );
        }
      }
      else
      {
        os << fmt::format( "  assign new_n{} = unknown gate;\n", ntk.node_to_index( n ) );
      }

      node_names[n] = fmt::format( "new_n{}", ntk.node_to_index( n ) );
      return true;
    } );

    latch_index = 1u;
    ntk.foreach_po( [&]( auto const& f, auto i ) {
      if(i < ntk.num_pos() - ntk.num_latches()){
        if(!ps.skip_feedthrough || (output_names[i] != node_names[f])){
          os << fmt::format( "  assign {} = {}{} ;\n", output_names[i], ntk.is_complemented( f ) ? "~" : "", node_names[f] );
        }
      }
      else{
        if(ntk.num_latches()>0){
          if(!ps.skip_feedthrough || (ri_names[latch_index] != node_names[f]))
            os << fmt::format( "  assign {} = {}{} ;\n", ri_names[latch_index], ntk.is_complemented( f ) ? "~" : "", node_names[f] );
          latch_index++;
        }
      }
      
    } );

    if(ntk.num_latches() > 0) {
      os << " always @ (posedge clock) begin\n";

      ntk.foreach_ro([&](auto const &ro, auto i) {
        auto ro_sig = ntk.make_signal(ro);
        auto ri_node = ntk.ro_to_ri(ro_sig);
        os << fmt::format("    {} <= {} ;\n", node_names[ro_sig], ri_names[ntk.ri_index(ri_node) + 1]);
      });

      os << " end\n";

      os << " initial begin\n";
      ntk.foreach_ro([&](auto const &f, auto i) {
        os << fmt::format("    {} <= 1'b0;\n", node_names[f]);
      });

      os << " end\n";
    }

    os << "endmodule\n"
       << std::flush;
  }

/*! \brief Writes network in structural Verilog format into a file
 *
 * **Required network functions:**
 * - `num_pis`
 * - `num_pos`
 * - `foreach_pi`
 * - `foreach_node`
 * - `foreach_fanin`
 * - `foreach_ri`
 * - `foreach_ro`
 * - `get_node`
 * - `get_constant`
 * - `is_constant`
 * - `is_ci`
 * - `is_and`
 * - `is_or`
 * - `is_xor`
 * - `is_xor3`
 * - `is_maj`
 * - `node_to_index`
 * - `ri_index`
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