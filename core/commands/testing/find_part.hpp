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
  class find_part_command : public alice::command{

    public:
        explicit find_part_command( const environment::ptr& env )
                : command( env, "Classify partitions and perform corresponding optimization" ){

            opts.add_option( "--node,-n", nodeIdx, "Node to find partition of" )->required();
            add_flag("--mig,-m", "Look at stored MIG");
        }

    protected:
      void execute(){
        int partition = 0;

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){

            auto ntk = *store<mig_ntk>().current();

            if(!store<part_man_mig_ntk>().empty()){
              
              auto partitions = *store<part_man_mig_ntk>().current();

              mockturtle::mig_network::node node_to_find = ntk.node_to_index(nodeIdx);
              int num_partitions = partitions.get_part_num();
              for(int i = 0; i < num_partitions; i++){
                oracle::partition_view<mig_names> part = partitions.create_part(ntk, i);
                auto nodes = part.get_node_list();
                if(std::find(nodes.begin(), nodes.end(), node_to_find) != nodes.end()){
                  std::cout << "Found in partition " << i << " ";
                  if(part.is_pi(node_to_find)){
                    std::cout << "Input\n";
                  }
                  else if(part.is_po(node_to_find)){
                    std::cout << "Output\n";
                  }
                  else if(part.is_pi(node_to_find) && part.is_po(node_to_find)){
                    std::cout << "Input and Output\n";
                  }
                  else{
                    std::cout << "\n";
                  }
                }
              }
          
            }
            else{
                std::cout << "MIG not partitioned yet\n";
            }
          }
          else{
            std::cout << "No MIG stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){

            auto ntk = *store<aig_ntk>().current();

            if(!store<part_man_aig_ntk>().empty()){

              auto partitions = *store<part_man_aig_ntk>().current();

              mockturtle::aig_network::node node_to_find = ntk.node_to_index(nodeIdx);
              int num_partitions = partitions.get_part_num();
              for(int i = 0; i < num_partitions; i++){
                oracle::partition_view<aig_names> part = partitions.create_part(ntk, i);
                auto nodes = part.get_node_list();
                if(std::find(nodes.begin(), nodes.end(), node_to_find) != nodes.end()){
                  std::cout << "Found in partition " << i << " ";
                  if(part.is_pi(node_to_find)){
                    std::cout << "Input\n";
                  }
                  else if(part.is_po(node_to_find)){
                    std::cout << "Output\n";
                  }
                  else if(part.is_pi(node_to_find) && part.is_po(node_to_find)){
                    std::cout << "Input and Output\n";
                  }
                  else{
                    std::cout << "\n";
                  }
                }
              }
          
            }
            else{
                std::cout << "AIG not partitioned yet\n";
            }
          }
          else{
            std::cout << "No AIG stored\n";
          }
        }
        
      }
    private:
        int nodeIdx = 0;
    };

  ALICE_ADD_COMMAND(find_part, "Testing");
}