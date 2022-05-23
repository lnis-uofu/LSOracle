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
#include <stdlib.h>
#include <math.h>
#include "algorithms/optimization/optimization_test.hpp"
#include <sys/stat.h>
#include "algorithms/optimization/resynthesis.hpp"


namespace alice
{
class oracle_command : public alice::command
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

        explicit oracle_command(const environment::ptr &env)
            : command(env,
                      "Partitions current network using KaHyPar k-means hypergraph partitioner")
            {
                add_flag("--aig,-a", "Partition stored AIG (Default)");
                add_flag("--mig,-m", "Partition stored MIG");
                add_flag("--xag,-x", "Partition stored XAG");
                add_flag("--xmg,-g", "Partition stored XMG");

                auto num_opt = opts.add_option("--num,num", num_partitions,
                                               "Number of desired partitions");
                opts.add_option("--abc_exec", abc_exec,
                        "ABC executable, defaults to using path.");
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
                opts.add_option("--strategy",strategych, "Strategy for optimization");
                opts.add_option("--out,-o", out_file,
                        "output file to write resulting network to [.v, .blif]");
                
            }

    protected:

        template <typename network>
        void partition_network(string name)
        {
            if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
                env->err() << name << " network not stored\n";
                return;
            }
            mockturtle::names_view<network> ntk =
                *store<std::shared_ptr<mockturtle::names_view<network>>>().current();


            if (num_partitions == 0) {
                num_partitions = std::max(ntk.size() / size_partitions, 1u);
            }
            env->out() << "Using " << num_partitions << " partitions" << std::endl;

            int *node_weights = nullptr;
            int *edge_weights = nullptr;
            if (edge_weight_file != "") {
                env->out() << "Reading edge weights from " << edge_weight_file << std::endl;
                std::vector<int> data = read_integer_file(edge_weight_file);
                edge_weights = &data[0];
            }
            if (node_weight_file != "") {
                env->out() << "Reading node weights from " << node_weight_file << std::endl;
                std::vector<int> data = read_integer_file(node_weight_file);
                if (data.size() != ntk.size()) {
                    env->out() << "Node weight file contains the incorrect number of nodes: got " <<
                        data.size() << " expected " << ntk.size() << std::endl;
                    exit(1);
                } else {
                    node_weights = &data[0];
                }
            }

            if (config_direc == "") {
                config_direc = make_temp_config();
            }
            env->out() << "Using KaHyPar configuration " << config_direc << std::endl;

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
        synth<mockturtle::aig_network>("AIG");
        }

        template <typename network>
        void synth(string name){

        if (is_set("mig")) {
            partition_network<mockturtle::mig_network>("MIG");
        } else if (is_set("xag")) {
            partition_network<mockturtle::xag_network>("XAG");
        } else if (is_set("xmg")) {
            partition_network<mockturtle::xmg_network>("XMG");
        } else {
            partition_network<mockturtle::aig_network>("AIG");
        }

        auto ntk = *store<std::shared_ptr<mockturtle::names_view<network>>>().current();
        oracle::partition_manager_junior<network> partitions_jr =
            *store<std::shared_ptr<oracle::partition_manager_junior<network>>>().current();

        mockturtle::depth_view orig_depth(partitions_jr.get_network());
        auto start = std::chrono::high_resolution_clock::now();
        mockturtle::names_view<mockturtle::xmg_network> ntk_result;

	    oracle::optimization_strategy strategy;
        if (strategych=="depth") {
		strategy = oracle::optimization_strategy::depth;
	    } else if (strategych=="nodes") {
		strategy = oracle::optimization_strategy::size;
	    } else {
		strategy = oracle::optimization_strategy::balanced;
	    }
        ntk_result = oracle::optimize_basic<network>(partitions_jr, abc_exec, strategy,false);

        auto stop = std::chrono::high_resolution_clock::now();
        mockturtle::depth_view new_depth(ntk_result);


        if (ntk_result.size() == partitions_jr.get_network().size()
                && orig_depth.depth() == new_depth.depth()) {
            env->err() << "No change made to network" << std::endl;
        }

        env->out() << "Final ntk size = " << ntk_result.num_gates() << " and depth = "
                    << new_depth.depth() << "\n";
        env->out() << "Final number of latches = " << ntk_result.num_latches() << "\n";
        env->out() << "Node Depth Product = "
                    << ntk_result.num_gates() * new_depth.depth()
                    << "\n";
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                        (stop - start);
        env->out() << "Full Optimization: " << duration.count() << "ms\n";
        env->out() << "Finished optimization\n";
        store<std::shared_ptr<mockturtle::names_view<mockturtle::xmg_network>>>().extend() =
                    std::make_shared<mockturtle::names_view<mockturtle::xmg_network>>(ntk_result);
        if (out_file != "") {
                    if (oracle::checkExt(out_file, "v")) {
                        mockturtle::write_verilog_params ps;
                        //might be nice to have again, but for now commenting this out to allow us to use stock mockturtle I/O
                        //if(is_set("skip-feedthrough"))
                        //ps.skip_feedthrough = 1u;

                        mockturtle::write_verilog(ntk_result, out_file, ps);
                        env->out() << "Resulting network written to " << out_file << "\n";
                    } else if (oracle::checkExt(out_file, "blif")) {
                        mockturtle::write_blif_params ps;
                        //if(is_set("skip-feedthrough"))
                        //ps.skip_feedthrough = 1u;

                        mockturtle::write_blif(ntk_result, out_file, ps);
                        env->out() << "Resulting network written to " << out_file << "\n";
                    } else {
                        env->err() << out_file << " is not an accepted output file {.v, .blif}\n";
                    }
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
        std::string out_file{};
        std:: string strategych={""};
        string abc_exec{"abc"};
        
    };

ALICE_ADD_COMMAND(oracle, "Optimization");
}
