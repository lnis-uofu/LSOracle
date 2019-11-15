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

class test_sort_fanout_command : public alice::command{

    public:
      explicit test_sort_fanout_command( const environment::ptr& env )
          : command( env, "Order the nodes by fanout count" ){

        add_flag("--mig,-m", "Use stored MIG (AIG network is default)");
      }

    protected:
      void execute(){
        if(is_set("mig")){
          if(!store<mockturtle::mig_network>().empty()){
            auto ntk = store<mockturtle::mig_network>().current();

            mockturtle::fanout_view<mockturtle::mig_network> fanout_ntk{ntk};
            // std::vector<mockturtle::mig_network::node> nodes = ntk._storage->nodes;

            ntk.foreach_node( [&]( auto node ) {
              std::cout << "Node = " << node << " fanout_size = " << fanout_ntk.fanout(node).size() << "\n";
            } );

            // std::sort(ntk._storage->nodes.begin(), ntk._storage->nodes.end(), less_than_fanout());

            // for(int i = 0; i < ntk._storage->nodes.size(); i++){
            //   std::cout << "Node = " << ntk._storage->nodes[i] << " fanout_size = " << fanout_ntk.fanout(ntk._storage->nodes[i]).size() << "\n";
            // }

          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<mockturtle::aig_network>().empty()){
            auto ntk = store<mockturtle::aig_network>().current();

            mockturtle::fanout_view<mockturtle::aig_network> fanout_ntk{ntk};
            // std::vector<mockturtle::mig_network::node> nodes = ntk._storage->nodes;

            ntk.foreach_node( [&]( auto node ) {
              std::cout << "Node = " << node << " fanout_size = " << fanout_ntk.fanout(node).size() << "\n";
            } );

            // std::sort(ntk._storage->nodes.begin(), ntk._storage->nodes.end(), less_than_fanout());

            // for(int i = 0; i < ntk._storage->nodes.size(); i++){
            //   std::cout << "Node = " << ntk._storage->nodes[i] << " fanout_size = " << fanout_ntk.fanout(ntk._storage->nodes[i]).size() << "\n";
            // }
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
        
    private:

      // struct less_than_fanout
      // {
      //     inline bool operator() (const mockturtle::mig_network::node& node1, const mockturtle::mig_network::node& node2)
      //     {
      //         return (fanout_ntk.fanout(node1).size() < fanout_ntk.fanout(node2).size());
      //     }
      // };
      
    };

  ALICE_ADD_COMMAND(test_sort_fanout, "Testing");
}