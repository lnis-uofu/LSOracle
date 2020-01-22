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
  class cut_rewriting_command : public alice::command{

    public:
        explicit cut_rewriting_command( const environment::ptr& env )
                : command( env, "Perform cut rewriting on stored network" ){

                opts.add_option( "--cut_size,-c", cut_size, "Cut size (4 is the default)" );
                add_flag("--mig,-m", "Performs cut rewriting on stored MIG network (AIG is default)");
        }

    protected:
      void execute(){

        mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
        mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto &ntk_mig = *store<mig_ntk>().current();
            mockturtle::mig_npn_resynthesis resyn;
            mockturtle::cut_rewriting_params ps;
            ps.cut_enumeration_ps.cut_size = cut_size;

            mockturtle::cut_rewriting(ntk_mig, resyn, ps);
            ntk_mig = mockturtle::cleanup_dangling(ntk_mig);
          }
          else{
            std::cout << "No MIG stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto &ntk_aig = *store<aig_ntk>().current();
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::cut_rewriting_params ps;
            ps.cut_enumeration_ps.cut_size = cut_size;

            mockturtle::cut_rewriting(ntk_aig, resyn, ps);
            // ntk_aig.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << "\n";
            // });
            // ntk_aig.foreach_gate([&](auto gate){
            //   std::cout << "Gate: " << gate << "\n";
            //   if(ntk_aig.is_po(gate))
            //     std::cout << "IS PO\n";
            //   ntk_aig.foreach_fanin(gate, [&](auto conn, auto i){
            //     std::cout << "child[" << i << "] = " << conn.index << "\n";
            //   });
            // });
            ntk_aig = mockturtle::cleanup_dangling(ntk_aig);

            mockturtle::depth_view depth{ntk_aig};
            std::cout << "Final ntk size = " << ntk_aig.num_gates() << " and depth = " << depth.depth() << "\n";
          }
          else{
            std::cout << "No AIG stored\n";
          }
        }
        
      }
    private:
        int cut_size = 4;
    };

  ALICE_ADD_COMMAND(cut_rewriting, "Optimization");
}