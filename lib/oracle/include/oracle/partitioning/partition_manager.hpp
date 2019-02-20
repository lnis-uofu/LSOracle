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

  partition_manager(Ntk const& ntk, std::vector<std::set<node>> scope, std::unordered_map<int, std::set<node>> inputs, 
    std::unordered_map<int, std::set<node>> outputs){

    _part_scope = scope;
    partitionInputs = inputs;
    partitionOutputs = outputs;
  }

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

    for( int i = 0; i < kahyp_num_vertices; i++ )
      hyperedge_weights[i] = 2;
      

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

    for(int i = 0; i < part_num; i++){
      partitionInputs[i] = create_part_inputs(i);
      partitionOutputs[i] = create_part_outputs(i);
    }

    kahypar_context_free(context);

  }

private:
  


  
    /***************************************************
    Utility functions to be moved later
    ***************************************************/
        
    // Helper function to flip the bit
    char flip(char c){
      return (c == '0') ? '1' : '0';
    }

    int get_output_index(Ntk const& ntk, int nodeIdx){

      assert(ntk.is_po(nodeIdx));

      for(int i = 0; i < ntk._storage->outputs.size(); i++){
        if(ntk._storage->outputs.at(i).index == nodeIdx){
          return i;
        }
      }
    }//get_output_index()

    //Simple BFS Traversal to optain the depth of an output's logic cone before the truth table is built
    void BFS_traversal(Ntk const& ntk, node output, int partition){
      std::queue<int> net_queue;
      std::map<int, bool> visited;
      std::set<int> inputs;
      //Set all nodes to be unvisited
      ntk.foreach_node( [&]( auto node ) {
        visited[ntk.node_to_index(node)] = false;
      });
      int outputIdx = ntk.node_to_index(output);
      net_queue.push(outputIdx);
      visited[outputIdx] = true;
            
      while(!net_queue.empty()){

        int curr_node = net_queue.front();
        net_queue.pop();
        auto node = ntk.index_to_node(curr_node);

        //Make sure that the BFS traversal does not go past the inputs of the partition
        if(partitionInputs[partition].find(curr_node) == partitionInputs[partition].end()){

          for(int i = 0; i < ntk._storage->nodes[node].children.size(); i++){

            int childIdx = ntk._storage->nodes[node].children[i].index;
            bool is_valid = true;
            //Make sure a valid child index is found
            if(childIdx < 0){
              is_valid = false;
            }

            if(!visited[childIdx]){ 

              if(is_valid){
                                        
                                net_queue.push(childIdx);
                                visited[childIdx] = true;

                            }

                        }
                        
                    }
                }
                else{
                    inputs.insert(curr_node);
                }
            }
            logic_cone_inputs[output] = inputs;

        }//BFS_traversal()

        int computeLevel( Ntk const& ntk, node curr_node, int partition ) {
            //if node not visited
           if(ntk._storage->nodes[curr_node].data[1].h1==0) {

               //set node as visited
               ntk._storage->nodes[curr_node].data[1].h1=1;

               //if is input
               if (partitionInputs[partition].find(curr_node) != partitionInputs[partition].end()) {
                   return 0;
               }

                auto inIdx2 = ntk._storage->nodes[curr_node].children[1].data;
                if (inIdx2 & 1)
                    inIdx2 = inIdx2 - 1;

                //calculate input node index
                auto inNode1 = inIdx2 >> 1;
                int levelNode1 = computeLevel(ntk, inNode1, partition);

               auto inIdx = ntk._storage->nodes[curr_node].children[0].data;
               if (inIdx & 1)
                   inIdx = inIdx - 1;

               //calculate input node index
               auto inNode0 = inIdx >> 1;
               int levelNode0 = computeLevel(ntk, inNode0, partition);

               int level = 1 + std::max(levelNode0, levelNode1);
               return level;
           }


        }

        std::string to_binary(int dec){

            std::string bin;
            while(dec != 0){
                bin = (dec % 2 == 0 ? "0":"1") + bin;
                dec /= 2;
            }
            return bin;
        }

        std::string graytoBinary(std::string gray){
            std::string binary = "";

            // MSB of binary code is same as gray code
            binary += gray[0];

            // Compute remaining bits
            for (int i = 1; i < gray.length(); i++) {
                // If current bit is 0, concatenate
                // previous bit
                if (gray[i] == '0')
                    binary += binary[i - 1];

                    // Else, concatenate invert of
                    // previous bit
                else
                    binary += flip(binary[i - 1]);
            }

            return binary;
        }

        // Function to convert binary to decimal
        int binaryToDecimal(int n){

            int num = n;
            int dec_value = 0;

            // Initializing base value to 1, i.e 2^0
            int base = 1;

            int temp = num;
            while (temp)
            {
                int last_digit = temp % 10;
                temp = temp/10;

                dec_value += last_digit*base;

                base = base*2;
            }

            return dec_value;
        }

        /***************************************************/

        void tt_build(Ntk const& ntk, int partition, node curr_node, node root){
            int nodeIdx = ntk.node_to_index(curr_node);
            if(logic_cone_inputs[root].find(nodeIdx) != logic_cone_inputs[root].end() || partitionMap[nodeIdx] != partition){
                if(logic_cone_inputs[root].find(root) != logic_cone_inputs[root].end()){
                    auto output = ntk._storage->outputs.at(get_output_index(ntk,root));
                    if(output.data & 1){
                        tt_map[nodeIdx] = ~tt_map[nodeIdx];
                    }
                }
                return;
            }
            
            std::vector<signal> children;
            ntk.foreach_fanin(curr_node, [&]( auto const& child, auto i){
                children.push_back(child);
            });
            // int child1Idx = ntk._storage->nodes[nodeIdx].children[0].index;
            // int child2Idx = ntk._storage->nodes[nodeIdx].children[1].index;
            
            for(auto child : children){
                tt_build(ntk, partition, ntk.get_node(child), root);
            }

            if(logic_cone_inputs[root].find(nodeIdx) == logic_cone_inputs[root].end() ){

                std::vector<kitty::dynamic_truth_table> child_tts;
                for(auto child : children){
                    child_tts.push_back(tt_map[child.index]);
                }

                ntk.foreach_fanin( curr_node, [&]( auto const& conn, auto i ) {

                    int childIdx = conn.index;
                    if ( ntk.is_complemented( conn )) {
                        child_tts.at(i) = ~tt_map[childIdx];

                    }

                    if(ntk.is_po(childIdx) && logic_cone_inputs[root].find(childIdx) != logic_cone_inputs[root].end()){
                        auto output = ntk._storage->outputs.at(get_output_index(ntk,childIdx));
                        if(output.data & 1){
                            child_tts.at(i) = ~child_tts.at(i);
                        }
                    }
                });
                kitty::dynamic_truth_table tt = kitty::binary_and(child_tts.at(0), child_tts.at(1));
                tt_map[nodeIdx] = tt;
            }
            
            if(ntk.is_po(nodeIdx) && nodeIdx == root){
                auto output = ntk._storage->outputs.at(get_output_index(ntk,nodeIdx));
                if(output.data & 1){
                    tt_map[nodeIdx] = ~tt_map[nodeIdx];
                }

            }
        
        }

public:

     oracle::partition_view<Ntk> create_part( Ntk const& ntk, int part ){
         oracle::partition_view<Ntk> partition(ntk, partitionInputs[part], partitionOutputs[part], false);
         return partition;
     }

  // oracle::partition_view<mockturtle::fanout_view<Ntk>> create_part( Ntk const& ntk, int part_index ){

  //   mockturtle::fanout_view<Ntk> fanout_ntk (ntk);

  //   oracle::partition_view<mockturtle::fanout_view<Ntk>> partition(fanout_ntk, partitionInputs[part_index], partitionOutputs[part_index], false);
  //   return partition;
  // }

  int get_part_num(){
      return num_partitions;
  }

  std::set<node> get_part_outputs(int partition){
            return partitionOutputs[partition];
        }

        std::set<node> get_part_inputs(int partition){
            return partitionInputs[partition];
        }

  template<class NtkPart, class NtkOpt>
  void synchronize_part(oracle::partition_view<NtkPart> part, NtkOpt const& opt, Ntk const& ntk){
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

  void generate_truth_tables(Ntk const& ntk){
            for(int i = 0; i < num_partitions; i++){                  
                
                typename std::set<node>::iterator it;
                for(it = partitionOutputs[i].begin(); it != partitionOutputs[i].end(); ++it){

                    // start = clock();
                    auto curr_output = *it;                     
                    BFS_traversal(ntk, curr_output, i);
                    if(ntk.is_constant(curr_output)){
                        std::cout << "CONSTANT\n";
                    }
                    else if(logic_cone_inputs[curr_output].size() <= 16 && !ntk.is_constant(curr_output)){

                        int idx = 0;
                        std::set<int>::iterator input_it;
                        for(input_it = logic_cone_inputs[curr_output].begin(); input_it != logic_cone_inputs[curr_output].end(); ++input_it){
                            int nodeIdx = *input_it;
                            kitty::dynamic_truth_table tt( logic_cone_inputs[curr_output].size() );

                            kitty::create_nth_var(tt, idx);
                                
                            tt_map[nodeIdx] = tt;
                            idx++;
                        }
                        tt_build(ntk, i, curr_output, curr_output);
                            
                        output_tt[curr_output] = tt_map[curr_output];
                        // float runtime = ((float)clock() - start)/CLOCKS_PER_SEC;
                        ntk.foreach_node( [&]( auto node ) {
                            int index = ntk.node_to_index(node);
                            ntk._storage->nodes[index].data[1].h1 = 0;
                        });
                            
                    }
                    else{
                        std::cout << "Logic Cone too big at " << logic_cone_inputs[curr_output].size() << " inputs\n";
                    }
                }
            }
        }

        void write_karnaugh_maps(Ntk const& ntk, std::string directory){

            if(output_tt.empty()){
                generate_truth_tables(ntk);
            }

            mkdir(directory.c_str(), 0777);
            for(int i = 0; i < num_partitions; i++){
                int partition = i;
                typename std::set<node>::iterator it;
                for(it = partitionOutputs[i].begin(); it != partitionOutputs[i].end(); ++it){
                    auto output = *it;
                    BFS_traversal(ntk, output, partition);
                    int num_inputs = logic_cone_inputs[output].size();
                    ntk.foreach_node( [&]( auto node ) {
                        int index = ntk.node_to_index(node);
                        ntk._storage->nodes[index].data[1].h1 = 0;
                    });
                    int logic_depth = computeLevel(ntk, output, partition);

                    std::string file_out = ntk._storage->net_name + "_kar_part_" + std::to_string(partition) + "_out_" + 
                                        std::to_string(output) + "_in_" + std::to_string(num_inputs) + "_lev_" + std::to_string(logic_depth) + ".txt";

                                    
                    std::string tt = kitty::to_binary(output_tt[output]);
                    char* tt_binary = malloc(sizeof(char) * (tt.length() + 1));
                    strcpy(tt_binary, tt.c_str());
                                   
                    std::vector<std::string> onset_indeces;
                    int indx = 0;
                    for(int k = tt.length() - 1; k >= 0; k--){
                        int bit = (int)tt_binary[k] - 48;
                        if(bit == 1){
                            onset_indeces.push_back(to_binary(indx));
                        }
                        indx++;
                    }
                    for(int k = 0; k < onset_indeces.size(); k++){
                        while(onset_indeces.at(k).length() != logic_cone_inputs[output].size()){
                            onset_indeces.at(k).insert(0, "0");
                        }
                        std::reverse(onset_indeces.at(k).begin(), onset_indeces.at(k).end());
                    } 

                    int columns = num_inputs / 2;
                    int rows;
                    if(num_inputs <= 16 && num_inputs >= 2){
                        std::ofstream output_file(directory + file_out, std::ios::out | std::ios::binary | std::ios::trunc);
                        if(num_inputs % 2 != 0){
                            rows = columns + 1;
                        }
                        else{
                            rows = columns;
                        }

                        int row_num = pow(2, rows);
                        int col_num = pow(2, columns);
                        char **k_map = malloc(sizeof(char *) * col_num);
                        for(int y = 0; y < col_num; y++)
                            k_map[y] = malloc(sizeof(char) * row_num); 

                        for(int y = 0; y < col_num; y++){
                            for(int x = 0; x < row_num; x++){
                                k_map[y][x] = 0;
                            }
                        }
                                            
                        for(int k = 0; k < onset_indeces.size(); k++){
                                                
                            std::string row_index_gray = onset_indeces.at(k).substr(0, rows);
                            std::string col_index_gray = onset_indeces.at(k).substr(rows, onset_indeces.at(k).size() - 1);
                            std::string row_index_bin = graytoBinary(row_index_gray);
                            std::string col_index_bin = graytoBinary(col_index_gray);
                            int row_index = std::stoi(row_index_bin,nullptr,2);
                            int col_index = std::stoi(col_index_bin,nullptr,2);
                            k_map[col_index][row_index] = 2;
                                                
                        }
                                        
                        if(num_inputs < 16){
                            int padded_row = 256;
                            int padded_col = 256;       
                            char **k_map_pad = malloc(sizeof(char *) * padded_col);
                            for(int k = 0; k < padded_col; k++){
                                k_map_pad[k] = malloc(sizeof(char) * padded_row); 
                            }

                            for(int y = 0; y < padded_col; y++){
                                for(int x = 0; x < padded_row; x++){
                                    k_map_pad[y][x] = 1;
                                }
                            }
                            int row_offset = (padded_row - row_num);
                            if(row_offset % 2 != 0){
                                row_offset++;
                            }
                            int col_offset = (padded_col - col_num);
                            if(col_offset % 2 != 0){
                                col_offset++;
                            }
                            row_offset /= 2;
                            col_offset /= 2;
                            for(int y = 0; y < col_num; y++){
                                for(int x = 0; x < row_num; x++){
                                    k_map_pad[y + col_offset][x + row_offset] = k_map[y][x];
                                }
                            }
                            std::vector<char> data_1d(padded_row * padded_col);
                            for(int y = 0; y < padded_col; y++){
                                for(int x = 0; x < padded_row; x++){
                                    data_1d[x + y*padded_col] = k_map_pad[y][x];
                                }
                            }

                            output_file.write(data_1d.data(), data_1d.size()*sizeof(char));
                        }
                        else{
                            std::vector<char> data_1d(row_num * col_num);
                            for(int y = 0; y < col_num; y++){
                                for(int x = 0; x < row_num; x++){
                                    data_1d[x + y*col_num] = k_map[y][x];
                                }
                            }
                            output_file.write(data_1d.data(), data_1d.size()*sizeof(char));
                        }
                        output_file.close();
                    }

                }
            }
        }

  void connect_outputs(Ntk const& ntk){
      for(auto it = output_substitutions.begin(); it != output_substitutions.end(); ++it){
          ntk.substitute_node(it->first, it->second);
      }
  }

  std::set<node> create_part_outputs(int part_index){
    std::set<node> outputs;
    auto range = _part_pos.equal_range(part_index);
    for(auto i = range.first; i!=range.second;++i){
      if (std::find(outputs.begin(), outputs.end(), i->second) == outputs.end()) {
        outputs.insert(i->second);
      }
    }
    return outputs;
  }

  std::set<node> create_part_inputs(int part_index){
    std::set<node> inputs;
    auto range = _part_pis.equal_range(part_index);
    for(auto i = range.first; i!=range.second;++i){
      if (std::find(inputs.begin(), inputs.end(), i->second) == inputs.end()) {
        inputs.insert(i->second);
      }
    }
    return inputs;
  }

    std::unordered_map<int, int> get_all_partition_conn(){
        return partitionMap;
    }

    // int get_part_size(int partition){
    //     return partitionSize[partition];
    // }

    std::set<node> get_part_context (int partition_num){
        return _part_scope[partition_num];
    }

  //Map of each node's respective connection indeces
  std::unordered_map<int, std::vector<int>> connections;

private:
  int num_partitions = 0;

  std::multimap<int, node> _part_nodes;
  std::multimap<int, node> _part_pis;
  std::multimap<int, node> _part_pos;
  std::vector<std::set<node>> _part_scope;

  std::unordered_map<int, int> partitionMap;

  std::unordered_map<int, std::set<node>> partitionOutputs;
  std::unordered_map<int, std::set<node>> partitionInputs;

  std::unordered_map<node, signal> output_substitutions;
  
  std::map<int, int> output_cone_depth;
  std::unordered_map<node, std::set<int>> logic_cone_inputs;

  std::map<int,kitty::dynamic_truth_table> tt_map;
  std::map<int,kitty::dynamic_truth_table> output_tt;

};
} /* namespace oracle */
