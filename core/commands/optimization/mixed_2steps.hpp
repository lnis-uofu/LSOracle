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
class mixed_2step_command : public alice::command
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

    explicit mixed_2step_command(const environment::ptr &env)
        : command(env,
                  "Performs AIG optimization on corresponding partitions and then repartition for MIG optimization")
    {
        opts.add_option("--nn_model,-c", nn_model,
                        "Trained neural network model for classification");
        opts.add_option("--num_parts,-p", num_parts,
                        "Number of partitions to create")->required();
        opts.add_option("--out,-o", out_file, "Verilog output")->required();
        add_flag("--brute,-b", "Uses a brute force approach instead of classification");
    }

protected:
    void execute()
    {

        //read AIG to generate hypergraph
        if (!store<aig_ntk>().empty()) {
            auto ntk = *store<aig_ntk>().current();
            spdlog::info("AIG initial size = {}",ntk.num_gates());
            mockturtle::depth_view depth{ntk};
            spdlog::info("AIG initial size = {} and depth = {}", ntk.num_gates(), depth.depth());

            mockturtle::mig_npn_resynthesis resyn_mig;
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;

            oracle::partition_manager<aig_names> partitions_aig(ntk, num_parts);

            std::vector<int> aig_parts1;
            std::vector<int> mig_parts1;

            if (is_set("brute")) {

                for (int i = 0; i < num_parts; i++) {
                    oracle::partition_view<aig_names> part_aig = partitions_aig.create_part(ntk, i);

                    auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>(part_aig,
                                   resyn_aig);
                    mockturtle::depth_view part_aig_depth{opt_aig};
                    spdlog::info("aig part size = {} and depth = {}", opt_aig.num_gates(), part_aig_depth.depth());
                    mockturtle::aig_script aigopt;
                    opt_aig = aigopt.run(opt_aig);
                    mockturtle::depth_view part_aig_opt_depth{opt_aig};
                    int aig_opt_size = opt_aig.num_gates();
                    int aig_opt_depth = part_aig_opt_depth.depth();
                    spdlog::info("optimized aig part size = {} and depth = {}", aig_opt_size, aig_opt_depth);

                    auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>(part_aig,
                                   resyn_mig);
                    mockturtle::depth_view part_mig_depth{opt_mig};
                    spdlog::info("mig part size = {} and depth = {}", opt_mig.num_gates(), part_mig_depth.depth());
                    mockturtle::mig_script migopt;
                    opt_mig = migopt.run(opt_mig);
                    mockturtle::depth_view part_mig_opt_depth{opt_mig};
                    int mig_opt_size = opt_mig.num_gates();
                    int mig_opt_depth = part_mig_opt_depth.depth();
                    spdlog::info("optimized mig part size = {} and depth = {}", mig_opt_size, mig_opt_depth);

                    if ((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)) {
                        spdlog::info("AIG wins");
                        aig_parts1.push_back(i);
                    } else {
                        spdlog::info("MIG wins");
                        mig_parts1.push_back(i);
                    }
                }
            } else {
                if (!nn_model.empty()) {
                    partitions_aig.run_classification(ntk, nn_model);

                    aig_parts1 = partitions_aig.get_aig_parts();
                    mig_parts1 = partitions_aig.get_mig_parts();
                } else {
                    spdlog::error("Must include CNN model json file");
                }
            }
            //Deal with AIG partitions
            spdlog::info("Total number of partitions for AIG 1 {}",aig_parts1.size());
            spdlog::info("Total number of partitions for MIG 1 {}",mig_parts1.size());

            for (int i = 0; i < aig_parts1.size(); i++) {
                oracle::partition_view<aig_names> part_aig = partitions_aig.create_part(ntk,
                        aig_parts1.at(i));

                spdlog::info("\nPartition {}",i);
                mockturtle::depth_view part_depth{part_aig};
                spdlog::info("Partition size = {} and depth = {}", part_aig.num_gates(), part_depth.depth());

                auto aig_opt = mockturtle::node_resynthesis<mockturtle::aig_network>(part_aig,
                               resyn_aig);

                mockturtle::aig_script aigopt;
                auto aig = aigopt.run(aig_opt);

                mockturtle::depth_view part_aig_depth{aig};
                spdlog::info("Post optimization part size = {} and depth = {}", aig.num_gates(), part_aig_depth.depth());

                partitions_aig.synchronize_part(part_aig, aig, ntk);
            }

            partitions_aig.connect_outputs(ntk);
            auto ntk_final = mockturtle::cleanup_dangling(ntk);

            mockturtle::depth_view depth_final{ntk_final};

            spdlog::info("Final AIG size = {} and depth = {}", ntk_final.num_gates(), depth_final.depth());

            oracle::partition_manager<aig_names> tmp(ntk_final, num_parts);

            std::vector<int> aig_parts2;
            std::vector<int> mig_parts2;
            if (is_set("brute")) {

                for (int i = 0; i < num_parts; i++) {
                    oracle::partition_view<aig_names> part_aig = tmp.create_part(ntk_final, i);

                    auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>(part_aig,
                                   resyn_aig);
                    mockturtle::depth_view part_aig_depth{opt_aig};
                    spdlog::info("aig part size = {} and depth = {}", opt_aig.num_gates(), part_aig_depth.depth());
                    mockturtle::aig_script aigopt;
                    opt_aig = aigopt.run(opt_aig);
                    mockturtle::depth_view part_aig_opt_depth{opt_aig};
                    int aig_opt_size = opt_aig.num_gates();
                    int aig_opt_depth = part_aig_opt_depth.depth();
                    spdlog::info("optimized aig part size = {} and depth = {}", aig_opt_size, aig_opt_depth);

                    auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>(part_aig,
                                   resyn_mig);
                    mockturtle::depth_view part_mig_depth{opt_mig};
                    spdlog::info("mig part size = {} and depth = {}", opt_mig.num_gates(), part_mig_depth.depth());
                    mockturtle::mig_script migopt;
                    opt_mig = migopt.run(opt_mig);
                    mockturtle::depth_view part_mig_opt_depth{opt_mig};
                    int mig_opt_size = opt_mig.num_gates();
                    int mig_opt_depth = part_mig_opt_depth.depth();
                    spdlog::info("optimized mig part size = {} and depth = {}", mig_opt_size, mig_opt_depth);

                    if ((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)) {
                        spdlog::info("AIG wins");
                        aig_parts2.push_back(i);
                    } else {
                        spdlog::info("MIG wins");
                        mig_parts2.push_back(i);
                    }
                }

            } else {
                if (!nn_model.empty()) {
                    tmp.run_classification(ntk_final, nn_model);

                    aig_parts2 = tmp.get_aig_parts();
                    mig_parts2 = tmp.get_mig_parts();
                } else {
                    spdlog::error("Must include CNN model json file");
                }

            }

            mockturtle::direct_resynthesis<mockturtle::mig_network> convert_mig;

            auto mig = mockturtle::node_resynthesis<mockturtle::mig_network>(ntk_final,
                       convert_mig);
            spdlog::info("Initial MIG size = {}",mig.num_gates());

            oracle::partition_manager<mig_names> partitions_mig(mig, num_parts);

            //Deal with AIG partitions
            spdlog::info("Total number of partitions for AIG 2 {}",aig_parts2.size());
            spdlog::info("Total number of partitions for MIG 2 {}",mig_parts2.size());
            for (int i = 0; i < mig_parts2.size(); i++) {
                oracle::partition_view<mig_names> part_mig = partitions_mig.create_part(mig,
                        mig_parts2.at(i));

                spdlog::info("\nPartition {}",i);
                mockturtle::depth_view part_depth{part_mig};
                spdlog::info("Partition size = {} and depth = {}", part_mig.num_gates(), part_depth.depth());

                auto mig_opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part_mig,
                               resyn_mig);
                mockturtle::mig_script migopt;

                mig_opt = migopt.run(mig_opt);

                mockturtle::depth_view opt_mig_depth{mig_opt};

                spdlog::info("Post optimization part size = {} and depth = {}", mig_opt.num_gates(), opt_mig_depth.depth()
                          );

                partitions_mig.synchronize_part(part_mig, mig_opt, mig);
            }

            partitions_mig.connect_outputs(mig);
            mig = mockturtle::cleanup_dangling(mig);

            mockturtle::depth_view final_mig{mig};

            spdlog::info("new ntk size = {} and depth = {}", mig.num_gates(), final_mig.depth());
            spdlog::info("Finished optimization");

            mockturtle::write_verilog(mig, out_file);
        } else {
            spdlog::error("There is no stored AIG network");
        }
    }

private:
    int num_parts = 0;
    std::string nn_model{};
    std::string out_file{};
};

ALICE_ADD_COMMAND(mixed_2step, "Optimization");
}
