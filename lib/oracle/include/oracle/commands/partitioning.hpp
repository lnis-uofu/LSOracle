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
  class partitioning_command : public alice::command{

    public:
      explicit partitioning_command( const environment::ptr& env )
        : command( env, "Partitionins current network using k-means hypergraph partitioner" ) {

          opts.add_option( "--num,num", num_partitions, "Number of desired partitions" )->required();
          add_flag("--mig,-m", "Partitions stored MIG network (AIG network is default)");
        }

    protected:
      void execute(){

        if(is_set("mig")){
          if(!store<mockturtle::mig_network>().empty()){
            std::cout << "Partitioning stored MIG network\n";
            auto ntk = store<mockturtle::mig_network>().current();
            oracle::partition_manager<mockturtle::mig_network> partitions(ntk, num_partitions);
            store<oracle::partition_manager<mockturtle::mig_network>>().extend() = partitions;
          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<mockturtle::aig_network>().empty()){
            std::cout << "Partitioning stored AIG network\n";
            auto ntk = store<mockturtle::aig_network>().current();
            oracle::partition_manager<mockturtle::aig_network> partitions(ntk, num_partitions); 
            store<oracle::partition_manager<mockturtle::aig_network>>().extend() = partitions;
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
    private:
      int num_partitions{};
  };

  ALICE_ADD_COMMAND(partitioning, "Partitioning");
}
