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

class fpga_seed_partitioning_command : public alice::command{

  public:
    explicit fpga_seed_partitioning_command( const environment::ptr& env )
            : command( env, "Runs AIG optimization on entire circuit before partitioning and determining best MIG partitions" ){

            opts.add_option( "--net_delay,-d", net_delay, "Hyperparameter that controls the trade-off between net sharing and delay minimization" );
            opts.add_option( "--max_net,-n", max_net, "Controls the maxinum number of nets that could connect to a partition" );
            opts.add_option( "--num_pis,-p", num_pis, "Number of PIs constraint" );
            opts.add_option( "--num_int,-i", num_int, "Number of internal nodes constraint" );
            add_flag("--mig,-m", "Use fpga seed partitioning on stored MIG network (AIG is default)");
    }

  protected:
    void execute(){
      if(is_set("mig")){
        if(!store<mockturtle::mig_network>().empty()){
          auto ntk = store<mockturtle::mig_network>().current();
          oracle::fpga_seed_partitioner<mockturtle::mig_network> partitioner(ntk, net_delay, max_net, num_pis, num_int);
          oracle::partition_manager<mockturtle::mig_network> part_man = partitioner.create_part_man(ntk);
          store<oracle::partition_manager<mockturtle::mig_network>>().extend() = part_man;
          
        }
        else{
          std::cout << "MIG network not stored\n";
        }
      }
      else{
        if(!store<mockturtle::aig_network>().empty()){
          auto ntk = store<mockturtle::aig_network>().current();
          auto start = std::chrono::high_resolution_clock::now();
          oracle::fpga_seed_partitioner<mockturtle::aig_network> partitioner(ntk, net_delay, max_net, num_pis, num_int);
          auto stop = std::chrono::high_resolution_clock::now();
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
          std::cout << "Partitioning time: " << duration.count() << "ms\n";
          oracle::partition_manager<mockturtle::aig_network> part_man = partitioner.create_part_man(ntk);
          store<oracle::partition_manager<mockturtle::aig_network>>().extend() = part_man;
          
        }
        else{
          std::cout << "AIG network not stored\n";
        }
      }  
    }
  
  private:
    double net_delay = 0.0;
    double max_net = 0.0;
    int num_pis = 0;
    int num_int = 0;
  };

  ALICE_ADD_COMMAND(fpga_seed_partitioning, "Testing");


}