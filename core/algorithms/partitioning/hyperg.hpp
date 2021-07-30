/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*!
  \file hyperg.hpp
  \brief Converts the current network into an hypergraph
*/

#pragma once
#include <stdio.h>
#include <mockturtle/traits.hpp>

namespace oracle {

  template<class Ntk>
  class hypergraph {

    Ntk const& ntk;
    std::vector<std::vector<uint32_t>> hyperEdges;
    std::set<typename Ntk::node> nodes;
    std::vector<uint32_t> connections;

  public:
    hypergraph(Ntk const& ntk) : ntk(ntk) {};

    void get_hypergraph(Ntk const& ntk) {
      static_assert(mockturtle::has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method");
      static_assert(mockturtle::has_node_to_index_v<Ntk>, "Ntk does not implement the node_to_index method");
      static_assert(mockturtle::has_foreach_fanin_v<Ntk>, "Ntk does not implement the foreach_fanin method");
      static_assert(mockturtle::has_size_v<Ntk>, "Ntk does not implement the size method");

      //fanout view to iterate over fanouts and generate hyper edges
      mockturtle::fanout_view fanout{ntk};
      //Remove all children indeces from nodes so that the only connections remaining are outputs
      ntk.foreach_node([&](auto node) {
        nodes.clear();

        //clear vector connections here
        connections.clear();

        int nodeNdx = ntk.node_to_index(node);

        if(!is_po(ntk, node)) {
          fanout.foreach_fanout(node, [&](const auto &p) {
            nodes.insert(p);
          });

          for (auto it : nodes) {
            connections.push_back(ntk.node_to_index(it));
          }
        }

        else if (is_po(ntk, node) && !ntk.is_ro(node)) {
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

    void dump( std::string filename = "hypergraph.txt" ) {
      std::ofstream myfile;
      myfile.open (filename);
      myfile << hyperEdges.size() << " " << ntk.size()-1 << "\n";
      for (int i = 0; i < hyperEdges.size(); i++) {
        for (int j = 0; j < hyperEdges.at(i).size(); j++) {
          connections.push_back(hyperEdges.at(i).at(j) );
          myfile << hyperEdges.at(i).at(j) << " ";
        }
        myfile << "\n";
      }
    }

    void return_hyperedges(std::vector<uint32_t> &connections) {
      for (int i = 0; i < hyperEdges.size(); i++) {
        for (int j = 0; j < hyperEdges.at(i).size(); j++) {
          connections.push_back(hyperEdges.at(i).at(j) );
        }
      }
    }

    // void return_hyperedges_set(std::unordered_set<std::unordered_set<uint32_t>> &connections) {
    //   for (int i = 0; i < hyperEdges.size(); i++) {
    //     std::unordered_set<uint32_t> temp_edge;
    //     for (int j = 0; j < hyperEdges.at(i).size(); j++) {
    //       temp_edge.insert(hyperEdges.at(i).at(j) );
    //     }
    //     connections.insert(temp_edge);
    //   }
    // }

    int get_num_edges() {
      return hyperEdges.size();
    }

    int get_num_vertices() {
      return ntk.size();
    }

    uint32_t get_num_indeces() {
      uint32_t count = 0;

      for (int i = 0; i < hyperEdges.size(); i++) {
        //sum up the number of elements of each vector of vector -1 (the index of the given vector in the vector)
        count += hyperEdges.at(i).size();
      }
      return count;
    }

    uint64_t get_num_sets() {
      return hyperEdges.size();
    }

    std::vector<std::vector<uint32_t>> get_hyperedges() {
      return hyperEdges;
    }

    void get_indeces(std::vector<unsigned long> &indeces) {
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
  };
} //end of namespace
