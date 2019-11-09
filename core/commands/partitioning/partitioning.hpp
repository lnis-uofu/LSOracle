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
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit partitioning_command( const environment::ptr& env )
        : command( env, "Partitionins current network using k-means hypergraph partitioner" ) {

          opts.add_option( "--num,num", num_partitions, "Number of desired partitions" )->required();
          opts.add_option( "--config_direc,-c", config_direc, "Path to the configuration file for KaHyPar (../../core/test.ini is default)" );
          add_flag("--mig,-m", "Partitions stored MIG network (AIG network is default)");
        }

      void execute(int num_partitions, std::string config_direc, bool mig_flag = false){
        this->num_partitions = num_partitions;
        this->config_direc = config_direc;
        add_flag("--mig,-m", mig_flag, "Partitions stored MIG network (AIG network is default)");

        execute();
      }

    protected:
      void execute(){
        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            std::cout << "Partitioning stored MIG network\n";
            auto ntk = store<mig_ntk>().current();
            if(config_direc != ""){
              oracle::partition_manager<mig_names> partitions(*ntk, num_partitions, config_direc);
              store<part_man_mig_ntk>().extend() = std::make_shared<part_man_mig>( partitions );
            }
            else{
              oracle::partition_manager<mig_names> partitions(*ntk, num_partitions);
              store<part_man_mig_ntk>().extend() = std::make_shared<part_man_mig>( partitions );
            }
          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }

        else{
          if(!store<aig_ntk>().empty()){
            std::cout << "Partitioning stored AIG network\n";
            auto ntk = store<aig_ntk>().current();
            if(config_direc != ""){
              oracle::partition_manager<aig_names> partitions(*ntk, num_partitions, config_direc);
              store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>( partitions );
            }
            else{
              oracle::partition_manager<aig_names> partitions(*ntk, num_partitions);
              store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>( partitions );
            }
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
    private:
      int num_partitions{};
      std::string config_direc = "";
  };

  ALICE_ADD_COMMAND(partitioning, "Partitioning");
}
