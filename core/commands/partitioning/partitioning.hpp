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
#include <alice/alice.hpp>
#include <mockturtle/mockturtle.hpp>
#include "algorithms/partitioning/partition_manager.hpp"

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>
// #include <utah/BiPart.h>
#include "kahypar_config.hpp"
#ifdef ENABLE_GALOIS
#include <utah/BiPart.h>
#endif

namespace alice
{
class partitioning_command : public alice::command
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

    explicit partitioning_command(const environment::ptr &env)
        : command(env,
                  "Partitions current network using k-means hypergraph partitioner")
    {

        auto num_opt = opts.add_option("--num,num", num_partitions,
                                       "Number of desired partitions");
        opts.add_option("--size", size_partitions,
                        "Number of desired average nodes per partition.")->excludes(num_opt);
        opts.add_option("--config_direc,-c", config_direc,
                        "Path to the configuration file for KaHyPar.");
        opts.add_option("--file,-f", part_file,
                        "External file containing partition information");
        opts.add_option("--output,-o", output_file,
                        "External file to write the generated partitions to.");
        opts.add_option("--initial,-i", initial_file,
                        "External file to write the initial partitions to.");
        opts.add_option("--node_weights,-n", node_weight_file,
                        "External file containing node weights");
        opts.add_option("--edge_weights,-e", edge_weight_file,
                        "External file containing edge weights");
        add_flag("--mig,-m", "Partitions stored MIG network (AIG network is default)");
        add_flag("--sap,-s", "Apply Structure Aware Partitioning");
        opts.add_option("--epsilon", imbalance,
                        "Hypergraph partitioning epsilon imbalance parameter.");
#ifdef ENABLE_GALOIS
        add_flag("--bipart,-g",
                 "Run hypergraph partitionining using BiPart from the Galois system");
#endif
    }

protected:
    std::vector<int> read_file(string filename)
    {
        std::vector<int> output;
        std::ifstream ifs;

        ifs.open(filename);
        if (ifs.is_open()) {
            while (ifs.good()) {
                std::string part;
                getline(ifs, part);
                if (part != "")
                    output.push_back(std::stoi(part));
            }
            ifs.close();
            return output;
        } else {
            env->err() << "Unable to open partition data file\n";
            throw exception();
        }
    }

    template <typename gen_ntk, typename gen_names, typename part_man_gen, typename part_man_gen_ntk, typename node_type>
    void process_network(string type_name)
    {
        auto ntk = *store<gen_ntk>().current();
        if (part_file != "") {
            env->out() << "Partitioning stored " << type_name <<
                       " network using external file\n";
            std::map<node_type, int> part_data;
            std::vector<int> parts = read_file(part_file);
            if (parts.size() != ntk.size()) {
                env->out() << "Partition file contains the incorrect number of nodes\n";
                exit(1);
            }
            for (int i = 0; i < parts.size(); i++) {
                part_data[ntk.index_to_node(i)] = parts[i];
            }
            oracle::partition_manager<gen_names> partitions(ntk, part_data, num_partitions);
            store<part_man_gen_ntk>().extend() = std::make_shared<part_man_gen>(partitions);
        } else {
#ifdef ENABLE_GALOIS
            if (is_set("bipart")) {
                env->out() << "Partitioning stored " << type_name <<
                           " network using Galois BiPart\n";
                oracle::hypergraph<gen_names> t(ntk);
                uint32_t num_vertices = 0;

                t.get_hypergraph(ntk);
                std::vector<std::vector<uint32_t>> hedges = t.get_hyperedges();
                num_vertices = t.get_num_vertices();

                int num_threads = 14;
                scheduleMode mode = PP;
                std::map<int, int> bipart = biparting(hedges, num_vertices, num_partitions,
                                                      num_threads, mode);
                std::map<node_type, int> part_data;
                ntk.foreach_node([&](auto node) {
                    part_data[node] = bipart[node];
                });

                oracle::partition_manager<gen_names> partitions(ntk, part_data, num_partitions);
                store<part_man_gen_ntk>().extend() = std::make_shared<part_man_gen>(partitions);
            } else
#endif
            {
                env->out() << "Partitioning stored " << type_name << " network using KaHyPar\n";

                int *node_weights = nullptr;
                int *edge_weights = nullptr;
                if (edge_weight_file != "") {
                    env->out() << "Reading edge weights from " << edge_weight_file << std::endl;
                    std::vector<int> data = read_file(edge_weight_file);
                    edge_weights = &data[0];
                }
                if (node_weight_file != "") {
                    env->out() << "Reading node weights from " << node_weight_file << std::endl;
                    std::vector<int> data = read_file(node_weight_file);
                    if (data.size() != ntk.size()) {
                        env->out() << "Node weight file contains the incorrect number of nodes: got " <<
                                   data.size() << " expected " << ntk.size() << std::endl;
                        exit(1);
                    } else {
                        node_weights = &data[0];
                    }
                }

                if (num_partitions == 0) {
                    num_partitions = std::max(ntk.size() / size_partitions, 1u);
                }
                env->out() << "Using " << num_partitions << " partitions" << std::endl;

                if (config_direc == "") {
                    config_direc = make_temp_config();
                }
                env->out() << "Using KaHyPar configuration " << config_direc << std::endl;

                if (is_set("sap")) {
                    env->out() << "Using structure aware partitioning" << std::endl;
                }

                oracle::partition_manager<gen_names> partitions(ntk, num_partitions,
                        config_direc, node_weights, edge_weights, is_set("sap"), imbalance);
                store<part_man_gen_ntk>().extend() = std::make_shared<part_man_gen>(partitions);

                if (output_file != "") {
                    std::ofstream out;
                    out.open(output_file);
                    auto parts = partitions.get_partitions();
                    for (auto i = parts.begin(); i != parts.end(); i++) {
                        out << *i << std::endl;
                    }
                    out.close();
                }

                if (initial_file != "") {
                    std::ofstream out;
                    out.open(initial_file);
                    auto init = partitions.get_initial_partitions();
                    for (auto i = init.begin(); i != init.end(); i++) {
                        out << *i << std::endl;
                    }
                    out.close();
                }
            }
        }
    }
    void execute()
    {
        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;

        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {
                process_network<mig_ntk, mig_names, part_man_mig, part_man_mig_ntk, mockturtle::mig_network::node>("MIG");
            } else {
                env->err() << "MIG network not stored\n";
            }
        } else {
            if (!store<aig_ntk>().empty()) {
                process_network<aig_ntk, aig_names, part_man_aig, part_man_aig_ntk, mockturtle::aig_network::node>("AIG");
            } else {
                env->err() << "AIG network not stored\n";
            }
        }
    }
private:
    uint32_t num_partitions = 0;
    uint32_t size_partitions = 2048;
    std::string config_direc = "";
    std::string part_file = "";
    std::string output_file = "";
    std::string initial_file = "";
    std::string edge_weight_file = "";
    std::string node_weight_file = "";
    double imbalance = 0.9;
};

ALICE_ADD_COMMAND(partitioning, "Partitioning");
}
