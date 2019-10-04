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

#include <mockturtle/traits.hpp>
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/views/fanout_view.hpp>

namespace oracle
{

  template<typename Ntk>
  class pattern_rec : public Ntk {

    public:
      using storage = typename Ntk::storage;
      using node = typename Ntk::node;
      using signal = typename Ntk::signal;

    public:

      pattern_rec(){}

      explicit pattern_rec( )
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

      }

      inline void find_xor_groups( Ntk const& ntk){
        mockturtle::topo_view top{ntk};
        std::vector<node> reverse_top = top.get_node_vec();
        std::reverse(reverse_top.begin(), reverse_top.end());

        
        for(int i = 0; i < xor_patterns.size(); i++){
          Ntk curr_pattern = xor_patterns.at(i);
          int patt_idx = 0;
          mockturtle::topo_view patt_top{curr_pattern};
          std::vector<node> patt_nodes = patt_top.get_node_vec();
          std::reverse(patt_nodes.begin(), patt_nodes.end());
          std::set<node> xor_nodes;
          for(int j = 0; j < reverse_top.size(); j++){
            if(patt_idx > curr_pattern.size()){
              patt_idx = 0;
              xor_groups.push_back(xor_nodes);
              xor_nodes.clear();
            }
            if(reverse_top.at(j) == patt_nodes.at(patt_idx)){
              xor_nodes.insert(reverse_top.at(j));
            }
            else{
              patt_idx = 0;
              xor_nodes.clear();
            }
          }
        }

        std::cout << "xors found = " << xor_groups.size() << "\n";
        for(int i = 0; i < xor_groups.size(); i++){
          std::cout << "XOR group " << i + 1 << " = {"; 
          typename std::set<node>::iterator it;
          for(it = xor_groups.at(i).begin(); it != xor_groups.at(i).end(); ++it){
            std::cout << *it << " ";
          }
          std::cout << "}\n";
        }
      }

  private:

    void create_patterns(){
      Ntk xor_patt_1;
      auto x = xor_patt_1.create_pi();
      auto y = xor_patt_1.create_pi();
      auto n1 = xor_patt_1.create_and(x, xor_patt_1.create_not(y));
      auto n2 = xor_patt_1.create_and(xor_patt_1.create_not(x), y);
      auto n3 = xor_patt_1.create_and(xor_patt_1.create_not(n1), xor_patt_1.create_not(n2));
      xor_patt_1.create_po(n3);

      xor_patterns.push_back(xor_patt_1);
      
    }
    
    std::vector<Ntk> xor_patterns;
    std::vector<std::set<node>> xor_groups;
    };

  } /* namespace oracle */