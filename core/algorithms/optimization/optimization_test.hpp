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

namespace oracle{

  using aig_names = mockturtle::names_view<mockturtle::aig_network>;
  using aig_ntk = std::shared_ptr<aig_names>;
  using part_man_aig = oracle::partition_manager<aig_names>;
  using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

  using mig_names = mockturtle::names_view<mockturtle::mig_network>;
  using mig_ntk = std::shared_ptr<mig_names>;
  using part_man_mig = oracle::partition_manager<mig_names>;
  using part_man_mig_ntk = std::shared_ptr<part_man_mig>;
    
  mig_names optimization_test(aig_names ntk_aig, part_man_aig partitions_aig, unsigned strategy,std::string nn_model, 
    bool high, bool aig, bool mig, bool combine){

    mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
    mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;

    std::vector<int> aig_parts;
    std::vector<int> mig_parts;
    std::vector<int> comb_aig_parts;
    std::vector<int> comb_mig_parts;
    int num_parts = partitions_aig.get_part_num();

    auto ntk_mig = *aig_to_mig(ntk_aig, 1);
    oracle::partition_manager<mig_names> partitions_mig(ntk_mig, partitions_aig.get_all_part_connections(), 
            partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(),
            partitions_aig.get_all_partition_regs(), partitions_aig.get_all_partition_regin(), partitions_aig.get_part_num());

    if(aig){
      for(int i = 0; i < num_parts; i++){
        aig_parts.push_back(i);
      }
    }
    else if(mig){
      for(int i = 0; i < num_parts; i++){
        mig_parts.push_back(i);
      }
    }
    else if(!nn_model.empty()){
      std::cout << "Performing Classification using Neural Network\n";
      partitions_aig.run_classification(ntk_aig, nn_model);
      aig_parts = partitions_aig.get_aig_parts();
      mig_parts = partitions_aig.get_mig_parts();
    }
    else{
      std::cout << "Performing High Effort Classification and Optimization\n";
      for(int i = 0; i < num_parts; i++){
        oracle::partition_view<mig_names> part = partitions_mig.create_part(ntk_mig, i);

        auto opt_part_aig = *part_to_mig(part, 1);
        auto opt_aig = *mig_to_aig(opt_part_aig);

        oracle::aig_script aigopt;
        opt_aig = aigopt.run(opt_aig);
        mockturtle::depth_view part_aig_opt_depth{opt_aig};
        int aig_opt_size = opt_aig.num_gates();
        int aig_opt_depth = part_aig_opt_depth.depth();

        auto opt_mig = *part_to_mig(part, 0);
        oracle::mig_script migopt;
        opt_mig = migopt.run(opt_mig);
        mockturtle::depth_view part_mig_opt_depth{opt_mig};
        int mig_opt_size = opt_mig.num_gates();
        int mig_opt_depth = part_mig_opt_depth.depth();

        switch(strategy){
          default:
          case 0:
          {
            if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
              aig_parts.push_back(i);
              if(!combine){
                auto opt_aig_mig = *aig_to_mig(opt_aig, 0);
                partitions_mig.synchronize_part(part, opt_aig_mig, ntk_mig);
              }
            }
            else{
              mig_parts.push_back(i);
              if(!combine){
                partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
              }
            }
          }
          break;
          case 1:
          {
            if((aig_opt_size) <= (mig_opt_size)){
              aig_parts.push_back(i);
              if(!combine){
                auto opt_aig_mig = *aig_to_mig(opt_aig, 0);
                partitions_mig.synchronize_part(part, opt_aig_mig, ntk_mig);
              }
            }
            else{
              mig_parts.push_back(i);
              if(!combine){
                partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
              }
            }
          }
          break;
          case 2:
          {
            if((aig_opt_depth) <= (mig_opt_depth)){
              aig_parts.push_back(i);
              if(!combine){
                auto opt_aig_mig = *aig_to_mig(opt_aig, 0);
                partitions_mig.synchronize_part(part, opt_aig_mig, ntk_mig);
              }
            }
            else{
              mig_parts.push_back(i);
              if(!combine){
                partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
              }
            }
          }
          break;
        }
        
      }
    }

    std::cout << aig_parts.size() << " AIGs and " << mig_parts.size() << " MIGs\n";

    if(combine){
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

    if(!high){
      for(int i = 0; i < aig_parts.size(); i++){
      
        oracle::partition_view<mig_names> part = partitions_mig.create_part(ntk_mig, aig_parts.at(i));
        mockturtle::depth_view part_depth{part};

        auto opt_part = *part_to_mig(part, 1);
        auto opt = *mig_to_aig(opt_part);

        mockturtle::depth_view opt_part_depth{opt};

        oracle::aig_script aigopt;
        opt = aigopt.run(opt);

        auto opt_mig = *aig_to_mig(opt, 0);

        partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
      }
      
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
    }
    
    partitions_mig.connect_outputs(ntk_mig);
    
    ntk_mig = mockturtle::cleanup_dangling( ntk_mig );

    return ntk_mig;
  }
}