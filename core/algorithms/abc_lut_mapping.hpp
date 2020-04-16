/*!
  \file abc_lut_mapping.hpp
  \brief LUT mapping an abc_view AIG network using abc's &if

  \author Scott Temple
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
#include <aig/gia/giaUtil.c>
#include <aig/gia/giaMini.c>
#include <aig/miniaig/minilut.h>
#include <map/if/if.h>

namespace mockturtle
{

void abc_lut_mapping( abc_view& ntk, lut_mapping_params const& ps = {})
{
  /*
  static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
  static_assert( has_size_v<Ntk>, "Ntk does not implement the size method" );
  static_assert( has_is_pi_v<Ntk>, "Ntk does not implement the is_pi method" );
  static_assert( has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
  static_assert( has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );
  static_assert( has_index_to_node_v<Ntk>, "Ntk does not implement the index_to_node method" );
  static_assert( has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
  static_assert( has_foreach_po_v<Ntk>, "Ntk does not implement the foreach_po method" );
  static_assert( has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );
  static_assert( has_fanout_size_v<Ntk>, "Ntk does not implement the fanout_size method" );
  static_assert( has_clear_mapping_v<Ntk>, "Ntk does not implement the clear_mapping method" );
  static_assert( has_add_to_mapping_v<Ntk>, "Ntk does not implement the add_to_mapping method" );
  static_assert( !StoreFunction || has_set_cell_function_v<Ntk>, "Ntk does not implement the set_cell_function method" );
*/

  int nLuts = 0;
  abc::If_Par_t abc_mapping_params;
  abc::If_Par_t* pabc_mapping_params = &abc_mapping_params;
  abc::Gia_Man_t *p = ntk.get_gia();
  abc::Gia_Man_t *pNew;
  abc::Gia_ManSetIfParsDefault(pabc_mapping_params);
  pabc_mapping_params->nLutSize = ps.cut_enumeration_ps.cut_size;
  pabc_mapping_params->nCutsMax = ps.cut_enumeration_ps.cut_limit;
  pNew = abc::Gia_ManPerformMapping(p, pabc_mapping_params);
  ntk.set_gia(pNew);
  std::cout << "AIG network mapped using if -K "<<ps.cut_enumeration_ps.cut_size <<"\n";
}

} /* namespace mockturtle */
