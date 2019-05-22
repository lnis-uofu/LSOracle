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

class test_combine_part_command : public alice::command{

    public:
        explicit test_combine_part_command( const environment::ptr& env )
                : command( env, "Brute force approach to finding best optimization methods" ){

                opts.add_option( "--cnn_model,-c", cnn_model, "Trained CNN model for classification" );
                opts.add_option( "--out,-o", out_file, "Verilog output" )->required();
                add_flag("--brute,-b", "Uses a brute force approach instead of classification");
                add_flag("--aig,-a", "Perform only AIG optimization on all partitions");
                add_flag("--mig,-m", "Perform only MIG optimization on all partitions");
        }

    protected:
        void execute(){

          mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
          mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;
          std::vector<int> aig_parts;
          std::vector<int> mig_parts;
          std::vector<int> comb_aig_parts;
          std::vector<int> comb_mig_parts;
          if(!store<mockturtle::aig_network>().empty()){

            auto ntk_aig = store<mockturtle::aig_network>().current();
            std::string file_base = ntk_aig._storage->net_name;
            // std::cout << "ntk_aig size = " << ntk_aig.size() << "\n";
            std::string net_name = ntk_aig._storage->net_name;
            if(!store<oracle::partition_manager<mockturtle::aig_network>>().empty()){
              auto partitions_aig = store<oracle::partition_manager<mockturtle::aig_network>>().current();
              int num_parts = partitions_aig.get_part_num();

              if(is_set("aig")){
                for(int i = 0; i < num_parts; i++){
                  aig_parts.push_back(i);
                }
              }
              else if(is_set("mig")){
                for(int i = 0; i < num_parts; i++){
                  mig_parts.push_back(i);
                }
              }
              else if(is_set("brute")){

                for(int i = 0; i < num_parts; i++){
                  oracle::partition_view<mockturtle::aig_network> part_aig = partitions_aig.create_part(ntk_aig, i);

                  auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
                  mockturtle::depth_view part_aig_depth{opt_aig};
                  // std::cout << "aig part size = " << opt_aig.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";
                  mockturtle::aig_script aigopt;
                  opt_aig = aigopt.run(opt_aig);
                  mockturtle::depth_view part_aig_opt_depth{opt_aig};
                  int aig_opt_size = opt_aig.num_gates();
                  int aig_opt_depth = part_aig_opt_depth.depth();
                  // std::cout << "optimized aig part size = " << aig_opt_size << " and depth = " << aig_opt_depth << "\n";

                  auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( part_aig, resyn_mig );
                  mockturtle::depth_view part_mig_depth{opt_mig};
                  // std::cout << "mig part size = " << opt_mig.num_gates() << " and depth = " << part_mig_depth.depth() << "\n";
                  mockturtle::mig_script migopt;
                  opt_mig = migopt.run(opt_mig);
                  mockturtle::depth_view part_mig_opt_depth{opt_mig};
                  int mig_opt_size = opt_mig.num_gates();
                  int mig_opt_depth = part_mig_opt_depth.depth();
                  // std::cout << "optimized mig part size = " << mig_opt_size << " and depth = " << mig_opt_depth << "\n";

                  if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
                    // std::cout << "AIG wins\n";
                    aig_parts.push_back(i);
                  }
                  else{
                    // std::cout << "MIG wins\n";
                    mig_parts.push_back(i);
                  }
                }

              }
              else{
                if(!cnn_model.empty()){
                  partitions_aig.run_classification(ntk_aig, cnn_model);

                  aig_parts = partitions_aig.get_aig_parts();
                  mig_parts = partitions_aig.get_mig_parts();
                }
                else{
                  std::cout << "Must include CNN model json file\n";
                }

              }
              
              std::cout << "Scheduled optimization:\n";
              std::cout << "AIG Optimization = {";
              for(int i = 0; i < aig_parts.size(); i++){
                std::cout << aig_parts.at(i) << " ";
              }
              std::cout << "}\n";
              std::cout << "MIG Optimization = {";
              for(int i = 0; i < mig_parts.size(); i++){
                std::cout << mig_parts.at(i) << " ";
              }
              std::cout << "}\n";
              std::vector<int> visited;
              std::unordered_map<int, int> comb_part;
              for(int i = 0; i < num_parts; i++){
                std::cout << "Partition " << i << "\n";
                std::cout << "Partition " << i << " Inputs: {";
                auto part_inputs = partitions_aig.get_part_inputs(i);
                auto part_outputs = partitions_aig.get_part_outputs(i);
                typename std::set<mockturtle::aig_network::node>::iterator it;
                for(it = part_inputs.begin(); it != part_inputs.end(); ++it){
                  std::cout << *it << " ";
                }
                std::cout << "}\n";
                std::cout << "Partition " << i << " Outputs: {";
                for(it = part_outputs.begin(); it != part_outputs.end(); ++it){
                  std::cout << *it << " ";
                }
                std::cout << "}\n";
                
                std::vector<int> parts_to_combine;
                for(it = part_outputs.begin(); it != part_outputs.end(); ++it){
                  // std::cout << "output = " << ntk_aig.node_to_index(*it) << "\n";
                  if(ntk_aig.is_po(*it)){
                      // std::cout << "PO\n";
                  }
                  else{
                    std::vector<int> input_partitions = partitions_aig.get_input_part(*it);
                    
                    std::cout << "In partition = {";
                    for(int j = 0; j < input_partitions.size(); j++){
                      std::cout << input_partitions.at(j) << " ";
                      if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                        if(std::find(aig_parts.begin(), aig_parts.end(), input_partitions.at(j)) != aig_parts.end()){
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), input_partitions.at(j)) == parts_to_combine.end()
                            && input_partitions.at(j) != i){
                            parts_to_combine.push_back(input_partitions.at(j));
                          }
                        }
                      }
                      else{
                        if(std::find(mig_parts.begin(), mig_parts.end(), input_partitions.at(j)) != mig_parts.end()){
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), input_partitions.at(j)) == parts_to_combine.end()
                            && input_partitions.at(j) != i){
                            parts_to_combine.push_back(input_partitions.at(j));
                          }
                        }
                      }
                    }
                    std::cout << "}\n";                            
                  }
                }
                if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                  std::cout << "AIG\n";
                }
                else{
                  std::cout << "MIG\n";
                }
                std::cout << "Partitions to combine current part with = {";
                for(int j = 0; j < parts_to_combine.size(); j++){
                  std::cout << parts_to_combine.at(j) << " ";
                }
                std::cout << "}\n";
                
                for(int j = 0; j < parts_to_combine.size(); j++){
                  int part_1 = 0;
                  int part_2 = 0;
                  if(std::find(visited.begin(), visited.end(), parts_to_combine.at(j)) == visited.end()){
                    part_1 = i;
                    part_2 = parts_to_combine.at(j);
                  }
                  else{
                    if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                      part_1 = parts_to_combine.at(j);
                      part_2 = i;
                    }
                  }

                  if(std::find(visited.begin(), visited.end(), part_2) == visited.end()){
                    std::unordered_map<int,int>::const_iterator got = comb_part.find (part_1);
                    if(got != comb_part.end()){
                      part_1 = got->second;
                    }
                    std::cout << "Combining " << part_1 << " with " << part_2 << "\n";
                    std::vector<std::set<mockturtle::aig_network::node>> combined_io = partitions_aig.combine_partitions(ntk_aig, part_1, part_2);
                    auto new_inputs = combined_io.at(0);
                    auto new_outputs = combined_io.at(1);
                    comb_part[part_2] = part_1;

                    std::cout << "New inputs = {";
                    typename std::set<mockturtle::aig_network::node>::iterator it;
                    for(it = new_inputs.begin(); it != new_inputs.end(); ++it){
                      std::cout << *it << " ";
                    }
                    std::cout << "}\n";
                    std::cout << "New outputs = {";
                    for(it = new_outputs.begin(); it != new_outputs.end(); ++it){
                      std::cout << *it << " ";
                    }
                    std::cout << "}\n";

                    partitions_aig.set_part_inputs(part_1, new_inputs);
                    partitions_aig.set_part_outputs(part_1, new_outputs);
                    if(std::find(aig_parts.begin(), aig_parts.end(), part_1) != aig_parts.end()){
                      if(std::find(comb_aig_parts.begin(), comb_aig_parts.end(), part_1) == comb_aig_parts.end()){
                        comb_aig_parts.push_back(part_1);
                      }
                    }
                    else{
                      if(std::find(comb_mig_parts.begin(), comb_mig_parts.end(), part_1) == comb_mig_parts.end()){
                        comb_mig_parts.push_back(part_1);
                      }
                    }

                    visited.push_back(part_2);     
                  }
                    
                }
                if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                  visited.push_back(i);
                  if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                    if(std::find(comb_aig_parts.begin(), comb_aig_parts.end(), i) == comb_aig_parts.end()){
                      comb_aig_parts.push_back(i);
                    }
                  }
                  else{
                    if(std::find(comb_mig_parts.begin(), comb_mig_parts.end(), i) == comb_mig_parts.end()){
                      comb_mig_parts.push_back(i);
                    }
                  }
                }
                  
              }

              std::cout << "New Scheduled optimization\n";
              std::cout << "AIG Optimization = {";
              for(int i = 0; i < comb_aig_parts.size(); i++){
                std::cout << comb_aig_parts.at(i) << " ";
              }
              std::cout << "}\n";
              std::cout << "MIG Optimization = {";
              for(int i = 0; i < comb_mig_parts.size(); i++){
                std::cout << comb_mig_parts.at(i) << " ";
              }
              std::cout << "}\n";

              mockturtle::mig_network ntk_mig = aig_to_mig(ntk_aig, 0); //mockturtle::node_resynthesis<mockturtle::mig_network>( ntk_aig, resyn_mig );
              oracle::partition_manager<mockturtle::mig_network> partitions_mig(ntk_mig, partitions_aig.get_all_part_connections(), 
                      partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(), partitions_aig.get_part_num());
              
              std::cout << "AIG Optimization\n";
              for(int i = 0; i < comb_aig_parts.size(); i++){
                std::cout << "Optimize partition " << comb_aig_parts.at(i) << "\n";
                oracle::partition_view<mockturtle::mig_network> part = partitions_mig.create_part(ntk_mig, comb_aig_parts.at(i));
                mockturtle::depth_view part_depth{part};
                std::cout << "part size = " << part.num_gates() << " and depth = " << part_depth.depth() << "\n";

                // auto opt = mockturtle::node_resynthesis<mockturtle::aig_network>(part, resyn_aig);
                auto opt_part = part_to_mig(part, 1);
                auto opt = mig_to_aig(opt_part);

                mockturtle::depth_view opt_part_depth{opt};
                std::cout << "part size after resynthesis = " << opt.num_gates() << " and depth = " << opt_part_depth.depth() << "\n";

                mockturtle::aig_script aigopt;
                opt = aigopt.run(opt);

                auto opt_mig = aig_to_mig(opt, 0);
                mockturtle::depth_view part_opt_depth{opt_mig};
                std::cout << "new part size = " << opt_mig.num_gates() << " and depth = " << part_opt_depth.depth() << "\n";

                partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
              }
              std::cout << "MIG Optimization\n";
              for(int i = 0; i < comb_mig_parts.size(); i++){
                std::cout << "Optimize partition " << comb_mig_parts.at(i) << "\n";
                oracle::partition_view<mockturtle::mig_network> part = partitions_mig.create_part(ntk_mig, comb_mig_parts.at(i));
                mockturtle::depth_view part_depth{part};
                std::cout << "part size = " << part.num_gates() << " and depth = " << part_depth.depth() << "\n";

                // auto opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part, resyn_mig);
                auto opt = part_to_mig(part, 1);

                mockturtle::depth_view opt_part_depth{opt};
                std::cout << "part size after resynthesis = " << opt.num_gates() << " and depth = " << opt_part_depth.depth() << "\n";
                
                mockturtle::mig_script migopt;
                opt = migopt.run(opt);
                
                mockturtle::depth_view part_opt_depth{opt};
                std::cout << "new part size = " << opt.num_gates() << " and depth = " << part_opt_depth.depth() << "\n";

                partitions_mig.synchronize_part(part, opt, ntk_mig);
              }
              std::cout << comb_aig_parts.size() << " AIGs and " << comb_mig_parts.size() << " MIGs\n";
              std::cout << "AIG partitions = {";
              for(int i = 0; i < comb_aig_parts.size(); i++){
                std::cout << comb_aig_parts.at(i) << " ";
              }
              std::cout << "}\n";
              std::cout << "MIG partitions = {";
              for(int i = 0; i < comb_mig_parts.size(); i++){
                std::cout << comb_mig_parts.at(i) << " ";
              }
              std::cout << "}\n";
              
              partitions_mig.connect_outputs(ntk_mig);
              
              mockturtle::depth_view ntk_before_depth2{ntk_mig};
              
              ntk_mig = mockturtle::cleanup_dangling( ntk_mig );
              mockturtle::depth_view ntk_depth2{ntk_mig};
              std::cout << "Final ntk size = " << ntk_mig.num_gates() << " and depth = " << ntk_depth2.depth() << "\n";
              std::cout << "Area Delay Product = " << ntk_mig.num_gates() * ntk_depth2.depth() << "\n";
              std::cout << "Finished optimization\n";
              store<mockturtle::mig_network>().extend() = ntk_mig;
              mockturtle::write_verilog(ntk_mig, out_file);     
            }
            else{
              std::cout << "AIG not partitioned\n";
            }           
        
        }
        else{
          std::cout << "No AIG stored\n";
        }
      }
    
    private:
        int num_parts = 0;
        std::string cnn_model{};
        std::string out_file{};
    };

    ALICE_ADD_COMMAND(test_combine_part, "Testing");
}