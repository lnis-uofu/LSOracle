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
class mixed_command : public alice::command{

public:
  explicit mixed_command( const environment::ptr& env )
    : command( env, "Optimize partitions with AIG based-optimizer." ){
    opts.add_option( "--cnn_model,-c", cnn_model, "Trained CNN model for classification" )->required();
    opts.add_option( "--num_parts,-p", num_parts, "Number of partitions to create" )->required();
  }

protected:
  void execute(){

    //read AIG to generate hypergraph
    if(!store<mockturtle::aig_network>().empty()) {
      auto ntk = store<mockturtle::aig_network>().current();
      std::cout << "AIG initial size = " << ntk.num_gates() << std::endl;
      mockturtle::depth_view depth{ntk};
      std::cout << "AIG initial size = " << ntk.num_gates() << " and depth = " << depth.depth() << "\n";

      std::string folder = ntk._storage->net_name + "_final_" + std::to_string(num_parts);

      //char final [256];
      std::string final = ntk._storage->net_name + "/" + folder;

      mkdir(final.c_str(),0775);
      std::string filename = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_mix_final_" + std::to_string(num_parts) + ".v";
      std::string filename2 = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_mix_first_" + std::to_string(num_parts) + ".v";
      std::string filename4 = ntk._storage->net_name + "/" +  folder  + "/" + ntk._storage->net_name + "_mix_final_stats_" + std::to_string(num_parts)+ ".txt";

      oracle::partition_manager<mockturtle::aig_network> partitions_aig(ntk, num_parts);
      partitions_aig.run_classification(ntk, cnn_model);
      std::vector<int> aig_parts1 = partitions_aig.get_aig_parts();
      std::vector<int> mig_parts1 = partitions_aig.get_mig_parts();

      std::vector<pair<int, int>> pre_opt_nodes_depth;
      std::vector<pair<int, int>> post_opt_nodes_depth;

      //Deal with AIG partitions
      std::cout << "Total number of partitions for AIG 1 " << aig_parts1.size() << std::endl;
      std::cout << "Total number of partitions for MIG 1 " << mig_parts1.size() << std::endl;

      for (int i = 0; i < aig_parts1.size(); i++) {
        oracle::partition_view<mockturtle::aig_network> part_aig = partitions_aig.create_part(ntk, aig_parts1.at(i));

        std::cout << "\nPartition " << i << "\n";
        mockturtle::depth_view part_depth{part_aig};
        std::cout << "Partition size = " << part_aig.num_gates() << " and depth = " << part_depth.depth() << "\n";
        pre_opt_nodes_depth.push_back(std::make_pair<int, int>(part_aig.num_gates(), part_depth.depth()));


        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;

        auto aig_opt = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );

        mockturtle::aig_script aigopt;
        auto aig = aigopt.run(aig_opt);

        mockturtle::depth_view part_aig_depth{aig};
        std::cout << "Post optimization part size = " << aig.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";

        post_opt_nodes_depth.push_back(std::make_pair<int, int>(aig.num_gates(), part_aig_depth.depth()));

        partitions_aig.synchronize_part(part_aig, aig, ntk);
      }

      partitions_aig.connect_outputs(ntk);
      auto ntk_final = mockturtle::cleanup_dangling(ntk);

      mockturtle::depth_view depth_final{ntk_final};

      std::cout << "Final AIG size = " << ntk_final.num_gates() << " and depth = " << depth_final.depth() << "\n";
      //mockturtle::write_verilog(ntk, filename);

      oracle::partition_manager<mockturtle::aig_network> tmp(ntk_final, num_parts);

      tmp.run_classification(ntk_final, cnn_model);

      mockturtle::direct_resynthesis<mockturtle::mig_network> convert_mig;

      auto mig = mockturtle::node_resynthesis<mockturtle::mig_network>(ntk_final, convert_mig);
      std::cout << "Initial MIG size = " << mig.num_gates() << "\n";

      oracle::partition_manager<mockturtle::mig_network> partitions_mig(mig, num_parts);

      std::vector<int> aig_parts2 = tmp.get_aig_parts();
      std::vector<int> mig_parts2 = tmp.get_mig_parts();

      //Deal with AIG partitions
      std::cout << "Total number of partitions for AIG 2 " << aig_parts2.size() << std::endl;
      std::cout << "Total number of partitions for MIG 2 " << mig_parts2.size() << std::endl;
      for (int i = 0; i < mig_parts2.size(); i++) {
        oracle::partition_view<mockturtle::mig_network> part_mig = partitions_mig.create_part(mig, mig_parts2.at(i));
        mockturtle::mig_npn_resynthesis resyn_mig;

        std::cout << "\nPartition " << i << "\n";
        mockturtle::depth_view part_depth{part_mig};
        std::cout << "Partition size = " << part_mig.num_gates() << " and depth = " << part_depth.depth() << "\n";

        pre_opt_nodes_depth.push_back(std::make_pair<int, int>(part_mig.num_gates(), part_depth.depth()));

        auto mig_opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part_mig, resyn_mig);
        mockturtle::mig_script migopt;

        mig_opt = migopt.run(mig_opt);

        mockturtle::depth_view opt_mig_depth{mig_opt};

        post_opt_nodes_depth.push_back(std::make_pair<int, int>(mig_opt.num_gates(), opt_mig_depth.depth()));

        std::cout << "Post optimization part size = " << mig_opt.num_gates() << " and depth = "
                  << opt_mig_depth.depth()
                  << "\n";

        partitions_mig.synchronize_part(part_mig, mig_opt, mig);
      }

      partitions_mig.connect_outputs(mig);
      mig = mockturtle::cleanup_dangling(mig);

      mockturtle::depth_view final_mig{mig};

      std::cout << "Final MIG size = " << mig.num_gates() << " and depth = " << final_mig.depth() << "\n";

      mockturtle::write_verilog(mig, filename);

      std::string mix_final_stats = ntk._storage->net_name + "/" +  folder + "/" + "mix_final_stats.txt";
      std::ofstream stats(mix_final_stats.c_str());
      if(stats.is_open()){
         stats << mig.num_gates() << " " << final_mig.depth();
         stats.close();
      }

      //save no-optimized partitions info
      std::string no_opt_nodes_depth_name = ntk._storage->net_name + "/" +  folder + "/" + "no_opt_nodes_depth_mix.txt";
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
        std::string opt_nodes_depth_name = ntk._storage->net_name + "/" +  folder + "/" + "opt_nodes_depth_mix.txt";
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
      std::cout << "There is no stored AIG network\n";
    }
  }

private:
  std::string filename{};
  int num_parts = 0;
  std::string cnn_model{};
};

ALICE_ADD_COMMAND(mixed, "Optimization");
}
