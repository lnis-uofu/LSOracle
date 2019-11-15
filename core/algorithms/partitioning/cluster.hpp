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

  template<typename Ntk>
  class cluster : public Ntk
  {
  public:
    using storage = typename Ntk::storage;
    using node = typename Ntk::node;
    using signal = typename Ntk::signal;

  public:
    cluster(){}

    cluster( Ntk const& ntk ) : Ntk( ntk ){}

    int size(){
      return nodes.size();
    }

    int num_int_nodes(){
      return nodes.size() - inputs.size();
    }

    int num_pis(){
      return inputs.size();
    }

    int num_pos(){
      return outputs.size();
    }

    std::set<node> get_cluster(){
      return nodes;
    }

    std::set<node> get_inputs(){
      return inputs;
    }

    std::set<node> get_outputs(){
      return outputs;
    }

    void add_to_cluster( Ntk const& ntk, std::vector<node> nodes2add ){

      for(int i = 0; i < nodes2add.size(); i++){
        node node2add = nodes2add.at(i);
        nodes.insert(node2add);
      
        auto start = std::chrono::high_resolution_clock::now();
        if(outputs.find(node2add) != outputs.end()){
          outputs.erase(node2add);
        }
        if(inputs.find(node2add) != inputs.end()){
          inputs.erase(node2add);
        }
        mockturtle::fanout_view fanout{ntk};
        fanout.foreach_fanout(node2add, [&](const auto& p){
          if(nodes.find(p) == nodes.end() && outputs.find(p) == outputs.end()){
            outputs.insert(p);
          }
        });
        ntk.foreach_fanin(node2add, [&](auto conn, auto i){
          node fanin = ntk.get_node(conn);
          if(nodes.find(fanin) == nodes.end() && inputs.find(fanin) == inputs.end()){
            inputs.insert(fanin);
          }
        });
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        // std::cout << "Updating IO: " << duration.count() << "us\n";
      }
      
    }

    int num_intersec( Ntk const& ntk, node node2add ){
      int num_intersec_nets = 0;
      mockturtle::fanout_view fanout{ntk};
      fanout.foreach_fanout(node2add, [&](const auto& p){
        if(nodes.find(p) != nodes.end()){
          num_intersec_nets++;
        }
      });
      ntk.foreach_fanin(node2add, [&](auto conn, auto i){
        node fanin = ntk.get_node(conn);
        if(nodes.find(fanin) != nodes.end()){
          num_intersec_nets++;
        }
      });

      return num_intersec_nets;
    }

    int num_intersec( Ntk const& ntk, node output, std::vector<node> inputs ){
      int num_intersec_nets = 0;
      mockturtle::fanout_view fanout{ntk};
      fanout.foreach_fanout(output, [&](const auto& p){
        if(nodes.find(p) != nodes.end()){
          num_intersec_nets++;
        }
      });
      for(int i = 0; i < inputs.size(); i++){
        node curr_input = inputs.at(i);
        ntk.foreach_fanin(curr_input, [&](auto conn, auto ){
          node fanin = ntk.get_node(conn);
          if(nodes.find(fanin) != nodes.end()){
            num_intersec_nets++;
          }
        });
      }
      
      return num_intersec_nets;
    }

    std::set<node> get_conn_nodes( Ntk const& ntk, std::set<node> nodes2part ){
      std::set<node> connected_nodes;
      mockturtle::fanout_view fanout{ntk};
      for( node curr_output : outputs ){
        // std::cout << "fanout = " << curr_output << "\n";
        if(nodes.find(curr_output) == nodes.end() && nodes2part.find(curr_output) != nodes2part.end()){
          connected_nodes.insert(curr_output);
        }
        // fanout.foreach_fanout( curr_output, [&](const auto& p){
        //   std::cout << "fanout = " << p << "\n";
        //   if(nodes.find(p) == nodes.end() && nodes2part.find(p) != nodes2part.end())
        //     connected_nodes.insert(p);
        // });
      }

      for( node curr_input : inputs ){
        // std::cout << "fanin = " << curr_input << "\n";
        if(nodes.find(curr_input) == nodes.end() && nodes2part.find(curr_input) != nodes2part.end()){
          connected_nodes.insert(curr_input);
        }
      }

      return connected_nodes;
    }
    
  private:
    
    std::set<node> nodes{};
    std::set<node> inputs{};
    std::set<node> outputs{};

    void update_io( Ntk const& ntk ){
      inputs.clear();
      outputs.clear();
      for( node curr_node : nodes ){
        if(ntk.is_pi(curr_node) && inputs.find(curr_node) == inputs.end()){
          inputs.insert(curr_node);
        }
        if(ntk.is_po(curr_node) && outputs.find(curr_node) == outputs.end()){
          outputs.insert(curr_node);
        }
        mockturtle::fanout_view fanout{ntk};
        fanout.foreach_fanout(curr_node, [&](const auto& p){
          if(nodes.find(p) == nodes.end() && outputs.find(p) == outputs.end()){
            outputs.insert(curr_node);
          }
        });
        ntk.foreach_fanin(curr_node, [&](auto conn, auto i){
          node fanin = ntk.get_node(conn);
          if(nodes.find(fanin) == nodes.end() && inputs.find(fanin) == inputs.end()){
            inputs.insert(fanin);
          }
        });
      }
      // std::cout << "inputs = {";
      // for( node curr_input : inputs ){
      //   std::cout << curr_input << " ";
      // }
      // std::cout << "}\n";
      // std::cout << "output = {";
      // for( node curr_output : outputs ){
      //   std::cout << curr_output << " ";
      // }
      // std::cout << "}\n";
    }

  };
} /* namespace oracle */
