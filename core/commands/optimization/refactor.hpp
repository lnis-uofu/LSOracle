#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>
#include <mockturtle/algorithms/akers_synthesis.hpp>

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
            mockturtle::akers_resynthesis<mockturtle::mig_network> resyn;
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
            mockturtle::bidecomposition_resynthesis<mockturtle::aig_network> fallback;
            mockturtle::dsd_resynthesis<mockturtle::aig_network, decltype( fallback )> resyn( fallback );
            auto& ntk = *store<aig_ntk>().current(); 
            mockturtle::refactoring( ntk, resyn, ps );
            ntk = mockturtle::cleanup_dangling(ntk);
          }
          else{
            std::cout << "There is no AIG network stored\n";
          }
        }
      }
    };

  ALICE_ADD_COMMAND(refactor, "Delay");
}
