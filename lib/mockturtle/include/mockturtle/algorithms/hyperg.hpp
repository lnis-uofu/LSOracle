/*!
  \file hyperg.hpp
  \brief Converts the current network into an hypergraph
*/

#include <stdio.h>
#include "../traits.hpp"

namespace mockturtle {

template<class Ntk>
void hypergraph( Ntk const& ntk, std::ostream& os ) {

  static_assert( has_foreach_node_v<Ntk>,  "Ntk does not implement the foreach_node method" );
  static_assert( has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method" );
  static_assert( has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method" );
  static_assert( has_size_v<Ntk>,          "Ntk does not implement the size method" );

  //fanout view to iterate over fanouts and generate hyper edges
  mockturtle::fanout_view fanout{ntk};

  //Remove all children indeces from nodes so that the only connections remaining are outputs
  std::vector<std::vector<int>> hyperEdges;
  std::set<node<Ntk>> nodes;

  ntk.foreach_node( [&]( auto node ) {
    nodes.clear();
    std::vector<int> connections;

    int nodeNdx = ntk.node_to_index(node);

    fanout.foreach_fanout(node, [&](const auto& p){
      nodes.insert(p);
    });

    for(auto it : nodes){
      connections.push_back(ntk.node_to_index(it));
    }

    if(ntk.is_po(node)){
      ntk.foreach_fanin( node, [&] (auto const& conn, auto i) {
        connections.push_back(ntk._storage->nodes[node].children[i].index);
      });
    }

    if(connections.size() > 0){
      std::vector<int> connection_to_add = connections;
      //Add root node to the hyper edge
      connection_to_add.insert(connection_to_add.begin(), nodeNdx);
      hyperEdges.push_back(connection_to_add);
    }
  } );

  //Write to the hypergraph file
  os << hyperEdges.size() + 1 << " " << ntk.size() << "\n";
  for(int i = 0; i < hyperEdges.size(); i++){
    for(int j = 0; j < hyperEdges.at(i).size(); j++){
      //Add 1 to the indeces because hMetis does not recognize hyperedges containing vertex 0
      os << hyperEdges.at(i).at(j) + 1 << " ";
    }
    os << "\n";
  }
};

template<class Ntk>
void hypergraph( Ntk const& ntk, std::string const& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  hypergraph( ntk, os );
  os.close();
}

} //end of namespace