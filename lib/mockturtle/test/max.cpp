#include <catch.hpp>

#include <iostream>

#include <mockturtle/algorithms/collapse_mapped.hpp>
#include <mockturtle/algorithms/lut_mapping.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/views/mapping_view.hpp>

#include <lorina/aiger.hpp>

using namespace mockturtle;

TEST_CASE( "learning mockturtle", "[max]" )
{
  aig_network aig;
  lorina::read_aiger( "~/epfl/benchmarks/epfl/arithmetic/sin.aig", aiger_reader( aig ) );

  std::cout << "gates = " << aig.num_gates() << "\n";

  // In order to perform LUT mapping, we need functions that allow to store
  // the mapping.  aig_network does not have these functions, but we can extend
  // it by wrapping it into a mapping_view.
  mapping_view<aig_network, true> mapped_aig{aig};
  lut_mapping<mapping_view<aig_network, true>, true>( mapped_aig );

  std::cout << "LUTs = " << mapped_aig.num_cells() << "\n";

  // Write the AIG (without LUTs) to a BENCH file
  write_bench( aig, "/tmp/aig.bench" );

  // Extract the k-LUT mapped network
  auto klut = collapse_mapped_network<klut_network>( mapped_aig );

  // Write the k-LUT netwrk to a BENCH file
  write_bench( aig, "/tmp/klut.bench" );
}
