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

        partition_manager( Ntk const& ntk, /*std::string hmetis_file*/int part_num ) : Ntk( ntk )
        {

            static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
            static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
            static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
            static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
            static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
            static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
            static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

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

            // std::cout << "Number of hyperedges " << num_hyperedges << "\n"
            // << "Number of vertices " << num_vertices << "\n"
            // << "Number of partitions " << part_num << "\n";

            const double imbalance = 0.03;
            const kahypar_partition_id_t k = part_num;

            kahypar_hyperedge_weight_t objective = 0;

            std::vector<kahypar_partition_id_t> partition(num_vertices, -1);

            kahypar_partition(num_vertices, num_hyperedges,
                              imbalance, k, nullptr, hyperedge_weights.get(),
                              hyperedge_indices.get(), hyperedges.get(),
                              &objective, context, partition.data());

            // std::cout << "################ Partitions ################" << std::endl;
            for(int i = 0; i < num_vertices; ++i) {
              // std::cout << partition[i] << std::endl;
              add_to_partition(i, partition[i]);
            }

            kahypar_context_free(context);
            /******************
            Map IO of partitions
            ******************/
            map_partition_conn(ntk);

            map_part_io(ntk);

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

                        //The current node is not in the output list and it is a constant
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
                                    && !ntk.is_pi(nodeIdx) && !ntk.is_constant(nodeIdx) && partitionInputs[i].find(node) == partitionInputs[i].end()){
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
                                    if(!ntk.is_constant(childIdx) && !child && partitionInputs[i].find(childIdx) == partitionInputs[i].end()){
                                        partitionInputs[i].insert(ntk.get_node(fn));
                                    }
                                }
                            });
                                
                        }
                    }
                });
                
                std::cout << "Partition " << i << " Inputs {";
                for(auto node : partitionInputs[i]){
                    std::cout << ntk.node_to_index(node) << " ";
                }
                std::cout << "}\n";
                std::cout << "Partition " << i << " Outputs{";
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

        oracle::partition_view<Ntk> create_part( Ntk const& ntk, int part ){
            oracle::partition_view<Ntk> partition(ntk, partitionInputs[part], partitionOutputs[part], false);
            return partition;
        }
        int get_part_num(){
            return num_partitions;
        }

        template<class NtkPart, class NtkOpt>
        void synchronize_part(oracle::partition_view<NtkPart> part, NtkOpt const& opt, Ntk const& ntk){

            // std::vector<node> dfs_order;
            // std::unordered_map<node, int> visited;
            // opt.foreach_node( [&](auto node){
            //     visited[node] = 0;
            // });
            // opt.foreach_po( [&]( auto po ) {
            //     dfs(opt, po, visited, dfs_order);
            //     std::cout << "DFS for opt done\n";
            //     for(int i = 0; i < dfs_order.size(); i++){
            //         std::cout << dfs_order.at(i) << " ";
            //     }
            //     std::cout << "\n";
            // });
            // visited.clear();
            // part.foreach_node( [&](auto node){
            //     visited[node] = 0;
            // });
            // std::vector<node> dfs_order_part;
            // part.foreach_po( [&]( auto po ) {
            //     dfs(part, po, visited, dfs_order_part);
            //     std::cout << "DFS for part done\n";
            //     for(int i = 0; i < dfs_order_part.size(); i++){
            //         std::cout << dfs_order_part.at(i) << " ";
            //     }
            //     std::cout << "\n";
            // });
                
            mockturtle::node_map<signal, NtkOpt> old_to_new( opt );
            // std::unordered_map<signal, signal> old_to_new;
            std::vector<signal> pis;
            part.foreach_pi( [&]( auto node ) {
                //need to match with pis from original circuit using partition_view
                std::cout << "part pi " << part.make_signal(node).index << " added\n";
                // old_to_new[node] = part.make_signal(node);
                pis.push_back(part.make_signal(node));
            });

            mockturtle::topo_view part_top{part};
            mockturtle::topo_view opt_top{opt};
            std::vector<node> opt_nodes = opt_top.get_top_view_nodes();
            std::cout << "opt_top size = " << opt_nodes.size() << "\n";
            int pi_idx = 0;
            std::set<signal> visited_pis;
            opt_top.foreach_node( [&]( auto node ) {

                if ( opt.is_constant( node ) || opt.is_ci( node ) )
                    return;

                /* collect children */
                std::vector<signal> children;
                std::cout << "nodeIdx = " << opt.node_to_index(node) << "\n";
                opt.foreach_fanin( node, [&]( auto child, auto ) {
                    //Update to add corresponding children in original network instead of indeces from the optimized network
                    const auto f = old_to_new[child];
                    if(opt.is_pi(opt.get_node(child))){
                        std::cout << "pi\n";
                        f = pis.at(child.index - 1);
                    }

                    if ( opt.is_complemented( child ) )
                    {
                        std::cout << "adding child " << f.index << "\n";
                        children.push_back( ntk.create_not( f ) );
                    }
                    else
                    {
                        std::cout << "adding child " << f.index << "\n";
                        children.push_back( f );
                    }
                } );

                // std::cout << "cloning node\n";
                old_to_new[node] = ntk.clone_node( opt, node, children );
                std::cout << "after clone = " << ntk.num_gates() << "\n";
                ntk.foreach_gate( [&](auto new_node){
                    std::cout << "nodeIdx = " << ntk.node_to_index(new_node) << "\n";
                    std::cout << "child[0] = " << ntk._storage->nodes[new_node].children[0].index << "\n";
                    std::cout << "child[1] = " << ntk._storage->nodes[new_node].children[1].index << "\n";
                    std::cout << "child[2] = " << ntk._storage->nodes[new_node].children[2].index << "\n";
                });
                // std::cout << "substituting node = " << ntk.node_to_index(part_node) << "\n";
                // ntk.foreach_fanin( part_node, [&] (auto fn){
                //     std::cout << "original node child = " << fn.index << "\n";
                // });
                // ntk.substitute_node(part_node, clone);
                // std::cout << "updated ntk size = " << ntk.num_gates() << "\n";
                // ntk.foreach_gate( [&](auto node){
                //     std::cout << "nodeIdx = " << ntk.node_to_index(node) << "\n";
                //     std::cout << "child[0] = " << ntk._storage->nodes[node].children[0].index << "\n";
                //     std::cout << "child[1] = " << ntk._storage->nodes[node].children[1].index << "\n";
                //     std::cout << "child[2] = " << ntk._storage->nodes[node].children[2].index << "\n";
                // });
            });

            opt.foreach_po( [&]( auto po ) {
                // const auto f = old_to_new[po];
                // std::cout << "cleanup pushing po\n";
                if ( opt.is_complemented( po ) )
                {
                    ntk.create_not( po );
                }
            } );

            for(int i = 0; i < opt._storage->outputs.size(); i++){
                auto opt_out = old_to_new[opt._storage->outputs.at(i)];
                auto part_out = part._roots.at(i);
                std::cout << "substituting " << part_out.index << " with " << opt_out.index << "\n";
                auto part_node = part.index_to_node(part_out.index);
                std::cout << "made part_node\n";
                ntk.substitute_node(ntk.get_node(part_out), opt_out);
            }
            
            // std::vector<node> opt_nodes = opt_top.get_top_view_nodes();

            // for(int i = 0; i < opt_nodes.size(); i++){
            //     node opt_node = opt_nodes.at(i);
            //     node part_node = part_top._nodes.at(i);
            //     int ntk_nodeIdx = part_top.node_to_index(part_node);
            //     std::vector<signal> children;
            //     if (!opt_top.is_pi(opt_top.node_to_index(opt_node)) && !opt_top.is_ci(opt_top.node_to_index(opt_node))){
            //         std::cout << "opt_top nodeIdx = " << opt_top.node_to_index(opt_node) << "\n";
            //         part_top.foreach_fanin( part_node, [&] (auto fn){
            //             std::cout << "adding child = " << fn.index << "\n";
            //             children.push_back(fn);
            //         });
            //         std::cout << "before substitution ntk size = " << ntk.num_gates() << "\n";
            //         ntk.foreach_gate( [&](auto node){
            //             std::cout << "nodeIdx = " << ntk.node_to_index(node) << "\n";
            //             std::cout << "child[0] = " << ntk._storage->nodes[node].children[0].index << "\n";
            //             std::cout << "child[1] = " << ntk._storage->nodes[node].children[1].index << "\n";
            //             std::cout << "child[2] = " << ntk._storage->nodes[node].children[2].index << "\n";
            //         });
            //         auto clone = ntk.clone_node(opt_top, opt_node, children);
            //         std::cout << "updated after clone ntk size = " << ntk.num_gates() << "\n";
            //         std::cout << "substituting node = " << ntk.node_to_index(part_node) << "\n";
            //         ntk.foreach_fanin( part_node, [&] (auto fn){
            //             std::cout << "original node child = " << fn.index << "\n";
            //         });
            //         ntk.substitute_node(part_node, clone);
            //         std::cout << "updated ntk size = " << ntk.num_gates() << "\n";
            //         ntk.foreach_gate( [&](auto node){
            //             std::cout << "nodeIdx = " << ntk.node_to_index(node) << "\n";
            //             std::cout << "child[0] = " << ntk._storage->nodes[node].children[0].index << "\n";
            //             std::cout << "child[1] = " << ntk._storage->nodes[node].children[1].index << "\n";
            //             std::cout << "child[2] = " << ntk._storage->nodes[node].children[2].index << "\n";
            //         });
            //     }
                
                
            // }
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
