/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
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

#include <memory>
#include <iostream>
#include <thread>

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <cassert>
#include <queue>
#include <mockturtle/mockturtle.hpp>
#include <kitty/kitty.hpp>
//#include <libkahypar.h>

#include "algorithms/partitioning/partition_view.hpp"
#include "algorithms/partitioning/structure_partition.hpp"
#include "algorithms/partitioning/hyperg.hpp"
#include "kahypar_config.hpp"
#include "utility.hpp"
#include <mtkahypar.h>
#include "algorithms/partitioning/slack_view.hpp"

namespace oracle
{
template <typename network>

class kahypar_partitioner
{
    using Ntk = mockturtle::names_view<network>;
    using node = typename Ntk::node;
public:
    kahypar_partitioner(Ntk &ntk, int part_num, std::string config_direc = "",
                    std::string target_graph_file = "", mt_kahypar_hypernode_weight_t *hypernode_weights = nullptr,
                        mt_kahypar_hyperedge_weight_t *hyperedge_weights = nullptr,
                        double imbalance = 0.9): ntk(ntk), part_num(part_num), node_partition(ntk)
    {

        // mockturtle::incomplete_node_map<kahypar_hyperedge_id_t, Ntk> initial_partitions;
        static_assert(mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type");
        static_assert(mockturtle::has_set_visited_v<Ntk>,
                      "Ntk does not implement the set_visited method");
        static_assert(mockturtle::has_visited_v<Ntk>,
                      "Ntk does not implement the visited method");
        static_assert(mockturtle::has_get_node_v<Ntk>,
                      "Ntk does not implement the get_node method");
        static_assert(mockturtle::has_get_constant_v<Ntk>,
                      "Ntk does not implement the get_constant method");
        static_assert(mockturtle::has_is_constant_v<Ntk>,
                      "Ntk does not implement the is_constant method");
        static_assert(mockturtle::has_make_signal_v<Ntk>,
                      "Ntk does not implement the make_signal method");


        std::cout << "num_partitions: " << part_num << "\n";

        if (part_num == 1) {
            ntk.foreach_node([&](auto n) { node_partition[n] = 0; });
            return;
        }

        uint32_t kahyp_num_hyperedges = 0;
        uint32_t kahyp_num_vertices = 0;
        uint32_t kahyp_num_indeces_hyper = 0;
        unsigned long kahyp_num_sets = 0;
        std::vector<uint32_t> kahypar_connections;
        std::vector<unsigned long> kahyp_set_indeces;

        /******************
        Generate HyperGraph
        ******************/

        hypergraph<Ntk> t(ntk);
        t.get_hypergraph(ntk);
        t.return_hyperedges(kahypar_connections);
        kahyp_num_hyperedges = t.get_num_edges();
        kahyp_num_vertices = t.get_num_vertices();
        kahyp_num_indeces_hyper = t.get_num_indeces();
        kahyp_num_sets = t.get_num_sets();
        t.get_indeces(kahyp_set_indeces);

        mt_kahypar_error_t error{};
        
        //cam add
        //set all nodes to have the same weight
        // if (hypernode_weights == nullptr) {
        //     hypernode_weights = new mt_kahypar_hypernode_weight_t[kahyp_num_vertices];
        //     for (int i = 0; i < kahyp_num_vertices; i++) {
        //         hypernode_weights[i] = 1;
        //     }
        // }

        // std::vector<node> critical_path = find_critical_path(ntk);

        // for (auto const& n : critical_path) {
        //     auto idx = ntk.node_to_index(n);
        //     hypernode_weights[idx] *= 10; // or any boost factor
        // }

        oracle::slack_view<network> sv(ntk);
        auto critical_path =  sv.find_critical_path(ntk); 

        if (hypernode_weights == nullptr) {
            static std::vector<mt_kahypar_hypernode_weight_t> default_weights(ntk.size(), 1);
            hypernode_weights = default_weights.data();
 
            for (auto const& n : critical_path) {         
                auto idx = ntk.node_to_index(n);
                hypernode_weights[idx] *= 10;
            }
        }

        // mt_kahypar_partition_id_t* fixed_vertices; 
        //std::vector<mt_kahypar_partition_id_t> fixed_vertices(ntk.size(), std::numeric_limits<mt_kahypar_partition_id_t>::max());
        std::vector<mt_kahypar_partition_id_t> fixed_vertices(kahyp_num_vertices, -1);
        for (auto n : critical_path) {
            fixed_vertices[n] = 0;
        }
        
        // if (hypernode_weights == nullptr) {
        //     hypernode_weights = new mt_kahypar_hypernode_weight_t[kahyp_num_vertices];
        //     ntk.foreach_node([&](auto n) {
        //         auto signal = ntk.make_signal(n);
        //         std::string name = ntk.has_name(signal) ? ntk.get_name(signal) : "";
        //         if (name.empty()) {
        //             hypernode_weights[n] = 0;
        //         } else {
        //             if (weight_map.count(name))
        //                 hypernode_weights[n] = weight_map[name];
        //             else 
        //                 hypernode_weights[n] = 0;
        //         }
        //     });
        // }

        


        // Initialize mt-KaHyPar
        mt_kahypar_initialize(
            std::thread::hardware_concurrency(), // Use all available cores
            true // Activate interleaved NUMA allocation policy
        );

        //set number of hyperedges and vertices. These variables are defined by the hyperG command
        const mt_kahypar_hyperedge_id_t num_hyperedges = kahyp_num_hyperedges;
        const mt_kahypar_hypernode_id_t num_vertices = kahyp_num_vertices;

        //set all edges to have the same weight
        if (hyperedge_weights == nullptr) {
            hyperedge_weights = new mt_kahypar_hyperedge_weight_t[kahyp_num_vertices];
            for (int i = 0; i < kahyp_num_vertices; i++) {
                hyperedge_weights[i] = 2;
            }
        }

        //vector with indeces where each set starts
        std::unique_ptr<size_t[]> hyperedge_indices = std::make_unique<size_t[]>
                (kahyp_num_sets + 1);

        for (int j = 0; j < kahyp_num_sets + 1; j++) {
            hyperedge_indices[j] = kahyp_set_indeces[j];
        }

        std::unique_ptr<mt_kahypar_hyperedge_id_t[]> hyperedges =
            std::make_unique<mt_kahypar_hyperedge_id_t[]>(kahyp_num_indeces_hyper);

        for (int i = 0; i < kahyp_num_indeces_hyper; i++) {
            hyperedges[i] = kahypar_connections[i];
        }

        const mt_kahypar_partition_id_t k = part_num;

        mt_kahypar_hyperedge_weight_t objective = 0;

        std::vector<mt_kahypar_partition_id_t> partition(num_vertices, -1);

        // Setup partitioning context
        mt_kahypar_context_t* context = mt_kahypar_context_from_preset(DEFAULT);
        mt_kahypar_set_partitioning_parameters(context, k, imbalance, KM1);

        mt_kahypar_set_seed(42);
        mt_kahypar_set_context_parameter(context, VERBOSE, "1", &error);

        mt_kahypar_hypergraph_t hypergraph = mt_kahypar_create_hypergraph(context,
                                        num_vertices,
                                        num_hyperedges,
                                        hyperedge_indices.get(),
                                        hyperedges.get(),
                                        hyperedge_weights,
                                        hypernode_weights,
                                        &error);

        if (!hypergraph.hypergraph) {
            std::cerr << "Error loading hypergraph: " << error.msg << std::endl;
            std::exit(1);
        }

        //cam add
        mt_kahypar_add_fixed_vertices(
            hypergraph,
            fixed_vertices.data(),
            part_num,  
            &error
        );
        //

        const char* target_cstr = target_graph_file.c_str();
        mt_kahypar_target_graph_t* target_graph = mt_kahypar_read_target_graph_from_file(target_cstr, context, &error);
        
        mt_kahypar_partitioned_hypergraph_t partitioned_hg = mt_kahypar_map(hypergraph, target_graph, context, &error);

        mt_kahypar_improve_partition(partitioned_hg, context, 3, &error);

        mt_kahypar_improve_mapping(partitioned_hg, target_graph, context, 3, &error);

        if (!partitioned_hg.partitioned_hg) {
            std::cerr << "Partitioning failed: " << error.msg << std::endl;
            std::exit(1);
        }

        // Extract Partition Results
        auto partition_results_mt = std::make_unique<mt_kahypar_partition_id_t[]>(mt_kahypar_num_hypernodes(hypergraph));
        mt_kahypar_get_partition(partitioned_hg, partition_results_mt.get());

        for (int i = 0; i < num_vertices; i++) {
            node_partition[i] = partition_results_mt[i];
        }

        // Clean up resources
        mt_kahypar_free_context(context);
        mt_kahypar_free_hypergraph(hypergraph);
        mt_kahypar_free_partitioned_hypergraph(partitioned_hg);
        mt_kahypar_free_target_graph(target_graph);

        // /******************
        // Partition with kahypar
        // ******************/
        // //configures kahypar
        // kahypar_context_t* context = kahypar_context_new();

        // std::cout << "Using config file " << config_direc << std::endl;
        // kahypar_configure_context_from_file(context, config_direc.c_str());

        // //set number of hyperedges and vertices. These variables are defined by the hyperG command
        // const kahypar_hyperedge_id_t num_hyperedges = kahyp_num_hyperedges;
        // const kahypar_hypernode_id_t num_vertices = kahyp_num_vertices;

        // //set all edges to have the same weight
        // if (hyperedge_weights == nullptr) {
        //     hyperedge_weights = new kahypar_hyperedge_weight_t[kahyp_num_vertices];
        //     for (int i = 0; i < kahyp_num_vertices; i++) {
        //         hyperedge_weights[i] = 2;
        //     }
        // }

        // //vector with indeces where each set starts
        // std::unique_ptr<size_t[]> hyperedge_indices = std::make_unique<size_t[]>
        //     (kahyp_num_sets + 1);

        // for (int j = 0; j < kahyp_num_sets + 1; j++) {
        //     hyperedge_indices[j] = kahyp_set_indeces[j];
        // }

        // std::unique_ptr<kahypar_hyperedge_id_t[]> hyperedges =
        //     std::make_unique<kahypar_hyperedge_id_t[]>(kahyp_num_indeces_hyper);

        // for (int i = 0; i < kahyp_num_indeces_hyper; i++) {
        //     hyperedges[i] = kahypar_connections[i];
        // }

        // const kahypar_partition_id_t k = part_num;

        // kahypar_hyperedge_weight_t objective = 0;

        // std::vector<kahypar_partition_id_t> partition(num_vertices, -1);
        // kahypar_hypergraph_t *hypergraph = kahypar_create_hypergraph(k,
        //                                                              num_vertices,
        //                                                              num_hyperedges,
        //                                                              hyperedge_indices.get(),
        //                                                              hyperedges.get(),
        //                                                              hyperedge_weights,
        //                                                              hypernode_weights);
        // kahypar_partition_hypergraph(hypergraph, k, imbalance, &objective, context,
        //                              partition.data());
        // for (int i = 0; i < num_vertices; i++) {
        //     node_partition[i] = partition[i];
        // }

        // kahypar_context_free(context);
    }

    partition_manager_junior<network> partition_manager() {
        return partition_manager_junior<network>(ntk, node_partition, part_num);
    }

private:
    int part_num = 0;
    mockturtle::node_map<int, mockturtle::names_view<network>> node_partition;
    mockturtle::names_view<network> ntk;
};
};
