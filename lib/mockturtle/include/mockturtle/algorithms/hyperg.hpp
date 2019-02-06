/*!
  \file hyperg.hpp
  \brief Converts the current network into an hypergraph
*/

#include <stdio.h>
#include "../traits.hpp"

namespace mockturtle {

template<class Ntk>
class hypergraph {

  Ntk const& ntk;
  std::vector<std::vector<uint32_t>> hyperEdges;
  std::set<Ntk::node> nodes;
  std::vector<uint32_t> connections;

public:
  hypergraph(Ntk const& ntk) : ntk(ntk) {};

  void get_hypergraph(Ntk const& ntk);

  void return_hyperedges(std::vector<uint32_t> &connections);

  int get_num_edges();

  int get_num_vertices();

  uint64_t get_num_sets();

  uint32_t get_num_indeces();

  void get_indeces(std::vector<unsigned long> &indeces);
};

template<class Ntk>
void hypergraph<Ntk>::get_hypergraph(Ntk const& ntk) {
  static_assert(has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method");
  static_assert(has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method");
  static_assert(has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method");
  static_assert(has_size_v<Ntk>, "Ntk does not implement the size method");
std::cout << "HERE\n";
  //fanout view to iterate over fanouts and generate hyper edges
  mockturtle::fanout_view fanout{ntk};

  //Remove all children indeces from nodes so that the only connections remaining are outputs

  ntk.foreach_node([&](auto node) {
    nodes.clear();

    //clear vector connections here
    connections.clear();

    int nodeNdx = ntk.node_to_index(node);

    if(!ntk.is_po(node)) {

      fanout.foreach_fanout(node, [&](const auto &p) {
        nodes.insert(p);
      });

      for (auto it : nodes) {
        connections.push_back(ntk.node_to_index(it));
      }
    }

    else if (ntk.is_po(node)) {
      ntk.foreach_fanin(node, [&](auto const &conn, auto i) {
        connections.push_back(ntk._storage->nodes[node].children[i].index);
      });
    }

    if (connections.size() > 0) {
      std::vector<uint32_t> connection_to_add = connections;
      //Add root node to the hyper edge
      connection_to_add.insert(connection_to_add.begin(), nodeNdx);
      hyperEdges.push_back(connection_to_add);
    }
  });
}

template<class Ntk>
void hypergraph<Ntk>::return_hyperedges(std::vector<uint32_t> &connections) {
  for (int i = 0; i < hyperEdges.size(); i++) {
    for (int j = 0; j < hyperEdges.at(i).size(); j++) {
      connections.push_back(hyperEdges.at(i).at(j) );
    }
  }
}

template<class Ntk>
int hypergraph<Ntk>::get_num_edges() {
  return hyperEdges.size();
}

template<class Ntk>
int hypergraph<Ntk>::get_num_vertices() {
  return ntk.size();
}

template<class Ntk>
uint32_t hypergraph<Ntk>::get_num_indeces() {
  uint32_t count = 0;

  for (int i = 0; i < hyperEdges.size(); i++) {
    //sum up the number of elements of each vector of vector -1 (the index of the given vector in the vector)
    count += hyperEdges.at(i).size();
  }
  return count;
}

template<class Ntk>
uint64_t hypergraph<Ntk>::get_num_sets() {
  return hyperEdges.size();
}

template<class Ntk>
void hypergraph<Ntk>::get_indeces(std::vector<unsigned long> &indeces) {
  unsigned long count = 0;
  for (int i = 0; i < hyperEdges.size(); i++) {
    if (i == 0) {
      indeces.push_back((unsigned long) (0));
    } else {
      count += (hyperEdges.at(i - 1).size());
      indeces.push_back(count);
    }
  }
  int last_index = hyperEdges.size()-1;
  count += (hyperEdges.at(last_index).size());
  indeces.push_back(count);
}
} //end of namespace