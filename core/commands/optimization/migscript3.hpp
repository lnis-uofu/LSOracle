#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

namespace alice
{
   ALICE_COMMAND(migscript3, "Optimization", "Exact NPN MIG rewriting") {
    if(!store<mig_ntk>().empty()){
      auto& opt = *store<mig_ntk>().current();

      auto start = std::chrono::high_resolution_clock::now();
      mockturtle::depth_view mig_depth{opt};

      //DEPTH REWRITING
      std::cout << "MIG logic depth " << mig_depth.depth() << " Majority nodes " << opt.num_gates() << std::endl;

      oracle::mig_script3 migopt;
      opt = migopt.run(opt);

      mockturtle::depth_view new_mig_depth{opt};
      std::cout << "MIG logic depth " << new_mig_depth.depth() << " Majority nodes " << opt.num_gates() << std::endl;

      std::cout << "Final ntk size = " << opt.num_gates() << " and depth = " << new_mig_depth.depth() << "\n";
      std::cout << "Area Delay Product = " << opt.num_gates() * new_mig_depth.depth() << "\n";
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      std::cout << "Full Optimization: " << duration.count() << "ms\n";
      std::cout << "Finished optimization\n";

    }
    else{
      std::cout << "There is not an MIG network stored.\n";
    }

  }
}