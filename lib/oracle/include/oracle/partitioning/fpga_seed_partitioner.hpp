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
#include <limits>

#include <mockturtle/traits.hpp>
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/views/fanout_view.hpp>
#include <libkahypar.h>

namespace oracle
{

  /*! \brief Partitions circuit using multi-level hypergraph partitioner
   *
   */
  template<typename Ntk>
  class fpga_seed_partitioner : public Ntk
  {
  public:
    using storage = typename Ntk::storage;
    using node = typename Ntk::node;
    using signal = typename Ntk::signal;

  public:
    fpga_seed_partitioner(){}


    fpga_seed_partitioner( Ntk const& ntk, double nd, double mn, int pi_const, int node_count_const ) : Ntk( ntk ), _arr( ntk ), _req_arr( ntk )
    {

      static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
      static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
      static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
      static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
      static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
      static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
      static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

      net_delay = nd;
      max_net = mn;

      mockturtle::topo_view top_view{ntk};
      std::vector<node> top_nodes = top_view.get_node_vec();
      /*************
      Determine Dmax for each PO
      *************/
      mockturtle::depth_view ntk_depth{ntk};
      ntk.foreach_node([&](auto node){
        _arr[node] = ntk_depth.level(node);
        if(ntk.is_po(node)){
          if(_arr[node] > dmax){
            dmax = _arr[node];
            rmax = _arr[node];
          } 
        }
      });

      get_required_arrival(ntk);

      ntk.foreach_po([&](auto po){
        _req_arr[ntk.get_node(po)] = rmax;
      });
      /*************
      Determine slack fro each node
      store max slack value

      *************/
      ntk.foreach_node([&](auto node){
        int curr_slack = slack(node);
        std::cout << "Node = " << node << " with slack = " << curr_slack << "\n";
        if(curr_slack > max_slack)
          max_slack = curr_slack;
        if(!ntk.is_constant(node) && !ntk.is_pi(node))
          nodes2part.insert(node);
      });

      /*************
      Determine seed
      *************/
      mockturtle::fanout_view fanout{ntk};
      while(true){

        if(nodes2part.size() == 0)
          break; 

        cluster<Ntk> curr_cluster(ntk);

        auto start = std::chrono::high_resolution_clock::now();

        node seed = find_seed(ntk);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "Finding seed: " << duration.count() << "us\n";
        // std::cout << "first seed = " << seed << "\n";
        curr_cluster.add_to_cluster(ntk, seed);
        // std::cout << "erasing " << seed << "\n";
        nodes2part.erase(seed);
        while(true){

          if((curr_cluster.num_pis() >= pi_const && curr_cluster.size() >= node_count_const) || nodes2part.size() == 0)
            break;
          
          start = std::chrono::high_resolution_clock::now();
          std::set<node> connected_nodes = curr_cluster.get_conn_nodes(ntk, nodes2part);
          stop = std::chrono::high_resolution_clock::now();
          duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
          std::cout << "Finding all connected nodes: " << duration.count() << "us\n";

          // std::cout << "cluster size = " << curr_cluster.size() << "\n";
          // std::cout << "number of cluster internal nodes = " << curr_cluster.num_int_nodes() << "\n";
          // std::cout << "number of cluster inputs = " << curr_cluster.num_pis() << "\n";
          // std::cout << "number of connected nodes = " << connected_nodes.size() << "\n";
          // std::cout << "number of nodes to part = " << nodes2part.size() << "\n";
          if(connected_nodes.size() == 0)
            break;
          node best_node;
          double best_attr = -1.0;
          auto start1 = std::chrono::high_resolution_clock::now();
          for( node curr_node : connected_nodes ){
            start = std::chrono::high_resolution_clock::now();
            double curr_attr = attraction(ntk, curr_node, curr_cluster);
            stop = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            // std::cout << "Determining attraction: " << duration.count() << "us\n";
            // std::cout << "curr_node = " << curr_node << " with attraction = " << curr_attr << "\n";
            if( curr_attr > best_attr ){
              best_attr = curr_attr;
              best_node = curr_node;
            }
            
          }
          // mapped_part[best_node] = num_partitions;
          curr_cluster.add_to_cluster(ntk, best_node);
          // std::cout << "erasing " << best_node << "\n";
          nodes2part.erase(best_node);
          auto stop1 = std::chrono::high_resolution_clock::now();
          auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(stop1 - start1);
          std::cout << "Finding best node to add: " << duration1.count() << "us\n";
          // std::cout << "done connected nodes\n";
        }
        std::set<node> curr_cluster_nodes = curr_cluster.get_cluster();
        // std::set<node> curr_cluster_outputs = curr_cluster.get_outputs();
        std::set<node> curr_cluster_inputs = curr_cluster.get_inputs();
        std::cout << "Partition " << num_partitions << " = {";
        for(node curr_node : curr_cluster_nodes ){
          std::cout << curr_node << " ";
          mapped_part[curr_node] = num_partitions;
        }
        std::cout << "}\n";
        // std::cout << "Inputs = {";
        for(node curr_input : curr_cluster_inputs){
          // std::cout << curr_input << " ";
          if(ntk.is_pi(curr_input))
            mapped_part[curr_input] = num_partitions;
        } 
        // std::cout << "}\n";
        // std::cout << "Outputs = {";
        // for(node curr_output : curr_cluster_outputs){
        //   std::cout << curr_output << " ";
        //   // mapped_part[curr_output] = num_partitions;
        // } 
        // std::cout << "}\n";
        num_partitions++;
      }
      std::cout << "Number of partitions = " << num_partitions << "\n";

    }

    void get_required_arrival( Ntk const& ntk ){

      std::map<node, int> level;
      ntk.foreach_node([&](auto node){
        level[node] = 0;
      });
      mockturtle::topo_view top_view{ntk};
      mockturtle::fanout_view fanout{ntk};
      std::vector<node> top_nodes = top_view.get_node_vec();
      std::reverse(top_nodes.begin(), top_nodes.end());
      for(int i = 0; i < top_nodes.size(); i++){
        node curr_node = top_nodes.at(i);
        fanout.foreach_fanout(curr_node, [&](const auto& p){
          if(level[curr_node] < level[p] + 1)
            level[curr_node] = level[p] + 1;
        });
        _req_arr[curr_node] = dmax - level[curr_node];
      }
    }

    int slack( node curr_node ){
      return _req_arr[curr_node] - _arr[curr_node];
    }

    double connection_crit( node curr_node ){
      int curr_slack = slack(curr_node);
      return 1.0 - (double(curr_slack) / double(max_slack));
    }

    double attraction( Ntk const& ntk, node curr_node, cluster<Ntk> curr_cluster ){

      int net_intersec = curr_cluster.num_intersec(ntk, curr_node);

      return net_delay * connection_crit(curr_node) + (1 - net_delay) * net_intersec / max_net;
    }

    node find_seed(Ntk const& ntk){
      double max_crit = 0.0;
      node seed;
      ntk.foreach_gate([&]( auto curr_node ){
        if(nodes2part.find(curr_node) != nodes2part.end()){
          double conn_crit = connection_crit(curr_node);
          std::cout << "Node = " << curr_node << " with conn crit = " << conn_crit << "\n";
          if(conn_crit > max_crit){
            max_crit = conn_crit;
            seed = curr_node;
          }
        }
      });
      return seed;
    }

    partition_manager<Ntk> create_part_man(Ntk const& ntk){
      partition_manager<Ntk> part_man(ntk, mapped_part, num_partitions);
      return part_man;
    }

  private:

    int num_partitions = 0;
    std::map<node, int> mapped_part;
    double max_net = 0.0;
    double net_delay = 0.0;

    mockturtle::node_map<uint32_t, Ntk> _req_arr;
    mockturtle::node_map<uint32_t, Ntk> _arr;
    int dmax = 0;
    int rmax = std::numeric_limits<int>::max();
    int max_slack = 0;

    std::set<node> nodes2part;

  };
} /* namespace oracle */
