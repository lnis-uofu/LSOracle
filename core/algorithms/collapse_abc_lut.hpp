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
  \file collapse_mapped.hpp
  \brief Collapses mapped network into k-LUT network

  \author Mathias Soeken
*/

#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <string>

#include <ez/direct_iterator.hpp>
#include <fmt/format.h>

#include <aig/gia/gia.h>
#include <aig/gia/giaAig.h>
#include <map/if/if.h>

#define MINI_LUT_NULL       (0x7FFFFFFF)
#define MINI_LUT_NULL2      (0x7FFFFFFE)
#define MINI_LUT_START_SIZE (0x000000FF)


namespace mockturtle
{

namespace detail
{

template<class NtkDest, class NtkSource>
class collapse_abc_impl
{
public:
  collapse_abc_impl( NtkSource const& ntk )
      : ntk( ntk )
  {
  }

  void run( NtkDest& dest )
  {
    abc::Gia_Man_t *p = ntk.get_gia();
    // abc::Mini_Lut_t miniLut;
    abc::Mini_Lut_t* pminiLut;
    pminiLut = Gia_ManToMiniLut(p);
    std::cout << "printing LUT stats\n";
    abc::Mini_LutPrintStats(pminiLut);
    int LutSize = abc::Mini_LutSize(pminiLut);
    int i = 0;
    int k = 0;
    int Fan = 0;

    std::unordered_map <int, mockturtle::signal<NtkDest>> signals;

    //foreach pi
    for (i = 2; i < abc::Mini_LutNodeNum(pminiLut); i++) if ( !abc::Mini_LutNodeIsPi(pminiLut, i) )   {} else {
      signals[i] = dest.create_pi();
      std::cout << "creating PI.  i = " << i <<" signals[i] = " << signals[i] << " dest size: " << dest.size() << "\n";
    }

    //foreach node
    for (i = 2; i < abc::Mini_LutNodeNum(pminiLut); i++) if ( !abc::Mini_LutNodeIsNode(pminiLut, i) ) {} else {
      std::vector<mockturtle::signal<NtkDest>> cell_children;
      
      //convert abc TT -> hex -> kitty
      unsigned * puTruth = abc::Mini_LutNodeTruth(pminiLut, i);
      abc::word Truth = ((abc::word)*puTruth << 32) | (abc::word)*puTruth; 
      abc::word * pTruth = LutSize < 6 ? &Truth : (abc::word *)puTruth;
      char tt_string[16];
      abc::Abc_TtWriteHexRev( tt_string, pTruth, LutSize); //abc's and LSOracles conventions are reversed, so you need the Rev version
      std::string tt_new(tt_string); //kitty wants c++ strings; presumably for iterator support
      //make the tt
      kitty::dynamic_truth_table result (LutSize);
      kitty::create_from_hex_string(result, tt_new);

      for (k = 0; (k < pminiLut->LutSize) && (Fan = abc::Mini_LutNodeFanin(pminiLut, i, k)) < MINI_LUT_NULL2; k++) {
        cell_children.push_back(signals[Fan]);
        std::cout << "push back child " << Fan <<"  corresponding to signal " << signals[Fan] << " array size now " << cell_children.size() <<"\n";
      }
      std::cout << "*******************\n" << tt_new << "\n*************************\n";
      signals[i] = dest.create_node(cell_children, result);
    }

    //foreach po
    for (i = 2; i < abc::Mini_LutNodeNum(pminiLut); i++) if ( !abc::Mini_LutNodeIsPo(pminiLut, i) )   {} else {
      Fan = abc::Mini_LutNodeFanin(pminiLut, i, 0);
      std::cout << "creating POs.  i = " << i << " Fan: " << Fan << "\n";
      dest.create_po(signals[Fan]);

    }

    //looping through created network for testing/debugging
    dest.foreach_node( [&](auto const& n){
      if ( dest.is_constant( n ) || dest.is_pi( n ) ){
          std::cout << n << " is constant or PI\n";
          return;
      }
      auto func = dest.node_function( n );
      std::cout << "node " << n << " func: " << kitty::to_hex(func) <<" \n";
    });

    //free memory
    abc::Mini_LutStop( pminiLut );
  }
private:
  NtkSource const& ntk;


  static inline int *       Gia_ObjLutFanins( abc::Gia_Man_t * p, int Id )         { return abc::Vec_IntEntryP(p->vMapping, abc::Vec_IntEntry(p->vMapping, Id)) + 1;}

};
} /* namespace detail */


template<class NtkDest, class NtkSource>
NtkDest collapse_abc_lut( NtkSource const& ntk )
{
/*
  static_assert( is_network_type_v<NtkSource>, "NtkSource is not a network type" );
  static_assert( is_network_type_v<NtkDest>, "NtkDest is not a network type" );

  static_assert( has_has_mapping_v<NtkSource>, "NtkSource does not implement the has_mapping method" );
  static_assert( has_num_gates_v<NtkSource>, "NtkSource does not implement the num_gates method" );
  static_assert( has_get_constant_v<NtkSource>, "NtkSource does not implement the get_constant method" );
  static_assert( has_get_node_v<NtkSource>, "NtkSource does not implement the get_node method" );
  static_assert( has_foreach_pi_v<NtkSource>, "NtkSource does not implement the foreach_pi method" );
  static_assert( has_foreach_po_v<NtkSource>, "NtkSource does not implement the foreach_po method" );
  static_assert( has_foreach_node_v<NtkSource>, "NtkSource does not implement the foreach_node method" );
  static_assert( has_foreach_cell_fanin_v<NtkSource>, "NtkSource does not implement the foreach_cell_fanin method" );
  static_assert( has_is_constant_v<NtkSource>, "NtkSource does not implement the is_constant method" );
  static_assert( has_is_pi_v<NtkSource>, "NtkSource does not implement the is_pi method" );
  static_assert( has_is_cell_root_v<NtkSource>, "NtkSource does not implement the is_cell_root method" );
  static_assert( has_cell_function_v<NtkSource>, "NtkSource does not implement the cell_function method" );
  static_assert( has_is_complemented_v<NtkSource>, "NtkSource does not implement the is_complemented method" );

  static_assert( has_get_constant_v<NtkDest>, "NtkDest does not implement the get_constant method" );
  static_assert( has_create_pi_v<NtkDest>, "NtkDest does not implement the create_pi method" );
  static_assert( has_create_node_v<NtkDest>, "NtkDest does not implement the create_node method" );
  static_assert( has_create_not_v<NtkDest>, "NtkDest does not implement the create_not method" );
*/

    detail::collapse_abc_impl<NtkDest, NtkSource> p( ntk );
    NtkDest dest;
    p.run( dest );
    return dest;

}

template<class NtkDest, class NtkSource>
bool collapse_abc_lut( NtkDest& dest, NtkSource const& ntk )
{
/*
  static_assert( is_network_type_v<NtkSource>, "NtkSource is not a network type" );
  static_assert( is_network_type_v<NtkDest>, "NtkDest is not a network type" );

  static_assert( has_has_mapping_v<NtkSource>, "NtkSource does not implement the has_mapping method" );
  static_assert( has_num_gates_v<NtkSource>, "NtkSource does not implement the num_gates method" );
  static_assert( has_get_constant_v<NtkSource>, "NtkSource does not implement the get_constant method" );
  static_assert( has_get_node_v<NtkSource>, "NtkSource does not implement the get_node method" );
  static_assert( has_foreach_pi_v<NtkSource>, "NtkSource does not implement the foreach_pi method" );
  static_assert( has_foreach_po_v<NtkSource>, "NtkSource does not implement the foreach_po method" );
  static_assert( has_foreach_node_v<NtkSource>, "NtkSource does not implement the foreach_node method" );
  static_assert( has_foreach_cell_fanin_v<NtkSource>, "NtkSource does not implement the foreach_cell_fanin method" );
  static_assert( has_is_constant_v<NtkSource>, "NtkSource does not implement the is_constant method" );
  static_assert( has_is_pi_v<NtkSource>, "NtkSource does not implement the is_pi method" );
  static_assert( has_is_cell_root_v<NtkSource>, "NtkSource does not implement the is_cell_root method" );
  static_assert( has_cell_function_v<NtkSource>, "NtkSource does not implement the cell_function method" );
  static_assert( has_is_complemented_v<NtkSource>, "NtkSource does not implement the is_complemented method" );

  static_assert( has_get_constant_v<NtkDest>, "NtkDest does not implement the get_constant method" );
  static_assert( has_create_pi_v<NtkDest>, "NtkDest does not implement the create_pi method" );
  static_assert( has_create_node_v<NtkDest>, "NtkDest does not implement the create_node method" );
  static_assert( has_create_not_v<NtkDest>, "NtkDest does not implement the create_not method" );
*/
  if ( !ntk.has_mapping() )
  {
    return false;
  }
  else
  {
    detail::collapse_abc_impl<NtkDest, NtkSource> p( ntk );
    p.run( dest );
    return true;
  }
}

} /* namespace mockturtle */
