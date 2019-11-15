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
  class get_fanout_command : public alice::command{

    public:
        explicit get_fanout_command( const environment::ptr& env )
                : command( env, "Get the fanout list of a given node" ){

            opts.add_option( "--node,-n", nodeIdx, "Node to find fanout of" )->required();
            add_flag("--mig,-m", "Look at stored MIG");
        }

    protected:
      void execute(){
        int partition = 0;

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){

            auto ntk = *store<mig_ntk>().current();
            mockturtle::fanout_view fanout{ntk};
            std::cout << "Fanout of " << nodeIdx << " = {";
            fanout.foreach_fanout(ntk.index_to_node(nodeIdx), [&](auto const& p){
              std::cout << p << " ";
            });
            std::cout << "}\n";
          }
          else{
            std::cout << "No MIG stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){

            auto ntk = *store<aig_ntk>().current();
            mockturtle::fanout_view fanout{ntk};
            std::cout << "Fanout of " << nodeIdx << " = {";
            fanout.foreach_fanout(ntk.index_to_node(nodeIdx), [&](auto const& p){
              std::cout << p << " ";
            });
            std::cout << "}\n";
          }
          else{
            std::cout << "No AIG stored\n";
          }
        }
        
      }
    private:
        int nodeIdx = 0;
    };

  ALICE_ADD_COMMAND(get_fanout, "Testing");
}