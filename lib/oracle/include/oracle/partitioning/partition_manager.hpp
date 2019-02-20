/* mockturtle: C++ logic network library
 * Copyright (C) 2018  EPFL
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
  \file window_view.hpp
  \brief Implements an isolated view on a window in a network
  \author Heinz Riener
*/

#pragma once

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <cassert>

#include <mockturtle/traits.hpp>
#include "partition_view.hpp"
#include "hyperg.hpp"
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/views/fanout_view.hpp>
#include <libkahypar.h>

namespace oracle
{

/*! \brief Partitions circuit using multi-level hypergraph partitioner
 *
 */

template<typename Ntk>
class partition_manager : public Ntk
{
public:
  using storage = typename Ntk::storage;
  using node = typename Ntk::node;
  using signal = typename Ntk::signal;

public:
  partition_manager(){}

  partition_manager( Ntk const& ntk, int part_num ) : Ntk( ntk )
  {

      static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
      static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
      static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
      static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
      static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
      static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
      static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

      num_partitions = part_num;

      for(int i = 0; i<part_num; ++i)
        _part_scope.push_back(std::set<node>());

      uint32_t kahyp_num_hyperedges = 0;
      uint32_t kahyp_num_vertices = 0;
      uint32_t kahyp_num_indeces_hyper = 0;
      unsigned long kahyp_num_sets = 0;
      std::vector<uint32_t> kahypar_connections;
      std::vector<unsigned long> kahyp_set_indeces;

      /******************
      Generate HyperGraph
      ******************/
      oracle::hypergraph<Ntk> t(ntk);
      t.get_hypergraph(ntk);
      t.return_hyperedges(kahypar_connections);
      kahyp_num_hyperedges = t.get_num_edges();
      kahyp_num_vertices = t.get_num_vertices();
      kahyp_num_indeces_hyper = t.get_num_indeces();
      kahyp_num_sets = t.get_num_sets();
      t.get_indeces(kahyp_set_indeces);
      /******************
      Partition with kahypar
      ******************/
      //configures kahypar
      kahypar_context_t* context = kahypar_context_new();
      kahypar_configure_context_from_file(context, "../../core/test.ini");

      //set number of hyperedges and vertices. These variables are defined by the hyperG command
      const kahypar_hyperedge_id_t num_hyperedges = kahyp_num_hyperedges;
      const kahypar_hypernode_id_t num_vertices = kahyp_num_vertices;

      //set all edges to have the same weight
      std::unique_ptr<kahypar_hyperedge_weight_t[]> hyperedge_weights = std::make_unique<kahypar_hyperedge_weight_t[]>(kahyp_num_vertices);

      for( int i = 0; i < kahyp_num_vertices; i++ ){
          hyperedge_weights[i] = 2;
      }

      //vector with indeces where each set starts
      std::unique_ptr<size_t[]> hyperedge_indices = std::make_unique<size_t[]>(kahyp_num_sets+1);

      for ( int j = 0; j < kahyp_num_sets+1; j++){
          hyperedge_indices[j] = kahyp_set_indeces[j];
          // std::cout << "HyperEdge indices at " << j << " is " << hyperedge_indices[j] << std::endl;
      }

      std::unique_ptr<kahypar_hyperedge_id_t[]> hyperedges = std::make_unique<kahypar_hyperedge_id_t[]>(kahyp_num_indeces_hyper);

      for ( int i = 0; i < kahyp_num_indeces_hyper; i++){
          hyperedges[i] = kahypar_connections[i];
          // std::cout << "HyperEdges at " << i << " is " << hyperedges[i] << std::endl;
      }

      const double imbalance = 0.03;
      const kahypar_partition_id_t k = part_num;

      kahypar_hyperedge_weight_t objective = 0;

      std::vector<kahypar_partition_id_t> partition(num_vertices, -1);

      kahypar_partition(num_vertices, num_hyperedges,
                        imbalance, k, nullptr, hyperedge_weights.get(),
                        hyperedge_indices.get(), hyperedges.get(),
                        &objective, context, partition.data());

    for(int i = 0; i < num_vertices; ++i) {
      //get rid of circuit PIs
      if(ntk.is_pi(ntk.index_to_node(i)) && !ntk.is_constant(ntk.index_to_node(i))){
        //std::cout << "Node "<< i << " is ckt PI " << std::endl;
        _part_scope[partition[i]].insert(ntk.index_to_node(i));
        _part_pis.insert(std::pair<int, node>(partition[i], ntk.index_to_node(i)));
      }

      //get rid of circuit POs
      else if(ntk.is_po(ntk.index_to_node(i))){
        _part_scope[partition[i]].insert(ntk.index_to_node(i));
        _part_pos.insert(std::pair<int, node>(partition[i], ntk.index_to_node(i)));
      }

      else if(!ntk.is_constant(ntk.index_to_node(i))){
        _part_scope[partition[i]].insert(ntk.index_to_node(i));
        _part_nodes.insert(std::pair<int, node>(partition[i], ntk.index_to_node(i)));
      }

      //look to partition inputs (those that are not circuit PIs)
      ntk.foreach_fanin( ntk.index_to_node(i), [&]( auto const& conn, auto j ) {
        if(partition[ntk._storage->nodes[ntk.index_to_node(i)].children[j].index]!=partition[i]){
          _part_scope[partition[i]].insert(ntk.index_to_node(i));
          _part_pis.insert(std::pair<int, node>(partition[i], ntk.index_to_node(ntk._storage->nodes[ntk.index_to_node(i)].children[j].index)));
        }
      });

      //look to partition outputs (those that are not circuit POs)
      mockturtle::fanout_view fanout{ntk};
      fanout.foreach_fanout( ntk.index_to_node(i), [&]( auto const& p ) {
        //if fanout node belongs to another partition it is a partition output
        if(partition[ntk.node_to_index(p)]!=partition[i]){
          _part_scope[partition[i]].insert(ntk.index_to_node(i));
          _part_pos.insert(std::pair<int, node>(partition[i], ntk.index_to_node(i)));
        }
      });
    }

      kahypar_context_free(context);

  }

private:
  int num_partitions = 0;

  std::multimap<int, node> _part_nodes;
  std::multimap<int, node> _part_pis;
  std::multimap<int, node> _part_pos;
  std::vector<std::set<node>> _part_scope;

  std::vector<node> roots;
  std::vector<node> leaves;

  // DFS traversal of the vertices reachable from v.
  // It uses recursive DFSUtil()
  template<class NtkDFS>
  void dfs(NtkDFS const& ntk, signal sig,  std::unordered_map<node, int> visited, std::vector<node> &dfs_order)
  {
      auto node = ntk.get_node(sig);
      if(!ntk.visited(node)){
          // std::cout << nodeIdx << " ";
          dfs_order.push_back(node);
          ntk.set_visited(node, 1);

          if(!ntk.is_pi(node)){
              ntk.foreach_fanin( node, [&] (auto fn){
                if(!ntk.is_constant(ntk.get_node(fn)))
                    dfs(ntk, fn, visited, dfs_order);
              });
          }
      }
  }

public:

//  oracle::partition_view<Ntk> create_part( Ntk const& ntk, int part ){
//      oracle::partition_view<Ntk> partition(ntk, partitionInputs[part], partitionOutputs[part], false);
//      return partition;
//  }

  oracle::partition_view<mockturtle::fanout_view<Ntk>> create_part( Ntk const& ntk, int part_index ){
    roots.clear();
    leaves.clear();

    create_part_inputs(part_index);
    create_part_outputs(part_index);

    mockturtle::fanout_view<Ntk> fanout_ntk (ntk);

    oracle::partition_view<mockturtle::fanout_view<Ntk>> partition(fanout_ntk, leaves, roots, false);
    return partition;
  }

  int get_part_num(){
      return num_partitions;
  }

  template<class NtkPart, class NtkOpt>
  void synchronize_part(oracle::partition_view<mockturtle::fanout_view<NtkPart>> part, NtkOpt const& opt, Ntk const& ntk){
      mockturtle::node_map<signal, NtkOpt> old_to_new( opt );
      std::vector<signal> pis;
      part.foreach_pi( [&]( auto node ) {
        pis.push_back(part.make_signal(node));
      });

      mockturtle::topo_view part_top{part};
      mockturtle::topo_view opt_top{opt};
      std::vector<node> opt_nodes = opt_top.get_top_view_nodes();

      int pi_idx = 0;
      std::set<signal> visited_pis;
      opt_top.foreach_node( [&]( auto node ) {
        if ( opt.is_constant( node ) || opt.is_ci( node ) )
            return;

        /* collect children */
        std::vector<signal> children;
        opt.foreach_fanin( node, [&]( auto child, auto ) {
          const auto f = old_to_new[child];
          if(opt.is_pi(opt.get_node(child))){
              f = pis.at(child.index - 1);
          }
          if ( opt.is_complemented( child ) )
          {
              children.push_back( ntk.create_not( f ) );
          }
          else
          {
              children.push_back( f );
          }
        } );
        old_to_new[node] = ntk.clone_node( opt, node, children );
      });

      opt.foreach_po( [&]( auto po ) {
        if ( opt.is_complemented( po ) )
        {
            ntk.create_not( po );
        }
      } );

      for(int i = 0; i < opt._storage->outputs.size(); i++){
          auto opt_out = old_to_new[opt._storage->outputs.at(i)];
          auto part_out = part._roots.at(i);
          output_substitutions[ntk.get_node(part_out)] = opt_out;
      }
  }

  void connect_outputs(Ntk const& ntk){
      for(auto it = output_substitutions.begin(); it != output_substitutions.end(); ++it){
          ntk.substitute_node(it->first, it->second);
      }
  }

  void create_part_outputs(int part_index){
    auto range = _part_pos.equal_range(part_index);
    for(auto i = range.first; i!=range.second;++i){
      if (std::find(roots.begin(), roots.end(), i->second) == roots.end()) {
        roots.push_back(i->second);
      }
    }
  }

  void create_part_inputs(int part_index){
    auto range = _part_pis.equal_range(part_index);
    for(auto i = range.first; i!=range.second;++i){
      if (std::find(leaves.begin(), leaves.end(), i->second) == leaves.end()) {
        leaves.push_back(i->second);
      }
    }
  }

  std::set<node> get_part_context (int partition_num){
    return _part_scope[partition_num];
  }

  //Map of each node's respective connection indeces
  std::unordered_map<int, std::vector<int>> connections;

private:
  std::unordered_map<node, signal> output_substitutions;

};
} /* namespace oracle */