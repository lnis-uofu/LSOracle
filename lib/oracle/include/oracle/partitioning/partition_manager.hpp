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
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/views/fanout_view.hpp>

namespace oracle
{

/*! \brief Partitions circuit using multi-level hypergraph partitioner
 *
 */
    // template<typename Ntk, bool window_interface = has_traverse_v<Ntk>>
    // class window_view
    // {
    // };

    template<typename Ntk>
    class partition_manager : public Ntk
    {
    public:
        using storage = typename Ntk::storage;
        using node = typename Ntk::node;
        using signal = typename Ntk::signal;

    public:
        explicit partition_manager( Ntk const& ntk, std::string hmetis_file/*,int part_num*/ ) : Ntk( ntk )
        {

            static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
            static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
            static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
            static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
            static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
            static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
            static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

            /******************
            Generate HyperGraph
            ******************/

            /******************
            Partition with MLPart
            ******************/

            /******************
            Map IO of partitions
            ******************/
            map_part(ntk, hmetis_file);

            // check_parts();

        }

        inline void map_part( Ntk const& ntk, std::string hmetis_file){
            std::string line;
            std::ifstream input (hmetis_file);

            if(input.is_open())
            {
                int nodeNdx = 0;
                while ( getline (input,line) )
                {

                    int partition = atoi(line.c_str());
                    add_to_partition(nodeNdx, partition);
                    nodeNdx++;
                }
                input.close();

                map_partition_conn(ntk);

                map_part_io(ntk);
            }
            else
            {
                std::cout << "Unable to open file\n";
            }
            
            
        }
    

    private:
        void add_to_partition( int nodeIdx, int partition)
        {
            int temp_part_num = partition + 1;

            //Calculate the number of partitions by keeping track of the
            //maximum partition number added so far
            if(temp_part_num > num_partitions)
                num_partitions = temp_part_num;

            partitionMap[nodeIdx] = partition;
            partitionSize[partition]++;
        }

        void map_partition_conn(Ntk const& ntk)
        {
            for(int i = 0; i < num_partitions; i++)
            {

                std::unordered_map<int, std::vector<int>> partConnTemp;
                mockturtle::fanout_view fanout{ntk};
                std::set<node> nodes;
                // ntk.foreach_node( [&] (auto node) {
                //     std::cout << "current node = " << ntk.node_to_index(node) << "\n";
                //     std::cout << "size = " << ntk._storage->nodes.size() << "\n";
                // });
                ntk.foreach_node( [&] (auto node) {
                    // std::cout << "2current node = " << ntk.node_to_index(node) << "\n";
                    // std::cout << "2size = " << ntk._storage->nodes.size() << "\n";
                    nodes.clear();
                    std::vector<int> connections;

                    int nodeIdx = ntk.node_to_index(node);
                    fanout.foreach_fanout(node, [&](const auto& p){
                      nodes.insert(p);
                    });

                    for(auto it : nodes){
                      connections.push_back(ntk.node_to_index(it));
                    }

                    if(ntk.is_po(node)){
                        ntk.foreach_fanin(node, [&](const auto& child){
                            connections.push_back(child.index); 
                        });
                    }               

                    //If the current node is part of the current partition, it gets
                    //added to the partition connection
                    if(partitionMap[nodeIdx] == i)
                    {
                      partConnTemp[nodeIdx] = connections;
                    }

                });
                partitionConn[i] = partConnTemp;

            }

        }

        void map_part_io(Ntk const& ntk){
            for(int i = 0; i < num_partitions; i++){
                
                ntk.foreach_node( [&] (auto node) {
                
                    int nodeIdx = ntk.node_to_index(node);
                        
                    //Check to see if the current node is in the partition
                    if(partitionMap[nodeIdx] == i){

                        //The current node is in the output list and it is a constant
                        if(ntk.is_po(node) && ntk.is_constant(node) 
                                && partitionOutputs[i].find(node) == partitionOutputs[i].end()){

                            partitionOutputs[i].insert(node);
                        }

                        if(ntk.is_po(node) && ntk.is_pi(node)){
                            if(partitionInputs[i].find(node) == partitionInputs[i].end()){
                                partitionInputs[i].insert(node);
                            }
                            if(partitionOutputs[i].find(node) == partitionOutputs[i].end()){
                                partitionOutputs[i].insert(node);
                            }
                        }
                        for(int j = 0; j < partitionConn[i][nodeIdx].size(); j++){

                            //Add to the output list for the partition if (the node is a primary output OR (if output connections go to nodes
                            //that are not located in the partition AND it is not already included in the output list)) AND (the node is 
                            //not an input to the partition)
                                
                            if( (ntk.is_po(node) || partitionConn[i].find(partitionConn[i][nodeIdx].at(j)) == partitionConn[i].end())
                                 && partitionOutputs[i].find(node) == partitionOutputs[i].end()
                                    && !ntk.is_pi(nodeIdx) && partitionInputs[i].find(node) == partitionInputs[i].end()){
                                
                                partitionOutputs[i].insert(node);
                            }
                                //Primary output is tied directly to a primary input
                            
                            if(ntk.is_pi(nodeIdx) && partitionInputs[i].find(node) == partitionInputs[i].end()){
                                partitionInputs[i].insert(node);
                            }
                            ntk.foreach_fanin(node, [&](const auto& fn){

                                int childIdx = fn.index;
                                bool child = partitionConn[i].find(childIdx) != partitionConn[i].end();
                                if(!ntk.is_pi(nodeIdx)){
                                    if(!child && partitionInputs[i].find(childIdx) == partitionInputs[i].end()){
                                        partitionInputs[i].insert(ntk.get_node(fn));
                                    }
                                }
                            });
                                
                        }
                    }
                });
                
                std::cout << "partitionInputs " << i << " {";
                for(auto node : partitionInputs[i]){
                    std::cout << ntk.node_to_index(node) << " ";
                }
                std::cout << "}\n";
                std::cout << "partitionOutputs " << i << " {";
                for(auto node : partitionOutputs[i]){
                    std::cout << ntk.node_to_index(node) << " ";
                }
                std::cout << "}\n";
                
                
            }

        }

        // void check_parts(){
        //     for(int j = 0; j < partitions.size(); j++){
        //         auto curr_part = partitions.at(j);
        //         std::cout << "partition " << j << "\n";
        //         std::cout << "Inputs\n";
        //         curr_part.foreach_pi( [&]( auto node){
        //             std::cout << "nodeIdx = " << curr_part.node_to_index(node) << "\n";
        //         });
        //         std::cout << "Outputs\n";
        //         curr_part.foreach_po( [&]( auto node){
        //             std::cout << "nodeIdx = " << node.index << "\n";
        //         });
        //         std::cout << "All nodes\n";
        //         curr_part.foreach_node( [&]( auto node){
        //             std::cout << "nodeIdx = " << curr_part.node_to_index(node) << "\n";
        //             curr_part.foreach_fanin( node, [&]( auto const& child, auto i ) {
        //                 std::cout << "child[" << i << "] = " << child.index << "\n";
        //             });
        //         });
        //     }
        // }

    public:

        oracle::partition_view<Ntk> create_part( Ntk const& ntk, int part ){
            oracle::partition_view<Ntk> partition(ntk, partitionInputs[part], partitionOutputs[part], false);
            return partition;
        }
        int get_part_num(){
            return num_partitions;
        }
        
        // template<typename Fn>
        // void foreach_partition( Fn&& fn ) const{
        //     detail::foreach_element( ez::make_direct_iterator<uint64_t>( 0 ),
        //                      ez::make_direct_iterator<uint64_t>( partitions.size() ),
        //                      fn );
        // }
        //Map of each node's respective connection indeces in each respective partition
        std::unordered_map<int, std::unordered_map<int, std::vector<int>>> partitionConn;
        //The connections coming into a specific partition
        std::unordered_map<int, std::set<node>> partitionInputs;
        //The connections coming out of a specific partition
        std::unordered_map<int, std::set<node>> partitionOutputs;
        //Stores the size of each partition
        std::unordered_map<int, int> partitionSize;

        int num_partitions = 0;
        //Map of each node's partition number
        std::unordered_map<int, int> partitionMap;
        //Map of each node's respective connection indeces
        std::unordered_map<int, std::vector<int>> connections;

    };

} /* namespace oracle */
