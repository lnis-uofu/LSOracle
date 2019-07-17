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
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/views/fanout_view.hpp>
#include <libkahypar.h>

namespace oracle
{

  /*! \brief Partitions circuit using multi-level hypergraph partitioner
   *
   */
  template<typename Ntk>
  class seed_partitioner : public Ntk
  {
  public:
    using storage = typename Ntk::storage;
    using node = typename Ntk::node;
    using signal = typename Ntk::signal;

  public:
    seed_partitioner(){}


    seed_partitioner( Ntk const& ntk, int pi_const, int node_count_const ) : Ntk( ntk )
    {

      static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
      static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
      static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
      static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
      static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
      static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
      static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

      mockturtle::topo_view top_view{ntk};
      std::vector<node> nodes2part = top_view.get_node_vec();
      // std::reverse(nodes2part.begin(), nodes2part.end());

      std::map<node, bool> visited;
      for(int i = 0; i < nodes2part.size(); i++){
        visited[nodes2part.at(i)] = false;
      }

      int num_pi = 0;
      int num_int = 0;
      int part_idx = 0;

      for(int i = 0; i < nodes2part.size(); i++){
        auto curr_node = nodes2part.at(i);
        std::cout << "Current node = " << curr_node << "\n";

        if(visited[curr_node] == true){
          continue;
        }

        int new_pi = 0;
        ntk.foreach_fanin(curr_node, [&](auto conn, auto i){
          std::cout << "fanin = " << conn.index << "\n";
          if(visited[ntk.get_node(conn)] == false){
            new_pi = 1;
          }
        });

        if(new_pi == 1 || ntk.is_pi(curr_node)){
          std::cout << "increase pi count\n";
          num_pi++;
        }
        else{
          std::cout << "increase internal node count\n";
          num_int++;
        }

        visited[curr_node] = true;
        mapped_part[curr_node] = part_idx;
        std::cout << "num pi = " << num_pi << "\n";
        std::cout << "num internal nodes = " << num_int << "\n";

        if(num_pi >= pi_const && num_int >= node_count_const){
          std::cout << "Create partition\n";
          part_idx++;
          num_partitions++;
          num_pi = 0;
          num_int = 0;
          for(int j = 0; j < nodes2part.size(); j++){
            visited[nodes2part.at(j)] = false;
          }
        }

      }
    }

    partition_manager<Ntk> create_part_man(Ntk const& ntk){
      partition_manager<Ntk> part_man(ntk, mapped_part, num_partitions);
      return part_man;
    }

  private:

    int num_partitions = 1;
    std::map<node, int> mapped_part;

  };
} /* namespace oracle */
