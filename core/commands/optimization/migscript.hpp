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
  class migscript_command : public alice::command{

    public:
        explicit migscript_command( const environment::ptr& env )
                : command( env, "Perform MIG based optimization script" ){

                opts.add_option( "--strategy", strategy, "Optimization strategy [0-2]" );
        }

    protected:
      void execute(){

        if(!store<mig_ntk>().empty()){
          auto& opt = *store<mig_ntk>().current();

          auto start = std::chrono::high_resolution_clock::now();
          mockturtle::depth_view mig_depth{opt};

          //DEPTH REWRITING
          std::cout << "MIG logic depth " << mig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

          switch(strategy){
            default:
            case 0:
            {
              oracle::mig_script migopt;
              opt = migopt.run(opt);
            }
            break;
            case 1:
            {
              oracle::mig_script2 migopt;
              opt = migopt.run(opt);
            }
            break;
            case 2:
            {
              oracle::mig_script3 migopt;
              opt = migopt.run(opt);
            }
            break;
          }
          
          mockturtle::depth_view new_mig_depth{opt};
          std::cout << "MIG logic depth " << new_mig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

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
    private:
        unsigned strategy{0u};
    };

  ALICE_ADD_COMMAND(migscript, "Optimization");
}