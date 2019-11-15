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

  class create_level_part_command : public alice::command{

    public:
      explicit create_level_part_command( const environment::ptr& env )
          : command( env, "Test Partition technique proposed in '04 ICCAD Paper" ){

        add_flag("--mig,-m", "Use stored MIG (AIG network is default)");
      }

    protected:
      void execute(){

        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        if(is_set("mig")){
          if(!store<mockturtle::mig_network>().empty()){
            auto ntk = store<mockturtle::mig_network>().current();
            oracle::level_partition_manager<mockturtle::mig_network> level_parts(ntk);
            store<oracle::level_partition_manager<mockturtle::mig_network>>().extend() = level_parts;
          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<mockturtle::aig_network>().empty()){
            auto ntk = store<mockturtle::aig_network>().current();
            oracle::level_partition_manager<mockturtle::aig_network> level_parts(ntk);
            store<oracle::level_partition_manager<mockturtle::aig_network>>().extend() = level_parts;
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
        
    private:
      
    };

  ALICE_ADD_COMMAND(create_level_part, "CAD_proj");

  class gen_part_man_command : public alice::command{

    public:
      explicit gen_part_man_command( const environment::ptr& env )
          : command( env, "Test Partition technique proposed in '04 ICCAD Paper" ){

        opts.add_option( "--coarse,coarse", coarse, "Number of level partitions to combine into partition for optimization" )->required();
        add_flag("--mig,-m", "Use cad_proj with type MIG (AIG network is default)");
      }

    protected:
      void execute(){

        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        if(is_set("mig")){
          if(!store<mockturtle::mig_network>().empty()){
            auto ntk = store<mockturtle::mig_network>().current();
            if(!store<oracle::level_partition_manager<mockturtle::mig_network>>().empty()){
              auto level_parts = store<oracle::level_partition_manager<mockturtle::mig_network>>().current();
              oracle::partition_manager<mockturtle::mig_network> partitions = level_parts.generate_partitions(ntk, coarse);

              store<oracle::partition_manager<mockturtle::mig_network>>().extend() = partitions;

              int num_partitions = partitions.get_part_num();
              for(int i = 0; i < num_partitions; i++){
                std::cout << "Partition " << i << "\n";
                oracle::partition_view<mockturtle::mig_network> part = partitions.create_part(ntk, i);

                auto opt = mockturtle::node_resynthesis<mockturtle::mig_network>( part, resyn_mig );
                std::cout << "size = " << opt.size() << "\n";
                std::set<mockturtle::mig_network::node> inputs = partitions.get_part_inputs(i);
                typename std::set<mockturtle::mig_network::node>::iterator it;
                std::cout << "Inputs = ";
                for(it = inputs.begin(); it != inputs.end(); ++it){
                  std::cout << *it << " ";
                }
                std::cout << "\n";

                std::set<mockturtle::mig_network::node> outputs = partitions.get_part_outputs(i);
                std::cout << "Outputs = ";
                for(it = outputs.begin(); it != outputs.end(); ++it){
                  std::cout << *it << " ";
                }
                std::cout << "\n";
              }
            }
            else{
              std::cout << "Level parts not created for MIG\n";
            }
          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<mockturtle::aig_network>().empty()){
            auto ntk = store<mockturtle::aig_network>().current();
            if(!store<oracle::level_partition_manager<mockturtle::aig_network>>().empty()){
              auto level_parts = store<oracle::level_partition_manager<mockturtle::aig_network>>().current();
              oracle::partition_manager<mockturtle::aig_network> partitions = level_parts.generate_partitions(ntk, coarse);

              store<oracle::partition_manager<mockturtle::aig_network>>().extend() = partitions;

              // int num_partitions = partitions.get_part_num();
              // for(int i = 0; i < num_partitions; i++){
              //   // std::cout << "Partition " << i << "\n";
              //   oracle::partition_view<mockturtle::aig_network> part = partitions.create_part(ntk, i);

              //   auto opt = mockturtle::node_resynthesis<mockturtle::aig_network>( part, resyn_aig );
              //   // std::cout << "size = " << opt.size() << "\n";
              //   std::set<mockturtle::aig_network::node> inputs = partitions.get_part_inputs(i);
              //   typename std::set<mockturtle::aig_network::node>::iterator it;
              //   std::cout << "Inputs = ";
              //   for(it = inputs.begin(); it != inputs.end(); ++it){
              //     std::cout << *it << " ";
              //   }
              //   std::cout << "\n";

              //   std::set<mockturtle::aig_network::node> outputs = partitions.get_part_outputs(i);
              //   std::cout << "Outputs = ";
              //   for(it = outputs.begin(); it != outputs.end(); ++it){
              //     std::cout << *it << " ";
              //   }
              //   std::cout << "\n";
              // }
            }
            else{
              std::cout << "Level parts not created for AIG\n";
            }
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
        
    private:
      int coarse{};
    };

  ALICE_ADD_COMMAND(gen_part_man, "CAD_proj");
}