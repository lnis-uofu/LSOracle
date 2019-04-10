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
class mig_partition_command : public alice::command{

public:
  explicit mig_partition_command( const environment::ptr& env )
    : command( env, "Optimize partitions with AIG based-optimizer." ){
    opts.add_option( "--num_parts,p", num_parts, "Number of partitions to create" )->required();
  }

protected:
  void execute(){

    //read AIG to generate hypergraph
    if(!store<mockturtle::mig_network>().empty()) {
      auto ntk = store<mockturtle::mig_network>().current();
      const mockturtle::mig_network &gold_ntk = ntk;

      std::vector<int> part_num;

      auto part_size = num_parts;

        mockturtle::mig_network tmp_ntk = gold_ntk;

        num_parts = part_size;

        mockturtle::depth_view depth{gold_ntk};
        std::cout << "AIG initial size = " << gold_ntk.num_gates() << " and depth = " << depth.depth() << "\n";

        //create folder for different partitions size!
        //const char* homeDir = getenv ("");
        std::string folder = ntk._storage->net_name + "_final_" + std::to_string(part_size);

        //char final [256];
        std::string final = ntk._storage->net_name + "/" + folder;

        mkdir(final.c_str(),0775);
        std::string filename = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_mig_final_" + std::to_string(num_parts) + ".v";
        std::string filename2 = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_mig_first_" + std::to_string(num_parts) + ".v";
        std::string filename4 = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_mig_final_stats_" + std::to_string(num_parts)+ ".txt";


        oracle::partition_manager<mockturtle::mig_network> partitions_aig(ntk, num_parts);

        std::vector<pair<int, int>> pre_opt_nodes_depth;
        std::vector<pair<int, int>> post_opt_nodes_depth;

        //Deal with AIG partitions
        for (int i = 0; i < num_parts; i++) {
          oracle::partition_view<mockturtle::mig_network> part_aig = partitions_aig.create_part(tmp_ntk, i);
          mockturtle::mig_npn_resynthesis resyn_mig;

          mockturtle::write_verilog(tmp_ntk, filename2);

          std::cout << "\nPartition " << i << "\n";
          mockturtle::depth_view part_depth{part_aig};
          std::cout << "Partition size = " << part_aig.num_gates() << " and depth = " << part_depth.depth() << "\n";

          pre_opt_nodes_depth.push_back(std::make_pair<int, int>(part_aig.num_gates(), part_depth.depth()));

          auto mig_opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part_aig, resyn_mig);
          mockturtle::mig_script migopt;
          mig_opt = migopt.run(mig_opt);

          mockturtle::depth_view opt_mig_depth{mig_opt};

          std::cout << "Post optimization part size = " << mig_opt.num_gates() << " and depth = "
                    << opt_mig_depth.depth()
                    << "\n";

          post_opt_nodes_depth.push_back(std::make_pair<int, int>(mig_opt.num_gates(), opt_mig_depth.depth()));

          partitions_aig.synchronize_part(part_aig, mig_opt, tmp_ntk);
        }
        partitions_aig.connect_outputs(tmp_ntk);
        tmp_ntk = mockturtle::cleanup_dangling(tmp_ntk);

        mockturtle::depth_view depth_final{tmp_ntk};

        std::string mig_final_stats = ntk._storage->net_name + "/" +  folder + "/" + "mig_final_stats.txt";
		std::ofstream stats(mig_final_stats.c_str());
		if(stats.is_open()){
			stats << tmp_ntk.num_gates() << " " << depth_final.depth();
			stats.close();
		}

        std::cout << "Final AIG size = " << tmp_ntk.num_gates() << " and depth = " << depth_final.depth() << "\n";
        mockturtle::write_verilog(tmp_ntk, filename);

        //save no-optimized partitions info
        std::string no_opt_nodes_depth_name = ntk._storage->net_name + "/" +  folder + "/" + "no_opt_nodes_depth_mig.txt";
        std::ofstream myfile(no_opt_nodes_depth_name.c_str());

        if(myfile.is_open()){
          for (auto const &part : pre_opt_nodes_depth) {
            std::cout << "No opt size and depth: " << part.first << " " << part.second << std::endl;
            myfile << part.first
                   << " "
                   << part.second
                   << "\n";
          }
          myfile.close();
        } else std::cout << "Error opening file " << std::endl;

        //save optimized partitions info
        std::string opt_nodes_depth_name = ntk._storage->net_name + "/" +  folder + "/" + "opt_nodes_depth_mig.txt";
        std::ofstream file(opt_nodes_depth_name.c_str());
        if (file.is_open()) {
          for (auto const &part : post_opt_nodes_depth) {
            std::cout << "Opt size and depth: " << part.first << " " << part.second << std::endl;

            file << part.first
                 << " "
                 << part.second
                 << "\n";
          }

          file.close();
        }

        else std::cout << "Error opening second file " << std::endl;

        final.clear();
        folder.clear();
        filename.clear();
        no_opt_nodes_depth_name.clear();
        opt_nodes_depth_name.clear();
      }

      else{
        std::cout << "There is no stored MIG network\n";
      }
  }

private:
  std::string filename{};
  int num_parts = 0;
};

ALICE_ADD_COMMAND(mig_partition, "Optimization");
}
