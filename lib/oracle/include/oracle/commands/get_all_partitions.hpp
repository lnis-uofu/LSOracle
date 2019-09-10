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
          if(!store<mockturtle::mig_network>().empty()){
            auto ntk = store<mockturtle::mig_network>().current(); 
            std::cout << "\n";
            if(!store<oracle::partition_manager<mockturtle::mig_network>>().empty()){
              auto partitions = store<oracle::partition_manager<mockturtle::mig_network>>().current(); 
              for(int i = 0; i < partitions.get_part_num(); i++){
                std::vector<mockturtle::mig_network> parts;
                std::vector<std::string> filenames;
                int partition = i;
                auto part_outputs = partitions.get_part_outputs(i);
                std::cout << "Partition " << i << ":\n";
                std::cout << "Number of Logic Cones = " << part_outputs.size() << "\n";
                mkdir(dir.c_str(), 0777);
                
                oracle::partition_view<mockturtle::mig_network> part = partitions.create_part(ntk, partition);
                auto part_ntk = mockturtle::node_resynthesis<mockturtle::mig_network>(part, resyn_mig);
                
                std::string filename = dir + "/" + ntk._storage->net_name + "_" + std::to_string(partition) + ".v";;
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
          if(!store<mockturtle::aig_network>().empty()){
            auto ntk = store<mockturtle::aig_network>().current(); 
            std::cout << "\n";
            if(!store<oracle::partition_manager<mockturtle::aig_network>>().empty()){
              auto partitions = store<oracle::partition_manager<mockturtle::aig_network>>().current();
              mockturtle::node_map<std::string, mockturtle::aig_network> node_names( ntk );
              mockturtle::node_map<std::string, mockturtle::aig_network> input_names( ntk );
              std::string toplevel_module = std::filesystem::path(ntk._storage->net_name).filename();
              std::string toplevel_file = dir + "/" + toplevel_module + ".v"; 
              for(int i = 0; i < partitions.get_part_num(); i++){
                std::vector<mockturtle::aig_network> parts;
                std::vector<std::string> filenames;
                int partition = i;
                auto part_outputs = partitions.get_part_outputs(i);
                std::cout << "Partition " << i << ":\n";
                std::cout << "Number of Logic Cones = " << part_outputs.size() << "\n";
                mkdir(dir.c_str(), 0777);

                oracle::partition_view<mockturtle::aig_network> part = partitions.create_part(ntk, partition);
                auto part_ntk = mockturtle::node_resynthesis<mockturtle::aig_network>(part, resyn_aig);
                
                std::string modulename = std::filesystem::path( ntk._storage->net_name + "_" + std::to_string(partition) ).filename();
                std::string filename = dir + "/" + ntk._storage->net_name + "_" + std::to_string(partition) + ".v";
                filenames.push_back(filename);
                parts.push_back(part_ntk);

                if ( part_ntk.num_pos() == 0 )
                    continue;
                
                assert(parts.size() == filenames.size());
                for(int j = 0; j < parts.size(); j++){
                    mockturtle::write_verilog(parts.at(j), filenames.at(j), modulename);
                }

                if (i == 0)
                    mockturtle::write_toplevel_verilog(ntk, partitions, toplevel_file, node_names, input_names, toplevel_module);
                
                mockturtle::call_submodule( ntk, part_ntk, toplevel_file, modulename, i, part, node_names, input_names);

                std::cout << "\n";
              }
              std::ofstream os( toplevel_file.c_str(), std::ofstream::app);
              os << "endmodule\n"
                 << std::flush;

              os.close();
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
