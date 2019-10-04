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
  class optimization_command : public alice::command{

    public:
        explicit optimization_command( const environment::ptr& env )
                : command( env, "Perform Mixed Synthesis on Network after Partitioning" ){

                opts.add_option( "--nn_model,-n", nn_model, "Trained neural network model for classification" );
                opts.add_option( "--out,-o", out_file, "Verilog output" );
                add_flag("--high,-b", "Uses a high effort approach instead of classification");
                add_flag("--aig,-a", "Perform only AIG optimization on all partitions");
                add_flag("--mig,-m", "Perform only MIG optimization on all partitions");
                add_flag("--combine,-c", "Combine adjacent partitions that have been classified for the same optimization");
        }

    protected:
      void execute(){

        mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
        mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;

        std::vector<int> aig_parts;
        std::vector<int> mig_parts;
        std::vector<int> comb_aig_parts;
        std::vector<int> comb_mig_parts;

        if(!store<aig_ntk>().empty()){

          auto ntk_aig = *store<aig_ntk>().current();

          // std::string file_base = ntk_aig._storage->net_name;
          // std::string net_name = ntk_aig._storage->net_name;

          if(!store<part_man_aig_ntk>().empty()){
            
            auto start = std::chrono::high_resolution_clock::now();

            auto partitions_aig = *store<part_man_aig_ntk>().current();
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
            else if(is_set("high")){
              // std::cout << "determining best fit\n";
              for(int i = 0; i < num_parts; i++){
                oracle::partition_view<aig_names> part_aig = partitions_aig.create_part(ntk_aig, i);

                auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
                
                oracle::aig_script aigopt;
                opt_aig = aigopt.run(opt_aig);
                mockturtle::depth_view part_aig_opt_depth{opt_aig};
                int aig_opt_size = opt_aig.num_gates();
                int aig_opt_depth = part_aig_opt_depth.depth();

                auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( part_aig, resyn_mig );
                
                oracle::mig_script migopt;
                opt_mig = migopt.run(opt_mig);
                mockturtle::depth_view part_mig_opt_depth{opt_mig};
                int mig_opt_size = opt_mig.num_gates();
                int mig_opt_depth = part_mig_opt_depth.depth();

                if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
                  aig_parts.push_back(i);
                }
                else{
                  mig_parts.push_back(i);
                }
              }

            }
            else{
              if(!nn_model.empty()){
                partitions_aig.run_classification(ntk_aig, nn_model);
                aig_parts = partitions_aig.get_aig_parts();
				        mig_parts = partitions_aig.get_mig_parts();
			  }
              else{
                std::cout << "Must include Neural Network model json file\n";
              }

            }

            std::cout << aig_parts.size() << " AIGs and " << mig_parts.size() << " MIGs\n";

            if(is_set("combine")){
              std::vector<int> visited;
              std::unordered_map<int, int> comb_part;
              for(int i = 0; i < num_parts; i++){
                if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                  std::vector<int> parts_to_combine;
                  
                  std::set<int>::iterator conn_it;
                  std::set<int> conn_parts;
                  conn_parts = partitions_aig.get_connected_parts(ntk_aig, i);
                  if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                    for(conn_it = conn_parts.begin(); conn_it != conn_parts.end(); ++conn_it){
                      if(std::find(aig_parts.begin(), aig_parts.end(), *conn_it) != aig_parts.end()){
                        parts_to_combine.push_back(*conn_it);
                      }
                    }
                  }
                  else{
                    for(conn_it = conn_parts.begin(); conn_it != conn_parts.end(); ++conn_it){
                      if(std::find(mig_parts.begin(), mig_parts.end(), *conn_it) != mig_parts.end()){
                        parts_to_combine.push_back(*conn_it);
                      }
                    }
                  }

                  if(parts_to_combine.size() == 0){
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

                  for(int idx = 0; idx < parts_to_combine.size(); idx++){
                    int curr_part = parts_to_combine.at(idx);
                    int part_1 = 0;
                    int part_2 = 0;
                    if(std::find(visited.begin(), visited.end(), curr_part) == visited.end()){
                      part_1 = i;
                      part_2 = curr_part;
                    }
                    else{
                      if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                        part_1 = curr_part;
                        part_2 = i;
                      }
                    }

                    if(std::find(visited.begin(), visited.end(), part_2) == visited.end()){
                      std::unordered_map<int,int>::const_iterator got = comb_part.find (part_1);
                      if(got != comb_part.end()){
                        part_1 = got->second;
                      }
                      std::set<int> connected_parts1 = partitions_aig.get_connected_parts(ntk_aig, part_1);
                      std::set<int> connected_parts2 = partitions_aig.get_connected_parts(ntk_aig, part_2);
                      std::set<int>::iterator conn_it;
                      
                      std::vector<std::set<mockturtle::aig_network::node>> combined_io = partitions_aig.combine_partitions(ntk_aig, part_1, part_2);
                      
                      auto new_inputs = combined_io.at(0);
                      auto new_outputs = combined_io.at(1);
                      comb_part[part_2] = part_1;

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

                      connected_parts1 = partitions_aig.get_connected_parts(ntk_aig, part_1);
                      for(conn_it = connected_parts1.begin(); conn_it != connected_parts1.end(); ++conn_it){
                        if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), *conn_it) == parts_to_combine.end() && 
                            std::find(aig_parts.begin(), aig_parts.end(), *conn_it) != aig_parts.end() &&
                            std::find(visited.begin(), visited.end(), *conn_it) == visited.end()){

                            parts_to_combine.push_back(*conn_it);
                          }
                        }
                        else{
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), *conn_it) == parts_to_combine.end() && 
                            std::find(mig_parts.begin(), mig_parts.end(), *conn_it) != mig_parts.end() &&
                            std::find(visited.begin(), visited.end(), *conn_it) == visited.end()){

                            parts_to_combine.push_back(*conn_it);
                          }
                        }
                      } 
                    }
                    visited.push_back(i);
                  }
                }
              }
              aig_parts = comb_aig_parts;
              mig_parts = comb_mig_parts;
              std::cout << "Scheduled optimization after partition merging\n";
              std::cout << aig_parts.size() << " AIGs and " << mig_parts.size() << " MIGs\n";
            }

            auto ntk_mig = *aig_to_mig(ntk_aig, 1);
            oracle::partition_manager<mig_names> partitions_mig(ntk_mig, partitions_aig.get_all_part_connections(), 
                    partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(),
                    partitions_aig.get_all_partition_regs(), partitions_aig.get_all_partition_regin(), partitions_aig.get_part_num());

            for(int i = 0; i < aig_parts.size(); i++){
              
              oracle::partition_view<mig_names> part = partitions_mig.create_part(ntk_mig, aig_parts.at(i));
              mockturtle::depth_view part_depth{part};

              auto opt_part = *part_to_mig(part, 1);
              auto opt = *mig_to_aig(opt_part);

              mockturtle::depth_view opt_part_depth{opt};

              oracle::aig_script aigopt;
              opt = aigopt.run(opt);

              auto opt_mig = *aig_to_mig(opt, 0);
              mockturtle::depth_view part_opt_depth{opt_mig};

              partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
            }
            // std::cout << "MIG Optimization\n";
            for(int i = 0; i < mig_parts.size(); i++){
              
              oracle::partition_view<mig_names> part = partitions_mig.create_part(ntk_mig, mig_parts.at(i));
              mockturtle::depth_view part_depth{part};

              auto opt = *part_to_mig(part, 0);

              mockturtle::depth_view opt_part_depth{opt};
              
              oracle::mig_script migopt;
              opt = migopt.run(opt);
              
              mockturtle::depth_view part_opt_depth{opt};

              partitions_mig.synchronize_part(part, opt, ntk_mig);
            }
            
            partitions_mig.connect_outputs(ntk_mig);
            
            mockturtle::depth_view ntk_before_depth2{ntk_mig};
            
            ntk_mig = mockturtle::cleanup_dangling( ntk_mig );
            mockturtle::depth_view ntk_depth2{ntk_mig};
            std::cout << "Final ntk size = " << ntk_mig.num_gates() << " and depth = " << ntk_depth2.depth() << "\n";
            std::cout << "Final number of latches = " << ntk_mig.num_latches() << "\n";
            std::cout << "Area Delay Product = " << ntk_mig.num_gates() * ntk_depth2.depth() << "\n";
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            std::cout << "Full Optimization: " << duration.count() << "ms\n";
            std::cout << "Finished optimization\n";
            store<mig_ntk>().extend() = std::make_shared<mig_names>( ntk_mig );

            if(out_file != ""){
              
              mockturtle::write_verilog(ntk_mig, out_file);
              std::cout << "Resulting Verilog written to " << out_file << "\n";
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
    private:
        std::string nn_model{};
        std::string out_file{};
    };

  ALICE_ADD_COMMAND(optimization, "Optimization");
}