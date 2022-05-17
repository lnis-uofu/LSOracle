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
#include <alice/alice.hpp>
#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <stdlib.h>

#include "utility.hpp"
#include "kahypar_config.hpp"
#include "algorithms/partitioning/partition_manager_junior.hpp"
#include "algorithms/partitioning/kahypar_partitioner.hpp"
namespace alice
{
    class kahypar_command : public alice::command
    {

    public:
        using aig_names = mockturtle::names_view<mockturtle::aig_network>;
        using aig_ntk = std::shared_ptr<aig_names>;
        using part_man_aig = oracle::partition_manager<aig_names>;
        using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

        using mig_names = mockturtle::names_view<mockturtle::mig_network>;
        using mig_ntk = std::shared_ptr<mig_names>;
        using part_man_mig = oracle::partition_manager<mig_names>;
        using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

        explicit kahypar_command(const environment::ptr &env)
            : command(env,
                      "Partitions current network using KaHyPar k-means hypergraph partitioner")
            {
                add_flag("--aig,-a", "Partition stored AIG (Default)");
                add_flag("--mig,-m", "Partition stored MIG");
                add_flag("--xag,-x", "Partition stored XAG");
                add_flag("--xmg,-g", "Partition stored XMG");

                auto num_opt = opts.add_option("--num,num", num_partitions,
                                               "Number of desired partitions");
                opts.add_option("--size", size_partitions,
                                "Number of desired average nodes per partition.")->excludes(num_opt);
                opts.add_option("--config_direc,-c", config_direc,
                                "Path to the configuration file for KaHyPar.");
                opts.add_option("--initial,-i", initial_file,
                                "External file to write the initial partitions to.");
                opts.add_option("--node_weights,-n", node_weight_file,
                                "External file containing node weights");
                opts.add_option("--edge_weights,-e", edge_weight_file,
                                "External file containing edge weights");
                add_flag("--sap,-s", "Apply Structure Aware Partitioning");
                opts.add_option("--epsilon", imbalance,
                                "Hypergraph partitioning epsilon imbalance parameter.");
            }

    protected:

        template <typename network>
        void partition_network(string name)
        {
            if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
                spdlog::error("{} network not stored", name);
                return;
            }
            mockturtle::names_view<network> ntk =
                *store<std::shared_ptr<mockturtle::names_view<network>>>().current();


            if (num_partitions == 0) {
                num_partitions = std::max(ntk.size() / size_partitions, 1u);
            }
            spdlog::info("Using {} partitions", num_partitions);

            int *node_weights = nullptr;
            int *edge_weights = nullptr;
            if (edge_weight_file != "") {
                spdlog::info("Reading edge weights from {}", edge_weight_file);
                std::vector<int> data = read_integer_file(edge_weight_file);
                edge_weights = &data[0];
            }
            if (node_weight_file != "") {
                spdlog::info("Reading node weights from {}",node_weight_file);
                std::vector<int> data = read_integer_file(node_weight_file);
                if (data.size() != ntk.size()) {
                    spdlog::info("Node weight file contains the incorrect number of nodes: got {} expected {}",
                                 data.size(), ntk.size());
                    exit(1);
                } else {
                    node_weights = &data[0];
                }
            }

            if (config_direc == "") {
                config_direc = make_temp_config();
            }
            spdlog::info("Using KaHyPar configuration {}",config_direc);

            oracle::kahypar_partitioner<network> partitioner(ntk,
                                                                  num_partitions,
                                                                  config_direc,
                                                                  node_weights,
                                                                  edge_weights,
                                                                  imbalance);

            store<std::shared_ptr<oracle::partition_manager_junior<network>>>().extend() =
                std::make_shared<oracle::partition_manager_junior<network>>(partitioner.partition_manager());
        }
        void execute()
        {
            if (is_set("mig")) {
                partition_network<mockturtle::mig_network>("MIG");
            } else if (is_set("xag")) {
                partition_network<mockturtle::xag_network>("XAG");
            } else if (is_set("xmg")) {
                partition_network<mockturtle::xmg_network>("XMG");
            } else {
                partition_network<mockturtle::aig_network>("AIG");
            }
        }
    private:
        uint32_t num_partitions = 0;
        uint32_t size_partitions = 2048;
        std::string config_direc = "";
        // std::string output_file = "";
        std::string initial_file = "";
        std::string edge_weight_file = "";
        std::string node_weight_file = "";
        double imbalance = 0.9;
    };

    ALICE_ADD_COMMAND(kahypar, "Partitioning");
}
