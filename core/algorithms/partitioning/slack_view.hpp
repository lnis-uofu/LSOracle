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
  class slack_view : public Ntk {

    public:
      using storage = typename Ntk::storage;
      using node = typename Ntk::node;
      using signal = typename Ntk::signal;

    public:
      slack_view(){}

      explicit slack_view( Ntk const& ntk )
              : Ntk( ntk ), _arr( ntk ), _req_arr( ntk )
      {
        static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
        static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
        static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
        static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
        static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
        static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
        static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

        // set up for slack critical path calculation
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

        ntk.foreach_node([&](auto node){
          int curr_slack = slack(node);
          if(curr_slack > max_slack)
            max_slack = curr_slack;
        });
      }

      bool is_critical_path( node curr_node ) {
        return slack(curr_node) == 0;
      }

      int slack( node curr_node ){
        return _req_arr[curr_node] - _arr[curr_node];
      }

      int get_max_slack(){
        return max_slack;
      }

      std::vector<node> get_critical_path( Ntk const& ntk ){
        std::vector<node> crit_path;
        ntk.foreach_node( [&]( auto curr_node ){
          if(is_critical_path(curr_node)){
            crit_path.push_back(curr_node);
          }
        });
        return crit_path;
      }

  private:

    

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
    
    mockturtle::node_map<uint32_t, Ntk> _req_arr;
    mockturtle::node_map<uint32_t, Ntk> _arr;
    int dmax = 0;
    int rmax = std::numeric_limits<int>::max();
    int max_slack = 0;
    };

  } /* namespace oracle */