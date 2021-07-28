#pragma once
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
  ALICE_COMMAND( cut_e, "Optimization", "Performs Cut Enumeration on a stored AIG network"){
  	auto aig = *store<aig_ntk>().current();
  	const mockturtle::network_cuts cuts = cut_enumeration( aig );
  	//store<mockturtle::network_cuts>().extend() = cuts;
  	aig.foreach_node( [&]( auto node ) {
  			std::cout << cuts.cuts( aig.node_to_index( node ) ) << "\n";
  	} );
  }
}