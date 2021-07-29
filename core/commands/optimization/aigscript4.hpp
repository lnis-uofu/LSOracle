#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

namespace alice
{
  ALICE_COMMAND(aigscript4, "Optimization", "Based on ABC resyn2 script without AIG balancing") {
    if(!store<aig_ntk>().empty()){
      auto& opt = *store<aig_ntk>().current();

      auto start = std::chrono::high_resolution_clock::now();
      mockturtle::depth_view aig_depth{opt};

      //DEPTH REWRITING
      std::cout << "AIG logic depth " << aig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

      oracle::aig_script4 aigopt;
      opt = aigopt.run(opt);

      mockturtle::depth_view new_aig_depth{opt};
      std::cout << "AIG logic depth " << new_aig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

      std::cout << "Final ntk size = " << opt.num_gates() << " and depth = " << new_aig_depth.depth() << "\n";
      std::cout << "Area Delay Product = " << opt.num_gates() * new_aig_depth.depth() << "\n";
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      std::cout << "Full Optimization: " << duration.count() << "ms\n";
      std::cout << "Finished optimization\n";

    }
    else{
      std::cout << "There is not an AIG network stored.\n";
    }

  }
}