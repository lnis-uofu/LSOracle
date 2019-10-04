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
  class write_hypergraph_command : public alice::command{

  public:
    explicit write_hypergraph_command( const environment::ptr& env )
        : command( env, "Output current stored network in a hypergraph representation to be used by hMetis" ){

      opts.add_option( "--filename,filename", filename, "hypergraph file to write to" )->required();
      add_flag("--mig,-m", "Use stored MIG network (Default is AIG)");
    }

  protected:
    void execute(){

      if(is_set("mig")){
        if(!store<mig_ntk>().empty()){

          if(checkExt(filename, "hpg")){
            std::ofstream output;
            output.open(filename);

            auto ntk = *store<mig_ntk>().current();  

            // std::map<int, std::vector<int>> connections;

            mockturtle::fanout_view fanout{ntk};
            std::vector<std::vector<int>> hyperEdges;
            ntk.foreach_node( [&]( auto node ) {
              std::vector<int> connections;
              int nodeNdx = ntk.node_to_index(node);

              std::set<mockturtle::node<mockturtle::mig_network>> nodes;
              fanout.foreach_fanout(node, [&](const auto& p){
                nodes.insert(p);
              });

              for(std::set<mockturtle::node<mockturtle::mig_network>>::iterator it = nodes.begin(); it != nodes.end(); ++it){
                
                connections.push_back(ntk.node_to_index(*it));
              }
              if(ntk.is_po(node)){
                connections.push_back(ntk._storage->nodes[node].children[0].index);
                connections.push_back(ntk._storage->nodes[node].children[1].index);
              }
              if(connections.size() > 0){
                  std::vector<int> connection_to_add = connections;
                  //Add root node to the hyper edge
                connection_to_add.insert(connection_to_add.begin(), nodeNdx);
                hyperEdges.push_back(connection_to_add);
              }
            }); 
              
            //Write to the hypergraph file
              //Subtract the size by one because the output node is duplicated with one of the gate nodes
              output << hyperEdges.size() + 1 << " " << ntk.size() /*- 1*/ << "\n";
              for(int i = 0; i < hyperEdges.size(); i++){
              for(int j = 0; j < hyperEdges.at(i).size(); j++){
                //Add 1 to the indeces because hMetis does not recognize hyperedges containing vertex 0
                output << hyperEdges.at(i).at(j) + 1 << " ";
              }
              output << "\n";
            }

            output.close();
          }
          else{
            std::cout << filename << " is not a valid hpg file\n";
          }

          
        }
        else{
          std::cout << "There is no MIG network stored\n";
        }
      }
      else{
        if(!store<aig_ntk>().empty()){

          if(checkExt(filename, "hpg")){
            std::ofstream output;
            output.open(filename);

            auto ntk = *store<aig_ntk>().current();  

            // std::map<int, std::vector<int>> connections;

            mockturtle::fanout_view fanout{ntk};
            std::vector<std::vector<int>> hyperEdges;
            ntk.foreach_node( [&]( auto node ) {
              std::vector<int> connections;
              int nodeNdx = ntk.node_to_index(node);

              std::set<mockturtle::node<mockturtle::aig_network>> nodes;
              fanout.foreach_fanout(node, [&](const auto& p){
                nodes.insert(p);
              });

              for(std::set<mockturtle::node<mockturtle::aig_network>>::iterator it = nodes.begin(); it != nodes.end(); ++it){
                
                connections.push_back(ntk.node_to_index(*it));
              }
              if(ntk.is_po(node)){
                connections.push_back(ntk._storage->nodes[node].children[0].index);
                connections.push_back(ntk._storage->nodes[node].children[1].index);
              }
              if(connections.size() > 0){
                  std::vector<int> connection_to_add = connections;
                  //Add root node to the hyper edge
                connection_to_add.insert(connection_to_add.begin(), nodeNdx);
                hyperEdges.push_back(connection_to_add);
              }
            }); 
              
            //Write to the hypergraph file
              //Subtract the size by one because the output node is duplicated with one of the gate nodes
              output << hyperEdges.size() + 1 << " " << ntk.size() /*- 1*/ << "\n";
              for(int i = 0; i < hyperEdges.size(); i++){
              for(int j = 0; j < hyperEdges.at(i).size(); j++){
                //Add 1 to the indeces because hMetis does not recognize hyperedges containing vertex 0
                output << hyperEdges.at(i).at(j) + 1 << " ";
              }
              output << "\n";
            }

            output.close();
          }
          else{
            std::cout << filename << " is not a valid hpg file\n";
          }

          
        }
        else{
          std::cout << "There is no AIG network stored\n";
        }
      }
    
      
    }

  private:
    std::string filename{};
  };

  ALICE_ADD_COMMAND(write_hypergraph, "Output");
}