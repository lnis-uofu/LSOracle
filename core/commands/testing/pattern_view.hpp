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


#pragma once

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <unordered_set>
#include <cassert>
#include <limits>

#include <mockturtle/traits.hpp>
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/views/fanout_view.hpp>

namespace oracle
{

  template<typename Ntk>
  class pattern_view : public Ntk {

    public:
      using storage = typename Ntk::storage;
      using node = typename Ntk::node;
      using signal = typename Ntk::signal;

    public:
      pattern_view(){}

      explicit pattern_view( Ntk const& ntk )
              : Ntk( ntk )
      {
        static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
        static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
        static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
        static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
        static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
        static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
        static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

        create_patterns();
        ntk.foreach_node([&](auto node){
          considered[node] = false;
          in_pattern[node] = false;
        });

        
        mockturtle::topo_view top{ntk};
        std::vector<node> reverse_top = top.get_node_vec();
        std::reverse(reverse_top.begin(), reverse_top.end());

        
        for(int i = 0; i < xor_patterns.size(); i++){
          Ntk curr_pattern = xor_patterns.at(i);
          
          std::vector<node> xor_nodes;       
          
          for(int j = 0; j < reverse_top.size() - ntk.num_pis(); j++){
            node ntk_node = reverse_top.at(j);
            if(!considered[ntk_node]){
              xor_nodes = xor_test(ntk, curr_pattern, ntk_node);
              for( node xor_node : xor_nodes ){
                in_pattern[xor_node] = true;
              }
            }
          }
        }

      }

      inline int xor_num(){
        return xor_groups.size();
      }

      inline node get_output( int xor_num ){

        std::vector<node> curr_xor = xor_groups.at(xor_num);

        std::sort(curr_xor.begin(), curr_xor.end());
        return curr_xor.back();
      }

      inline std::vector<node> get_inputs( Ntk const& ntk, int xor_num ){

        std::vector<node> curr_xor = xor_groups.at(xor_num);
        return xor_inputs(ntk, curr_xor);
      }

      inline std::vector<node> get_fanout( Ntk const& ntk, int xor_num ){
        mockturtle::fanout_view fanout{ntk};

        node output = get_output( xor_num );

        return fanout.fanout( output );
      }

      template<typename Fn>
      void foreach_xor( Fn&& fn ) const
      {
          mockturtle::detail::foreach_element( xor_groups.begin(), xor_groups.end(), fn );
      }

      inline void run_partitioning( Ntk const& ntk, int pi_const, int node_count_const ){

        ntk.foreach_node( [&](auto node){
          if(!ntk.is_constant(node) && !ntk.is_pi(node))
            nodes2part.insert(node);
        });

        for(int i = 0; i < xor_groups.size(); i++){
          patt2part.insert(xor_groups.at(i));
        }
        oracle::slack_view<Ntk> slack(ntk);
        while(true){

          if(nodes2part.size() == 0)
            break; 

          cluster<Ntk> curr_cluster(ntk);
          std::vector<node> seed = find_seed(ntk);
          curr_cluster.add_to_cluster(ntk, seed);
          
          std::cout << "Seed = {";
          for(int i = 0; i < seed.size(); i++){
            std::cout << seed.at(i) << " ";
          }
          std::cout << "}\n";

          while(true){
            if((curr_cluster.num_pis() >= pi_const && curr_cluster.size() >= node_count_const) || nodes2part.size() == 0)
              break;

            std::set<node> connected_nodes = curr_cluster.get_conn_nodes(ntk, nodes2part);
            std::cout << "number of connected_nodes = " << connected_nodes.size() << "\n";
            for(auto node : connected_nodes){
              std::cout << node << " ";
            }
            std::cout << "\n";
            if(connected_nodes.size() == 0)
              break;

            double best_attr = -1.0;
            std::vector<node> best_node;
            for( node curr_node : connected_nodes ){
              std::cout << "curr_node = " << curr_node << " in pattern = " << in_pattern[curr_node] << "\n";
              if(in_pattern[curr_node]){
                std::vector<node> pattern;
                int pattern_num = 0;
                foreach_xor([&]( auto xor_patt, int i ){
                  if(std::find(xor_patt.begin(), xor_patt.end(), curr_node) != xor_patt.end()){
                    pattern = xor_patt;
                    for( node xor_node : xor_patt ){
                      connected_nodes.erase(xor_node);
                    }
                  }
                });
                double curr_attr = attraction(ntk, pattern_num, curr_cluster);
                if( curr_attr > best_attr ){
                  best_attr = curr_attr;
                  best_node = pattern;
                }
              }
              else{
                // start = std::chrono::high_resolution_clock::now();
                double curr_attr = attraction(ntk, curr_node, curr_cluster);
                // stop = std::chrono::high_resolution_clock::now();
                // duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                // std::cout << "Determining attraction: " << duration.count() << "us\n";
                // std::cout << "curr_node = " << curr_node << " with attraction = " << curr_attr << "\n";
                if( curr_attr > best_attr ){
                  best_attr = curr_attr;
                  best_node = {curr_node};
                }
              }

              curr_cluster.add_to_cluster(ntk, best_node);
              if(in_pattern[curr_node])
                patt2part.erase(best_node);
              for(node curr_node : best_node){
                nodes2part.erase(curr_node);
              }
            }
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

  private:

    void create_patterns(){
      Ntk xor_patt_1;
      auto x = xor_patt_1.create_pi();
      auto y = xor_patt_1.create_pi();
      auto n1 = xor_patt_1.create_and(x, xor_patt_1.create_not(y));
      auto n2 = xor_patt_1.create_and(xor_patt_1.create_not(x), y);
      auto n3 = xor_patt_1.create_and(xor_patt_1.create_not(n1), xor_patt_1.create_not(n2));
      xor_patt_1.create_po(xor_patt_1.create_not(n3));

      xor_patterns.push_back(xor_patt_1);

      // Ntk xor_patt_2;
      // x = xor_patt_2.create_pi();
      // y = xor_patt_2.create_pi();
      // n1 = xor_patt_2.create_and(x, y);
      // n2 = xor_patt_2.create_and(xor_patt_2.create_not(x), xor_patt_2.create_not(y));
      // n3 = xor_patt_2.create_and(xor_patt_2.create_not(n1), xor_patt_2.create_not(n2));
      // xor_patt_2.create_po(n3);

      // xor_patterns.push_back(xor_patt_2);

      // Ntk xor_patt_6;
      // x = xor_patt_6.create_pi();
      // y = xor_patt_6.create_pi();
      // n1 = xor_patt_6.create_and(xor_patt_6.create_not(x), xor_patt_6.create_not(y));
      // n2 = xor_patt_6.create_and(x, y);
      // n3 = xor_patt_6.create_and(xor_patt_6.create_not(n1), xor_patt_6.create_not(n2));
      // xor_patt_6.create_po(n3);

      // xor_patterns.push_back(xor_patt_6);

      Ntk xor_patt_3;
      x = xor_patt_3.create_pi();
      y = xor_patt_3.create_pi();
      auto n4 = xor_patt_3.create_and(x, xor_patt_3.create_not(y));
      n1 = xor_patt_3.create_and(x, xor_patt_3.create_not(n4));
      n2 = xor_patt_3.create_and(xor_patt_3.create_not(n4), xor_patt_3.create_not(y));
      n3 = xor_patt_3.create_and(xor_patt_3.create_not(n1), xor_patt_3.create_not(n2));
      xor_patt_3.create_po(n3);

      xor_patterns.push_back(xor_patt_3);

      // Ntk xor_patt_4;
      // x = xor_patt_4.create_pi();
      // y = xor_patt_4.create_pi();
      // n4 = xor_patt_4.create_and(x, y);
      // n1 = xor_patt_4.create_and(x, xor_patt_4.create_not(n4));
      // n2 = xor_patt_4.create_and(xor_patt_4.create_not(n4), y);
      // n3 = xor_patt_4.create_and(xor_patt_4.create_not(n1), xor_patt_4.create_not(n2));
      // xor_patt_4.create_po(xor_patt_4.create_not(n3));

      // xor_patterns.push_back(xor_patt_4);

      // Ntk xor_patt_5;
      // x = xor_patt_5.create_pi();
      // y = xor_patt_5.create_pi();
      // n4 = xor_patt_5.create_and(xor_patt_5.create_not(x), xor_patt_5.create_not(y));
      // n1 = xor_patt_5.create_and(xor_patt_5.create_not(x), xor_patt_5.create_not(n4));
      // n2 = xor_patt_5.create_and(xor_patt_5.create_not(n4), xor_patt_5.create_not(y));
      // n3 = xor_patt_5.create_and(xor_patt_5.create_not(n1), xor_patt_5.create_not(n2));
      // xor_patt_5.create_po(xor_patt_5.create_not(n3));

      // xor_patterns.push_back(xor_patt_5);
      
    }

    bool node_equivalence(Ntk ntk, Ntk curr_pattern, node ntk_node, node patt_node){
      int ntk_comp_count = 0;
      int patt_comp_count = 0;
      bool equiv = true;
      mockturtle::depth_view depth{ntk};
      int prev_depth = depth.level(ntk.get_node(ntk._storage->nodes[ntk_node].children[1]));
      ntk.foreach_fanin(ntk_node, [&](auto conn, auto i){
        auto curr_patt_child = curr_pattern._storage->nodes[patt_node].children[i];

        if(ntk.is_complemented(conn)){
          ntk_comp_count++;
        }
        if(curr_pattern.is_complemented(curr_patt_child)){
          patt_comp_count++;
        }

        if(depth.level(ntk.get_node(conn)) != prev_depth && curr_pattern.is_po(patt_node))
          equiv = false;
      });
      if(ntk_comp_count != patt_comp_count || ntk.is_pi(ntk_node) || curr_pattern.is_pi(patt_node)){
        equiv = false;
      }

      return equiv;
    }

    std::vector<node> xor_inputs( Ntk const& ntk, std::vector<node> xor_group ){

      std::sort(xor_group.begin(), xor_group.end());
      mockturtle::depth_view depth{ntk};
      std::vector<node> inputs;
      if(xor_group.size() >= 2){
        node node1 = xor_group.at(0);
        node node2 = xor_group.at(1);


        if(ntk.is_pi(node1)){
          if(std::find(inputs.begin(), inputs.end(), node1) == inputs.end())
            inputs.push_back(node1);
        }
        else{
          ntk.foreach_fanin(node1, [&](auto conn, auto i){
            if(ntk.get_node(conn) != node2){
              if(std::find(inputs.begin(), inputs.end(), ntk.get_node(conn)) == inputs.end())
                inputs.push_back(ntk.get_node(conn));
            }
          });
        }

        if(ntk.is_pi(node2)){
          if(std::find(inputs.begin(), inputs.end(), node2) == inputs.end())
            inputs.push_back(node2);
        }
        else{
          ntk.foreach_fanin(node2, [&](auto conn, auto i){
            if(ntk.get_node(conn) != node1){
              if(std::find(inputs.begin(), inputs.end(), ntk.get_node(conn)) == inputs.end())
                inputs.push_back(ntk.get_node(conn));
            }
          });
        }

        for(int i = 0; i < xor_group.size(); i++){
          node curr_node = xor_group.at(i);
          if(ntk.is_pi(curr_node))
            if(std::find(inputs.begin(), inputs.end(), curr_node) == inputs.end())
              inputs.push_back(curr_node);

          ntk.foreach_fanin(curr_node, [&](auto conn, auto i){
            if(std::find(xor_group.begin(), xor_group.end(), ntk.get_node(conn)) == xor_group.end()){
              if(std::find(inputs.begin(), inputs.end(), ntk.get_node(conn)) == inputs.end())
                inputs.push_back(ntk.get_node(conn));
            }
          });
        }
        
      }
      
      return inputs;
      
    }

    double attraction( Ntk const& ntk, node curr_node, cluster<Ntk> curr_cluster ){

      int net_intersec = curr_cluster.num_intersec(ntk, curr_node);

      oracle::slack_view<Ntk> slack_view(ntk);
      mockturtle::fanout_view<Ntk> fanout(ntk);

      return net_delay * connection_crit(slack_view, fanout, curr_node) + (1 - net_delay) * net_intersec / max_net;
    }

    double attraction( Ntk const& ntk, int patt_num, cluster<Ntk> curr_cluster ){

      node output = get_output(patt_num);
      std::vector<node> inputs = get_inputs(ntk, patt_num);
      int net_intersec = curr_cluster.num_intersec(ntk, output, inputs);
      oracle::slack_view<Ntk> slack_view(ntk);
      mockturtle::fanout_view<Ntk> fanout(ntk);

      return net_delay * connection_crit(slack_view, fanout, output) + (1 - net_delay) * net_intersec / max_net;
    }

    std::vector<node> xor_test(Ntk const& ntk, Ntk const& curr_pattern, node root_node){

      std::vector<node> xor_nodes;
      mockturtle::fanout_view fanout{ntk};

      std::map<node, bool> visited; 
      ntk.foreach_node([&](auto node){
        visited[ntk.node_to_index(node)] = false;
      });
      // Create a queue for BFS 
      std::queue<node> net_queue; 
    
      // Mark the current node as visited and enqueue it 
      visited[root_node] = true; 
      net_queue.push(root_node); 
     
      while(!net_queue.empty()){
        node curr_node = net_queue.front();
        net_queue.pop();  
        xor_nodes.push_back(curr_node);

        if(xor_nodes.size() > 1 && xor_nodes.size() < (curr_pattern.size() - curr_pattern.num_pos() - curr_pattern.num_pis() + 1)
          && (fanout.fanout(curr_node).size() != 1 && !ntk.is_pi(curr_node))){
          xor_nodes.clear();
          break;
        }

        if(xor_nodes.size() == curr_pattern.size() - curr_pattern.num_pos())
          break;

        if(!ntk.is_pi(curr_node)){
          ntk.foreach_fanin(curr_node, [&](auto conn, auto i){
            node curr_child = ntk.get_node(conn);
            if(!visited[curr_child]){
              net_queue.push(curr_child);
              visited[curr_child] = true;
            }
          });   
        }   
      } 

      // Ensure xor_nodes is in topological order
      std::sort(xor_nodes.begin(), xor_nodes.end());
      // Separating inputs from xor_nodes 
      if(xor_nodes.size() > 2){

        xor_nodes.erase(xor_nodes.begin());
        xor_nodes.erase(xor_nodes.begin());
      }
      
      std::vector<node> inputs = xor_inputs(ntk, xor_nodes);
      std::vector<node> output = {xor_nodes.back()};
      std::sort(xor_nodes.begin(), xor_nodes.end());

      if(inputs.size() > 0){
        mockturtle::fanout_view<Ntk> fanout(ntk);
        fanout.clear_visited();
        mockturtle::window_view<mockturtle::fanout_view<Ntk>> window(fanout, inputs, output, false);

        mockturtle::default_simulator<kitty::dynamic_truth_table> sim( window.num_pis() );
        const auto tts = mockturtle::simulate<kitty::dynamic_truth_table>( window, sim );

        window.foreach_po( [&]( auto const&, auto i ) {

          if(kitty::to_hex( tts[i] ) == "6" || kitty::to_hex( tts[i] ) == "9"){
            if(std::find(xor_groups.begin(), xor_groups.end(), xor_nodes) == xor_groups.end()){
              bool duplicate = false;
              for(int i = 0; i < xor_groups.size(); i++){
                std::vector<node> other_inputs = xor_inputs(ntk, xor_groups.at(i));
                std::vector<node> other_output = {xor_groups.at(i).back()};
                if(other_inputs == inputs && other_output == output){
                  duplicate = true;
                }
              }
              if(!duplicate)
                xor_groups.push_back(xor_nodes);
            }
          }
        });

      }
      else{
        std::cout << "circuit too small\n";
      }

      return xor_nodes;
    }

    double connection_crit(oracle::slack_view<Ntk> slack_view, mockturtle::fanout_view<Ntk> fanout, node curr_node){
      std::cout << "slack = " << slack_view.slack(curr_node) << " and fanout size of " << fanout.fanout(curr_node).size() << "\n";
      return (1.0 - (slack_view.slack(curr_node) / slack_view.get_max_slack())) + fanout.fanout(curr_node).size();
    }

    std::vector<node> find_seed(Ntk const& ntk){
      oracle::slack_view<Ntk> slack_view(ntk);
      mockturtle::fanout_view<Ntk> fanout(ntk);
      double max_crit = 0.0;
      std::vector<node> seed;
      if(patt2part.size() != 0){
        for(int i = 0; i < xor_groups.size(); i++){
          if(patt2part.find(xor_groups.at(i)) != patt2part.end()){
            std::sort(xor_groups.at(i).begin(), xor_groups.at(i).end());
            node output = xor_groups.at(i).back();
            double conn_crit = connection_crit(slack_view, fanout, output);
            std::cout << "Node = " << output << " with conn crit = " << conn_crit << "\n";
            if(conn_crit > max_crit){
              max_crit = conn_crit;
              seed = xor_groups.at(i);
            }
          }
        }
        std::cout << "Pattern seed added\n";
        patt2part.erase(seed);
        for(int i = 0; i < seed.size(); i++){
          nodes2part.erase(seed.at(i));
        }
      }
      else{
        ntk.foreach_gate([&]( auto curr_node ){
          if(nodes2part.find(curr_node) != nodes2part.end()){
            double conn_crit = connection_crit(slack_view, fanout, curr_node);
            std::cout << "Node = " << curr_node << " with conn crit = " << conn_crit << "\n";
            if(conn_crit > max_crit){
              max_crit = conn_crit;
              seed = {curr_node};
            }
          }
        });

        std::cout << "node seed added = " << seed.at(0) << "\n";
        nodes2part.erase(seed.at(0));
      }
      
      return seed;
    }
    
    int num_partitions = 0;
    double max_net = 0.0;
    double net_delay = 0.0;
    std::map<node, int> mapped_part;
    std::vector<Ntk> xor_patterns;
    std::vector<std::vector<node>> xor_groups;
    std::map<node, bool> considered;
    std::map<node, bool> in_pattern;

    std::set<node> nodes2part;
    std::set<std::vector<node>> patt2part;
    };

  } /* namespace oracle */