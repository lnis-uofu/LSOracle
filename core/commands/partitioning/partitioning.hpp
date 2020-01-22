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
// #include <utah/BiPart.h>


namespace alice
{
  class partitioning_command : public alice::command{

    public:
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit partitioning_command( const environment::ptr& env )
        : command( env, "Partitionins current network using k-means hypergraph partitioner" ) {

          opts.add_option( "--num,num", num_partitions, "Number of desired partitions" )->required();
          opts.add_option( "--config_direc,-c", config_direc, "Path to the configuration file for KaHyPar (../../core/test.ini is default)" );
          opts.add_option("--file,-f", part_file, "External file containing partitiion information");
          add_flag("--mig,-m", "Partitions stored MIG network (AIG network is default)");
          // add_flag("--bipart,-g", "Run hypergraph partitionining using BiPart from the Galois system");
        }

    protected:
      void execute(){
        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto ntk = *store<mig_ntk>().current();
            if(part_file != ""){
              std::cout << "Partitioning stored MIG network using external file\n";
              std::map<mockturtle::mig_network::node, int> part_data;
              std::ifstream ifs;

              ifs.open(part_file);
              int nodeIdx = 1;
              if(ifs.is_open()){
                while(ifs.good()){
                  if(nodeIdx > ntk.size()){
                    std::cout << "Partition file contains the incorrect number of nodes\n";
                    exit(1);
                  }
                  std::string part;
                  getline(ifs, part);
                  int test = std::stoi(part);
                  part_data[ntk.index_to_node(nodeIdx)] = std::stoi(part);
                  nodeIdx++;
                }
                ifs.close();
                oracle::partition_manager<mig_names> partitions(ntk, part_data, num_partitions);
                store<part_man_mig_ntk>().extend() = std::make_shared<part_man_mig>( partitions );
              }
              else{
                std::cout << "Unable to open partition data file\n";
              }
            }
            else{
              // if(is_set("bipart")){
              //   std::cout << "Partitioning stored MIG network using Galois BiPart\n";
              //   oracle::hypergraph<mig_names> t(ntk);
              //   uint32_t num_vertices = 0;
          
              //   t.get_hypergraph(ntk);
              //   std::vector<std::vector<uint32_t>> hedges = t.get_hyperedges();
              //   num_vertices = t.get_num_vertices();

              //   int num_threads = 14;
              //   scheduleMode mode = PP;
              //   std::map<int, int> bipart = biparting(hedges, num_vertices, num_partitions, num_threads, mode);
              //   std::map<mockturtle::mig_network::node, int> part_data;
              //   ntk.foreach_node([&](auto node){
              //     part_data[node] = bipart[node];
              //   });

              //   oracle::partition_manager<mig_names> partitions(ntk, part_data, num_partitions);
              //   store<part_man_mig_ntk>().extend() = std::make_shared<part_man_mig>( partitions );
              // }
              // else{
                std::cout << "Partitioning stored MIG network using KaHyPar\n";
                if(config_direc != ""){
                  oracle::partition_manager<mig_names> partitions(ntk, num_partitions, config_direc);
                  store<part_man_mig_ntk>().extend() = std::make_shared<part_man_mig>( partitions );
                }
                else{
                  oracle::partition_manager<mig_names> partitions(ntk, num_partitions);
                  store<part_man_mig_ntk>().extend() = std::make_shared<part_man_mig>( partitions );
                }
              //}
            }
          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto ntk = *store<aig_ntk>().current();
            if(part_file != ""){
              std::cout << "Partitioning stored AIG network using external file\n";
              std::map<mockturtle::aig_network::node, int> part_data;
              std::ifstream ifs;

              ifs.open(part_file);
              int nodeIdx = 1;
              if(ifs.is_open()){
                while(ifs.good()){
                  if(nodeIdx > ntk.size()){
                    std::cout << "Partition file contains the incorrect number of nodes\n";
                    exit(1);
                  }
                  std::string part;
                  getline(ifs, part);
                  int test = std::stoi(part);
                  part_data[ntk.index_to_node(nodeIdx)] = std::stoi(part);
                  nodeIdx++;
                }
                ifs.close();
                oracle::partition_manager<aig_names> partitions(ntk, part_data, num_partitions);
                store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>( partitions );
              }
              else{
                std::cout << "Unable to open partition data file\n";
              }
            }
            else{
              // if(is_set("bipart")){
              //   std::cout << "Partitioning stored AIG network using Galois BiPart\n";
              //   oracle::hypergraph<aig_names> t(ntk);
              //   uint32_t num_vertices = 0;
          
              //   t.get_hypergraph(ntk);
              //   std::vector<std::vector<uint32_t>> hedges = t.get_hyperedges();
              //   num_vertices = t.get_num_vertices();

              //   int num_threads = 14;
              //   scheduleMode mode = PP;
              //   std::map<int, int> bipart = biparting(hedges, num_vertices, num_partitions, num_threads, mode);
              //   std::map<mockturtle::aig_network::node, int> part_data;
              //   ntk.foreach_node([&](auto node){
              //     part_data[node] = bipart[node];
              //   });

              //   oracle::partition_manager<aig_names> partitions(ntk, part_data, num_partitions);
              //   store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>( partitions );
              // }
              // else{
                std::cout << "Partitioning stored AIG network using KaHyPar\n";
                if(config_direc != ""){
                  oracle::partition_manager<aig_names> partitions(ntk, num_partitions, config_direc);
                  store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>( partitions );
                }
                else{
                  oracle::partition_manager<aig_names> partitions(ntk, num_partitions);
                  store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>( partitions );
                }
              //}
            }
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
    private:
      int num_partitions{};
      std::string config_direc = "";
      std::string part_file = "";
  };

  ALICE_ADD_COMMAND(partitioning, "Partitioning");
}
