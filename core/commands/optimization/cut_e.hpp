#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>

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