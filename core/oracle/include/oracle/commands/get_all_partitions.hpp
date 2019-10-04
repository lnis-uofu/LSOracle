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
  class get_all_partitions_command : public alice::command{

    public:

      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit get_all_partitions_command( const environment::ptr& env )
          : command( env, "Exports every partition to Verilog files" ){
            opts.add_option( "--directory,directory", dir, "Directory to write Verilog files to" )->required();
            // add_flag("--cone,-c", "Writes out every cone of every partition");
            // add_flag("--verilog,-v", "Writes every partition or cone to a verilog file");
            add_flag("--mig,-m", "Write out all of the partitions of the sored MIG network");
      }

    protected:
      void execute(){
        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto ntk = *store<mig_ntk>().current(); 
            std::cout << "\n";
            if(!store<part_man_mig_ntk>().empty()){
              auto partitions = *store<part_man_mig_ntk>().current(); 
              for(int i = 0; i < partitions.get_part_num(); i++){
                std::vector<mockturtle::mig_network> parts;
                std::vector<std::string> filenames;
                int partition = i;
                auto part_outputs = partitions.get_part_outputs(i);
                std::cout << "Partition " << i << ":\n";
                std::cout << "Number of Logic Cones = " << part_outputs.size() << "\n";
                mkdir(dir.c_str(), 0777);
                
                oracle::partition_view<mig_names> part = partitions.create_part(ntk, partition);
                auto part_ntk = mockturtle::node_resynthesis<mockturtle::mig_network>(part, resyn_mig);
                
                // std::string filename = dir + "/" + ntk._storage->net_name + "_" + std::to_string(partition) + ".v";
                std::string filename = dir + "/top_" + std::to_string(partition) + ".v";
                filenames.push_back(filename);
                parts.push_back(part_ntk);
                
                assert(parts.size() == filenames.size());
                for(int j = 0; j < parts.size(); j++){
                    mockturtle::write_verilog(parts.at(j), filenames.at(j));
                }
                std::cout << "\n";
              }
            }
            else{
              std::cout << "Partitions have not been mapped\n";
            }
          }
          else{
              std::cout << "There is no MIG network stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto ntk = *store<aig_ntk>().current(); 
            std::cout << "\n";
            if(!store<part_man_aig_ntk>().empty()){
              auto partitions = *store<part_man_aig_ntk>().current(); 
              for(int i = 0; i < partitions.get_part_num(); i++){
                std::vector<mockturtle::aig_network> parts;
                std::vector<std::string> filenames;
                int partition = i;
                auto part_outputs = partitions.get_part_outputs(i);
                std::cout << "Partition " << i << ":\n";
                std::cout << "Number of Logic Cones = " << part_outputs.size() << "\n";
                mkdir(dir.c_str(), 0777);

                oracle::partition_view<aig_names> part = partitions.create_part(ntk, partition);
                auto part_ntk = mockturtle::node_resynthesis<mockturtle::aig_network>(part, resyn_aig);
                
                // std::string filename = dir + "/" + ntk._storage->net_name + "_" + std::to_string(partition) + ".v";
                std::string filename = dir + "/top_" + std::to_string(partition) + ".v";
                filenames.push_back(filename);
                parts.push_back(part_ntk);
                
                assert(parts.size() == filenames.size());
                for(int j = 0; j < parts.size(); j++){
                    mockturtle::write_verilog(parts.at(j), filenames.at(j));
                }
                std::cout << "\n";
              }
            }
            else{
                std::cout << "Partitions have not been mapped\n";
            }
          }
          else{
              std::cout << "There is no AIG network stored\n";
          }
        }
      }
    private:
        std::string dir{};
    };

  ALICE_ADD_COMMAND(get_all_partitions, "Output");
}
