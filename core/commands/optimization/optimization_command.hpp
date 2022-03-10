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


namespace alice
{
class optimization_command : public alice::command
{

public:
    explicit optimization_command(const environment::ptr &env)
        : command(env, "Perform Mixed Synthesis on Network after Partitioning")
    {

        opts.add_option("--nn_model,-n", nn_model,
                        "Trained neural network model for classification");
        opts.add_option("--out,-o", out_file, "Verilog output");
        opts.add_option("--strategy,-s", strategy,
                        "classification strategy [area delay product=0, area=1, delay=2, delay_threshold=3]");
        opts.add_option("--threshold", threshold,
                        "maximum delay threshold for strategy 3");
        opts.add_flag("--fixed_initial", "Depth optimize initial partitions.");
        opts.add_option("--aig_partitions", aig_parts,
                        "space separated list of partitions to always be AIG optimized");
        opts.add_option("--mig_partitions", mig_parts,
                        "space separated list of partitions to always be MIG optimized");
        opts.add_option("--depth_partitions", depth_parts,
                        "space separated list of partitions to always be depth optimized");
        opts.add_option("--skip_partitions", skip_parts,
                        "space separated list of partitions that will not be optimized");
        opts.add_option("--area_partitions", area_parts,
                        "space separated list of partitions to always be area optimized");
        add_flag("--aig,-a", "Perform only AIG optimization on all partitions");
        add_flag("--mig,-m", "Perform only MIG optimization on all partitions");
        add_flag("--combine,-c",
                 "Combine adjacent partitions that have been classified for the same optimization");
        //add_flag("--skip-feedthrough", "Do not include feedthrough nets when writing out the file");
    }

protected:
    void execute()
    {

        if (!store<aig_ntk>().empty()) {
            auto ntk_aig = *store<aig_ntk>().current();
            mockturtle::depth_view orig_depth{ntk_aig};
            if (!store<part_man_aig_ntk>().empty()) {
                auto partitions_aig = *store<part_man_aig_ntk>().current();
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

                if (is_set("fixed_initial")) {
                    std::set<kahypar_partition_id_t> fixed_partitions =
                        partitions_aig.fixed_partitions();
                    std::copy(fixed_partitions.begin(), fixed_partitions.end(),
                              std::inserter(depth_parts, depth_parts.end()));
                }

                if (threshold == 0 && strategy == 3) {
                    threshold = calculate_depth_percentile(ntk_aig);
                }
                if (strategy == 3) {
                    env->out() << "Using threshold " << threshold << " max depth for optimization."
                               << std::endl;
                }

                std::copy(aig_parts.begin(), aig_parts.end(),
                          std::inserter(aig_always_partitions, aig_always_partitions.end()));
                std::copy(mig_parts.begin(), mig_parts.end(),
                          std::inserter(mig_always_partitions, mig_always_partitions.end()));
                std::copy(area_parts.begin(), area_parts.end(),
                          std::inserter(area_always_partitions, area_always_partitions.end()));
                std::copy(depth_parts.begin(), depth_parts.end(),
                          std::inserter(depth_always_partitions, depth_always_partitions.end()));
                std::copy(skip_parts.begin(), skip_parts.end(), std::inserter(skip_partitions,
                          skip_partitions.end()));

                auto start = std::chrono::high_resolution_clock::now();
                auto ntk_mig = oracle::optimization(ntk_aig, partitions_aig, strategy,
                                                    threshold, nn_model,
                                                    high, aig, mig, combine,
                                                    aig_always_partitions, mig_always_partitions,
                                                    depth_always_partitions, area_always_partitions,
                                                    skip_partitions);
                auto stop = std::chrono::high_resolution_clock::now();


                mockturtle::depth_view new_depth{ntk_mig};
                if (ntk_mig.size() != ntk_aig.size()
                        || orig_depth.depth() != new_depth.depth()) {
                    env->out() << "Final ntk size = " << ntk_mig.num_gates() << " and depth = " <<
                               new_depth.depth() << "\n";
                    env->out() << "Final number of latches = " << ntk_mig.num_latches() << "\n";
                    env->out() << "Area Delay Product = " << ntk_mig.num_gates() * new_depth.depth()
                               << "\n";
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                                    (stop - start);
                    env->out() << "Full Optimization: " << duration.count() << "ms\n";
                    env->out() << "Finished optimization\n";
                    store<mig_ntk>().extend() = std::make_shared<mig_names>(ntk_mig);

                    if (out_file != "") {
                        if (oracle::checkExt(out_file, "v")) {
                            mockturtle::write_verilog_params ps;

                            //removing to use stock mockturtle I/O.  May be nice to have in the future
                            //if(is_set("skip-feedthrough"))
                            //  ps.skip_feedthrough = 1u;

                            mockturtle::write_verilog(ntk_mig, out_file, ps);
                            env->out() << "Resulting network written to " << out_file << "\n";
                        } else if (oracle::checkExt(out_file, "blif")) {
                            mockturtle::write_blif_params ps;
                            //if(is_set("skip-feedthrough"))
                            //  ps.skip_feedthrough = 1u;

                            mockturtle::write_blif(ntk_mig, out_file, ps);
                            env->out() << "Resulting network written to " << out_file << "\n";
                        } else {
                            env->err() << out_file << " is not an accepted output file {.v, .blif}\n";
                        }
                    }
                } else {
                    env->out() << "No change made to network\n";
                }

            } else {
                env->err() << "AIG not partitioned yet\n";
            }
        } else {
            env->err() << "No AIG stored\n";
        }
    }
private:
    uint32_t calculate_depth_percentile(
        mockturtle::names_view<mockturtle::aig_network> net)
    {
        std::vector<uint32_t> depths;
        mockturtle::depth_view view(net);
        net.foreach_po([this, &depths, &view](auto po) {
            depths.push_back(view.level(view.get_node(po)));
        });
        std::sort(depths.begin(), depths.end());
        if (depths.size() == 1) {
            return depths[0];
        } else if (depths.size() == 0) {
            return 0;
        } else {
            // 95 percentile element or the second to last value, whichever is earlier.
            size_t index = (size_t) min(ceil(0.95 * depths.size()),
                                        (double)(depths.size() - 1)) - 1;
            return depths[index];
        }
    }
    std::string nn_model{};
    std::string out_file{};
    std::vector<int32_t> aig_parts{};
    std::vector<int32_t> mig_parts{};
    std::vector<int32_t> area_parts{};
    std::vector<int32_t> depth_parts{};
    std::vector<int32_t> skip_parts{};
    unsigned strategy{0u};
    unsigned threshold{0u};
    bool high = false;
    bool aig = false;
    bool mig = false;
    bool combine = false;
    std::set<int32_t> aig_always_partitions = {};
    std::set<int32_t> mig_always_partitions = {};
    std::set<int32_t> depth_always_partitions = {};
    std::set<int32_t> area_always_partitions = {};
    std::set<int32_t> skip_partitions = {};
};

ALICE_ADD_COMMAND(optimization, "Optimization");
}
