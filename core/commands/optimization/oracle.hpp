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
#include <math.h>
#include "algorithms/optimization/optimization_test.hpp"

namespace alice
{
class oracle_command : public alice::command
{

public:
    explicit oracle_command(const environment::ptr &env)
        : command(env,
                  "Partitions Stored AIG network, and Performs Mixed Synthesis on Network")
    {

        opts.add_option("--partition,partition", num_partitions,
                        "Number of partitions (Network Size / 300 set as partitition number if not specified)");
        opts.add_option("--nn_model,-n", nn_model,
                        "Trained neural network model for classification");
        opts.add_option("--out,-o", out_file,
                        "output file to write resulting network to [.v, .blif]");
        opts.add_option("--strategy,-s", strategy,
                        "classification strategy [area delay product{DEFAULT}=0, area=1, delay=2]");
        opts.add_option("--config,-f", config_file, "Config file", true);
        // add_flag("--bipart,-g", "Use BiPart from the Galois system for partitioning");
        add_flag("--aig,-a", "Perform only AIG optimization on all partitions");
        add_flag("--mig,-m", "Perform only MIG optimization on all partitions");
        add_flag("--combine,-c",
                 "Combine adjacent partitions that have been classified for the same optimization");
        //add_flag("--skip-feedthrough", "Do not include feedthrough nets when writing out the file");
#ifdef ENABLE_GALOIS
	// TODO replace this flag with a different letter, g is going to be xmg
        add_flag("--bipart,-g", "Use BiPart from the Galois system for partitioning");
#endif
    }

protected:
    void execute()
    {

        if (!store<aig_ntk>().empty()) {
            spdlog::info("\n\n\n1");
            auto ntk = *store<aig_ntk>().current();
            spdlog::info("\n\n\n2");

            //If number of partitions is not specified
            if (num_partitions == 0) {
                spdlog::info("\n\n\nif1");

                double size = ((double) ntk.size()) / 300.0;
                num_partitions = ceil(size);
            }
            spdlog::info("\n\n\n3");

            mockturtle::depth_view orig_depth{ntk};
            if (config_file == "") {
                config_file = make_temp_config();
            }
            spdlog::info("\n\n\n4");
            spdlog::info("constructing partition manager with partitions and config_file: {}", num_partitions, config_file);

            oracle::partition_manager<aig_names> partitions(ntk, num_partitions,
                    config_file);
            spdlog::info("\n\n\n5");

            store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>(partitions);

            spdlog::info("{} partitioned {} times", ntk.get_network_name(), num_partitions);
            if (!nn_model.empty())
                high = false;
            else
                high = true;
            if (is_set("aig"))
                aig = true;
            if (is_set("mig"))
                mig = true;
            if (is_set("combine"))
                combine = true;

            auto start = std::chrono::high_resolution_clock::now();

            auto ntk_mig = oracle::optimization_test(ntk, partitions, strategy, nn_model,
                           high, aig, mig, combine);

            auto stop = std::chrono::high_resolution_clock::now();

            mockturtle::depth_view new_depth{ntk_mig};
            if (ntk_mig.size() != ntk.size() || orig_depth.depth() != new_depth.depth()) {
                spdlog::info("Final ntk size = {} and depth = {}", ntk_mig.num_gates(), new_depth.depth());
                spdlog::info("Final number of latches = {}",ntk_mig.num_latches());
                // spdlog::info("Area Delay Product = {}",ntk_mig.num_gates() * new_depth.depth());
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                                (stop - start);
                spdlog::info("Full Optimization: {}ms", duration.count());
                // spdlog::info("Finished optimization");
                store<mig_ntk>().extend() = std::make_shared<mig_names>(ntk_mig);
                spdlog::info("MIG network stored");

                if (out_file != "") {
                    if (oracle::checkExt(out_file, "v")) {
                        mockturtle::write_verilog_params ps;
                        //might be nice to have again, but for now commenting this out to allow us to use stock mockturtle I/O
                        //if(is_set("skip-feedthrough"))
                        //ps.skip_feedthrough = 1u;

                        mockturtle::write_verilog(ntk_mig, out_file, ps);
                        spdlog::info("Resulting network written to {}",out_file);
                    } else if (oracle::checkExt(out_file, "blif")) {
                        mockturtle::write_blif_params ps;
                        //if(is_set("skip-feedthrough"))
                        //ps.skip_feedthrough = 1u;

                        mockturtle::write_blif(ntk_mig, out_file, ps);
                        spdlog::info("Resulting network written to {}",out_file);
                    } else {
                        spdlog::error("{} is not an accepted output file {.v, .blif}", out_file);
                    }
                }
            } else {
                spdlog::info("No change made to network");
                store<mig_ntk>().extend() = std::make_shared<mig_names>(ntk_mig);
                spdlog::info("MIG network stored");
            }
        } else {
            spdlog::error("AIG network not stored");
        }
    }
private:
    std::string filename{};
    int num_partitions{0u};
    std::string nn_model{};
    std::string out_file{};
    std::string config_file{};
    unsigned strategy{0u};
    bool high = false;
    bool aig = false;
    bool mig = false;
    bool combine = false;
#ifdef ENABLE_GALOIS
    bool bipart = false;
#endif
};

ALICE_ADD_COMMAND(oracle, "Optimization");
}
