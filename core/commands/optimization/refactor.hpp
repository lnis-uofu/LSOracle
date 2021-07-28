#pragma once

#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
  class refactor_command : public alice::command{

    public:
      explicit refactor_command( const environment::ptr& env )
          : command( env, "Perform refactoring on stored network" ){
            add_flag("--zero,-z", "Allow zero-gain substitution");
            add_flag("--mig,-m", "Write out all of the partitions of the sored MIG network");
      }

    protected:
      void execute(){
        
        mockturtle::refactoring_params ps;
        if(is_set("zero"))
          ps.allow_zero_gain = true;

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            mockturtle::mig_npn_resynthesis resyn;
            ps.max_pis = 4;
            auto& ntk = *store<mig_ntk>().current(); 
            mockturtle::refactoring( ntk, resyn, ps );
            ntk = mockturtle::cleanup_dangling(ntk);
          }
          else{
            std::cout << "There is no MIG network stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto& ntk = *store<aig_ntk>().current(); 
            // mockturtle::shannon_resynthesis<mockturtle::aig_network> fallback;
            // mockturtle::dsd_resynthesis<mockturtle::aig_network, decltype( fallback )> resyn( fallback );
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            ps.max_pis = 4;
            mockturtle::refactoring( ntk, resyn, ps );
            ntk = mockturtle::cleanup_dangling(ntk);

            mockturtle::depth_view depth{ntk};
            std::cout << "Final ntk size = " << ntk.num_gates() << " and depth = " << depth.depth() << "\n";
          }
          else{
            std::cout << "There is no AIG network stored\n";
          }
        }
      }
    };

  ALICE_ADD_COMMAND(refactor, "Delay");
}
