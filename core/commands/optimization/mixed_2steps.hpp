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
  class mixed_2step_command : public alice::command{

    public:
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit mixed_2step_command( const environment::ptr& env )
        : command( env, "Performs AIG optimization on corresponding partitions and then repartition for MIG optimization" ){
        opts.add_option( "--nn_model,-c", nn_model, "Trained neural network model for classification" );
        opts.add_option( "--num_parts,-p", num_parts, "Number of partitions to create" )->required();
        opts.add_option( "--out,-o", out_file, "Verilog output" )->required();
        add_flag("--brute,-b", "Uses a brute force approach instead of classification");
      }

    protected:
      void execute(){

        //read AIG to generate hypergraph
        if(!store<aig_ntk>().empty()) {
          auto ntk = *store<aig_ntk>().current();
          std::cout << "AIG initial size = " << ntk.num_gates() << std::endl;
          mockturtle::depth_view depth{ntk};
          std::cout << "AIG initial size = " << ntk.num_gates() << " and depth = " << depth.depth() << "\n";

          mockturtle::mig_npn_resynthesis resyn_mig;
          mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;

          oracle::partition_manager<aig_names> partitions_aig(ntk, num_parts);

          std::vector<int> aig_parts1;
          std::vector<int> mig_parts1;

          if(is_set("brute")){

            for(int i = 0; i < num_parts; i++){
              oracle::partition_view<aig_names> part_aig = partitions_aig.create_part(ntk, i);

              auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
              mockturtle::depth_view part_aig_depth{opt_aig};
              std::cout << "aig part size = " << opt_aig.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";
              mockturtle::aig_script aigopt;
              opt_aig = aigopt.run(opt_aig);
              mockturtle::depth_view part_aig_opt_depth{opt_aig};
              int aig_opt_size = opt_aig.num_gates();
              int aig_opt_depth = part_aig_opt_depth.depth();
              std::cout << "optimized aig part size = " << aig_opt_size << " and depth = " << aig_opt_depth << "\n";

              auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( part_aig, resyn_mig );
              mockturtle::depth_view part_mig_depth{opt_mig};
              std::cout << "mig part size = " << opt_mig.num_gates() << " and depth = " << part_mig_depth.depth() << "\n";
              mockturtle::mig_script migopt;
              opt_mig = migopt.run(opt_mig);
              mockturtle::depth_view part_mig_opt_depth{opt_mig};
              int mig_opt_size = opt_mig.num_gates();
              int mig_opt_depth = part_mig_opt_depth.depth();
              std::cout << "optimized mig part size = " << mig_opt_size << " and depth = " << mig_opt_depth << "\n";

              if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
                std::cout << "AIG wins\n";
                aig_parts1.push_back(i);
              }
              else{
                std::cout << "MIG wins\n";
                mig_parts1.push_back(i);
              }
            }
          }
          else{
            if(!nn_model.empty()){
              partitions_aig.run_classification(ntk, nn_model);

              aig_parts1 = partitions_aig.get_aig_parts();
              mig_parts1 = partitions_aig.get_mig_parts();
            }
            else{
              std::cout << "Must include CNN model json file\n";
            }
          }
          //Deal with AIG partitions
          std::cout << "Total number of partitions for AIG 1 " << aig_parts1.size() << std::endl;
          std::cout << "Total number of partitions for MIG 1 " << mig_parts1.size() << std::endl;

          for (int i = 0; i < aig_parts1.size(); i++) {
            oracle::partition_view<aig_names> part_aig = partitions_aig.create_part(ntk, aig_parts1.at(i));

            std::cout << "\nPartition " << i << "\n";
            mockturtle::depth_view part_depth{part_aig};
            std::cout << "Partition size = " << part_aig.num_gates() << " and depth = " << part_depth.depth() << "\n";

            auto aig_opt = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );

            mockturtle::aig_script aigopt;
            auto aig = aigopt.run(aig_opt);

            mockturtle::depth_view part_aig_depth{aig};
            std::cout << "Post optimization part size = " << aig.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";

            partitions_aig.synchronize_part(part_aig, aig, ntk);
          }

          partitions_aig.connect_outputs(ntk);
          auto ntk_final = mockturtle::cleanup_dangling(ntk);

          mockturtle::depth_view depth_final{ntk_final};

          std::cout << "Final AIG size = " << ntk_final.num_gates() << " and depth = " << depth_final.depth() << "\n";

          oracle::partition_manager<aig_names> tmp(ntk_final, num_parts);

          std::vector<int> aig_parts2;
          std::vector<int> mig_parts2;
          if(is_set("brute")){

            for(int i = 0; i < num_parts; i++){
              oracle::partition_view<aig_names> part_aig = tmp.create_part(ntk_final, i);

              auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
              mockturtle::depth_view part_aig_depth{opt_aig};
              std::cout << "aig part size = " << opt_aig.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";
              mockturtle::aig_script aigopt;
              opt_aig = aigopt.run(opt_aig);
              mockturtle::depth_view part_aig_opt_depth{opt_aig};
              int aig_opt_size = opt_aig.num_gates();
              int aig_opt_depth = part_aig_opt_depth.depth();
              std::cout << "optimized aig part size = " << aig_opt_size << " and depth = " << aig_opt_depth << "\n";

              auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( part_aig, resyn_mig );
              mockturtle::depth_view part_mig_depth{opt_mig};
              std::cout << "mig part size = " << opt_mig.num_gates() << " and depth = " << part_mig_depth.depth() << "\n";
              mockturtle::mig_script migopt;
              opt_mig = migopt.run(opt_mig);
              mockturtle::depth_view part_mig_opt_depth{opt_mig};
              int mig_opt_size = opt_mig.num_gates();
              int mig_opt_depth = part_mig_opt_depth.depth();
              std::cout << "optimized mig part size = " << mig_opt_size << " and depth = " << mig_opt_depth << "\n";

              if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
                std::cout << "AIG wins\n";
                aig_parts2.push_back(i);
              }
              else{
                std::cout << "MIG wins\n";
                mig_parts2.push_back(i);
              }
            }

          }
          else{
            if(!nn_model.empty()){
              tmp.run_classification(ntk_final, nn_model);

              aig_parts2 = tmp.get_aig_parts();
              mig_parts2 = tmp.get_mig_parts();
            }
            else{
              std::cout << "Must include CNN model json file\n";
            }

          }

          mockturtle::direct_resynthesis<mockturtle::mig_network> convert_mig;

          auto mig = mockturtle::node_resynthesis<mockturtle::mig_network>(ntk_final, convert_mig);
          std::cout << "Initial MIG size = " << mig.num_gates() << "\n";

          oracle::partition_manager<mig_names> partitions_mig(mig, num_parts);

          //Deal with AIG partitions
          std::cout << "Total number of partitions for AIG 2 " << aig_parts2.size() << std::endl;
          std::cout << "Total number of partitions for MIG 2 " << mig_parts2.size() << std::endl;
          for (int i = 0; i < mig_parts2.size(); i++) {
            oracle::partition_view<mig_names> part_mig = partitions_mig.create_part(mig, mig_parts2.at(i));

            std::cout << "\nPartition " << i << "\n";
            mockturtle::depth_view part_depth{part_mig};
            std::cout << "Partition size = " << part_mig.num_gates() << " and depth = " << part_depth.depth() << "\n";

            auto mig_opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part_mig, resyn_mig);
            mockturtle::mig_script migopt;

            mig_opt = migopt.run(mig_opt);

            mockturtle::depth_view opt_mig_depth{mig_opt};

            std::cout << "Post optimization part size = " << mig_opt.num_gates() << " and depth = "
                      << opt_mig_depth.depth()
                      << "\n";

            partitions_mig.synchronize_part(part_mig, mig_opt, mig);
          }

          partitions_mig.connect_outputs(mig);
          mig = mockturtle::cleanup_dangling(mig);

          mockturtle::depth_view final_mig{mig};

          std::cout << "new ntk size = " << mig.num_gates() << " and depth = " << final_mig.depth() << "\n";
          std::cout << "Finished optimization\n";

          mockturtle::write_verilog(mig, out_file);
        }
        else{
          std::cout << "There is no stored AIG network\n";
        }
      }

    private:
      int num_parts = 0;
      std::string nn_model{};
      std::string out_file{};
  };

  ALICE_ADD_COMMAND(mixed_2step, "Optimization");
}
