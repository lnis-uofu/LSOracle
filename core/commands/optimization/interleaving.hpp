#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
  ALICE_COMMAND(interleaving, "Optimization", "NPN + depth MIG rewriting") {
    if(!store<mig_ntk>().empty()){
      auto& mig = *store<mig_ntk>().current();

      mockturtle::mig_npn_resynthesis resyn;
      mockturtle::cut_rewriting_params ps;

      ps.cut_enumeration_ps.cut_size = 4;

      mockturtle::cut_rewriting(mig, resyn, ps);
      mig = mockturtle::cleanup_dangling( mig );
    }
    else{
      std::cout << "There is not an MIG network stored.\n";
    }

  }
}