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
class aig_partition_command : public alice::command{

public:
  explicit aig_partition_command( const environment::ptr& env )
    : command( env, "Optimize partitions with AIG based-optimizer." ){
    opts.add_option( "--num_parts,p", num_parts, "Number of partitions to create" )->required();
  }

protected:
  void execute(){

    //read AIG to generate hypergraph
    if(!store<mockturtle::aig_network>().empty()) {
      auto ntk = store<mockturtle::aig_network>().current();
      std::string ntk_name = ntk._storage->net_name;

      mockturtle::depth_view depth{ntk};
      std::cout << "AIG initial size = " << ntk.num_gates() << " and depth = " << depth.depth() << "\n";

      //create folder for different partitions size!
      //const char* homeDir = getenv ("");
      std::string folder = ntk._storage->net_name + "_final_" + std::to_string(num_parts);

      //char final [256];
      std::string final = ntk._storage->net_name + "/" + folder;

      mkdir(final.c_str(),0775);
      std::string filename = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_aig_final_" + std::to_string(num_parts) + ".v";
      std::string filename2 = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_aig_first_" + std::to_string(num_parts) + ".v";

      oracle::partition_manager<mockturtle::aig_network> partitions_aig(ntk, num_parts);

      std::vector<pair<int, int>> pre_opt_nodes_depth;
      std::vector<pair<int, int>> post_opt_nodes_depth;

      //Deal with AIG partitions
      for (int i = 0; i < num_parts; i++) {
        oracle::partition_view<mockturtle::aig_network> part_aig = partitions_aig.create_part(ntk, i);
        auto digits = std::strlen(std::to_string(num_parts).c_str());
        digits = digits - std::strlen(std::to_string(i).c_str());
//        char index[digits];
//        std::sprintf(index, "%0*d", digits, i);
        std::string index = std::string(digits, '0').append(std::to_string(i));
        std::string filename3 = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_aig_partition_" + index + "_" + std::to_string(num_parts) + ".v";

        std::cout << "\nPartition " << i << "\n";
        mockturtle::depth_view part_depth{part_aig};
        std::cout << "Partition size = " << part_aig.num_gates() << " and depth = " << part_depth.depth() << "\n";

        pre_opt_nodes_depth.push_back(std::make_pair<int, int> (part_aig.num_gates(), part_depth.depth()));

        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;

        mockturtle::write_verilog(ntk, filename2);

        auto aig_opt = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );

        //writes partition in Verilog format for ABC.
        mockturtle::write_verilog(aig_opt, filename3);

        mockturtle::aig_script aigopt;
        auto aig = aigopt.run(aig_opt);

        mockturtle::depth_view part_aig_depth{aig};
        std::cout << "Post optimization part size = " << aig.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";

        post_opt_nodes_depth.push_back(std::make_pair<int, int> (aig.num_gates(), part_aig_depth.depth()));

        partitions_aig.synchronize_part(part_aig, aig, ntk);
      }

      //partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(), partitions_aig.get_part_num());
      partitions_aig.connect_outputs(ntk);
      ntk = mockturtle::cleanup_dangling(ntk);

      mockturtle::depth_view depth_final{ntk};

	  std::string aig_final_stats = ntk_name + "/" +  folder + "/" + "aig_final_stats.txt";
    std::ofstream stats(aig_final_stats.c_str());
    if(stats.is_open()){
      stats << ntk.num_gates() << " " << depth_final.depth();
      stats.close();
    } else std::cout << "Error opening AIG final stats file. " << std::endl;


      std::cout << "Final AIG size = " << ntk.num_gates() << " and depth = " << depth_final.depth() << "\n";
      mockturtle::write_verilog(ntk, filename);

      //save no-optimized partitions info
      std::cout << "network name " << ntk_name << std::endl;

      std::string no_opt_nodes_depth_name = ntk_name + "/" + folder + "/" + "no_opt_nodes_depth_aig.txt";
      auto nom = no_opt_nodes_depth_name.c_str();
      std::cout << "Nome variable is " << nom << std::endl;
      std::ofstream myfile(no_opt_nodes_depth_name.c_str());

      if(myfile.is_open()){
        for (auto const &part : pre_opt_nodes_depth) {
          //std::cout << "No opt size and depth: " << part.first << " " << part.second << std::endl;
          myfile << part.first
                 << " "
                 << part.second
                 << "\n";
        }
        myfile.close();
      } else std::cout << "Error opening file " << std::endl;

      //save optimized partitions info
      std::string opt_nodes_depth_name = ntk_name + "/" +  folder + "/" + "opt_nodes_depth_aig.txt";
      std::ofstream file(opt_nodes_depth_name.c_str());
      if (file.is_open()) {
        for (auto const &part : post_opt_nodes_depth) {
          //std::cout << "Opt size and depth: " << part.first << " " << part.second << std::endl;

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
      std::cout << "There is no stored AIG network\n";
    }
  }

private:
  std::string filename{};
  int num_parts = 0;
};

ALICE_ADD_COMMAND(aig_partition, "Optimization");
}
