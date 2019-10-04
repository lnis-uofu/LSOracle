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
  class ntk_stats_command : public alice::command{

    public:
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit ntk_stats_command( const environment::ptr& env )
        : command( env, "Determines the number of Majority nodes and AND nodes internally in the stored MIG" ) {

        }

    protected:
      void execute(){
        
          if(!store<mig_ntk>().empty()){
            auto ntk = *store<mig_ntk>().current();

            int maj_num = 0;
            int and_num = 0;
            int input_num = 0;

            ntk.foreach_node([&]( auto node ){
              if(ntk.is_pi(node)){
                input_num++;
              }
              else if(ntk.is_constant(ntk.get_node(ntk._storage->nodes[node].children[0]))){
                and_num++;
              }
              else{
                maj_num++;
              }
            });

            std::cout << "MAJ nodes internally = " << maj_num << "\n";
            std::cout << "AND nodes internally = " << and_num << "\n";
            
          }
          else{
            std::cout << "MIG network not stored\n";
          }
        
      }
    private:
  };

  ALICE_ADD_COMMAND(ntk_stats, "Stats");
}
