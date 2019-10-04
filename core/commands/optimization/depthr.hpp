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
  ALICE_COMMAND(depthr, "Optimization", "Logic depth oriented MIG rewriting"){
    if(!store<mig_ntk>().empty()){
      auto& mig = *store<mig_ntk>().current();
    	std::cout << "Mig gates " << mig.num_gates() << std::endl;

    	//to compute at level
    	mockturtle::depth_view mig_depth{mig};

    	std::cout << "Mig level " << mig_depth.depth() << " mig gates " << mig.num_gates() << std::endl;

    	mockturtle::mig_algebraic_depth_rewriting_params pm;
    	pm.aggressive;
    	mockturtle::mig_algebraic_depth_rewriting(mig_depth, pm);

    	mig = mockturtle::cleanup_dangling( mig );
    	std::cout << "Mig level after algebraic rewriting " << mig_depth.depth() << " Mig gates " << mig.num_gates() << std::endl;
    }
    else{
      std::cout << "There is not an MIG network stored.\n";
    }
  }
}