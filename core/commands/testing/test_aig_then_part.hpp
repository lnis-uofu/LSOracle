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

class test_aig_then_part_command : public alice::command
{

public:
    explicit test_aig_then_part_command(const environment::ptr &env)
        : command(env,
                  "Runs AIG optimization on entire circuit before partitioning and determining best MIG partitions")
    {

        opts.add_option("--num_parts,-p", num_parts,
                        "Number of partitions to create")->required();
        opts.add_option("--cnn_model,-c", cnn_model,
                        "Trained CNN model for classification");
        opts.add_option("--out,-o", out_file, "Verilog output")->required();
        add_flag("--brute,-b", "Uses a brute force approach instead of classification");
    }

protected:
    void execute()
    {

        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;

        std::vector<int> mig_parts;
        if (!store<mockturtle::aig_network>().empty()) {

            auto ntk_aig = store<mockturtle::aig_network>().current();
            std::string file_base = ntk_aig._storage->net_name;
            mockturtle::depth_view orig_ntk_depth{ntk_aig};
            // spdlog::info("ntk_aig size = {} and depth = {}", ntk_aig.num_gates(), orig_ntk_depth.depth());
            std::string net_name = ntk_aig._storage->net_name;

            mockturtle::aig_script aigopt;
            ntk_aig = aigopt.run(ntk_aig);
            mockturtle::depth_view aig_opt_depth{ntk_aig};
            // spdlog::info("optimized aig network size = {} and depth = {}", ntk_aig.num_gates(), aig_opt_depth.depth());

            oracle::partition_manager<mockturtle::aig_network> partitions_aig(ntk_aig,
                    num_parts);
            if (is_set("brute")) {

                for (int i = 0; i < num_parts; i++) {
                    oracle::partition_view<mockturtle::aig_network> part_aig =
                        partitions_aig.create_part(ntk_aig, i);

                    auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>(part_aig,
                                   resyn_aig);
                    mockturtle::depth_view part_aig_depth{opt_aig};
                    // spdlog::info("aig part size = {} and depth = {}", opt_aig.num_gates(), part_aig_depth.depth());
                    mockturtle::aig_script aigopt;
                    opt_aig = aigopt.run(opt_aig);
                    mockturtle::depth_view part_aig_opt_depth{opt_aig};
                    int aig_opt_size = opt_aig.num_gates();
                    int aig_opt_depth = part_aig_opt_depth.depth();
                    // spdlog::info("optimized aig part size = {} and depth = {}", aig_opt_size, aig_opt_depth);

                    auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>(part_aig,
                                   resyn_mig);
                    mockturtle::depth_view part_mig_depth{opt_mig};
                    // spdlog::info("mig part size = {} and depth = {}", opt_mig.num_gates(), part_mig_depth.depth());
                    mockturtle::mig_script migopt;
                    opt_mig = migopt.run(opt_mig);
                    mockturtle::depth_view part_mig_opt_depth{opt_mig};
                    int mig_opt_size = opt_mig.num_gates();
                    int mig_opt_depth = part_mig_opt_depth.depth();
                    // spdlog::info("optimized mig part size = {} and depth = {}", mig_opt_size, mig_opt_depth);

                    if ((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)) {
                        // spdlog::info("AIG wins");
                    } else {
                        // spdlog::info("MIG wins");
                        mig_parts.push_back(i);
                    }
                }

            } else {
                if (!cnn_model.empty()) {
                    partitions_aig.run_classification(ntk_aig, cnn_model);

                    mig_parts = partitions_aig.get_mig_parts();
                } else {
                    spdlog::error("Must include CNN model json file");
                }

            }

            mockturtle::mig_network ntk_mig = aig_to_mig(ntk_aig,
                                              0); //mockturtle::node_resynthesis<mockturtle::mig_network>( ntk_aig, resyn_mig );
            oracle::partition_manager<mockturtle::mig_network> partitions_mig(ntk_mig,
                    partitions_aig.get_all_part_connections(),
                    partitions_aig.get_all_partition_inputs(),
                    partitions_aig.get_all_partition_outputs(), partitions_aig.get_part_num());

            // spdlog::info("Scheduled optimization:");
            // spdlog::info("MIG Optimization = {";
            // for(int i = 0; i < mig_parts.size(); i++){
            //     spdlog::info(mig_parts.at(i) << " ";
            // }
            // spdlog::info("}");

            for (int i = 0; i < mig_parts.size(); i++) {
                // spdlog::info("Optimize partition " << mig_parts.at(i));
                oracle::partition_view<mockturtle::mig_network> part =
                    partitions_mig.create_part(ntk_mig, mig_parts.at(i));
                mockturtle::depth_view part_depth{part};
                // spdlog::info("part size = {} and depth = {}", part.num_gates(), part_depth.depth());

                auto opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part,
                           resyn_mig);
                // spdlog::info("MIG before optimization");
                // opt.foreach_node([&](auto node){
                //     spdlog::info("node = " << node);
                //     opt.foreach_fanin(node, [&](auto conn, auto i){
                //         spdlog::info("child[" << i << "] = " << conn.index);
                //     });
                // });
                mockturtle::mig_script migopt;
                opt = migopt.run(opt);
                // spdlog::info("MIG after optimization");
                // opt.foreach_node([&](auto node){
                //     spdlog::info("node = " << node);
                //     opt.foreach_fanin(node, [&](auto conn, auto i){
                //         spdlog::info("child[" << i << "] = " << conn.index);
                //     });
                // });
                mockturtle::depth_view part_opt_depth{opt};
                // spdlog::info("new part size = {} and depth = {}", opt.num_gates(), part_opt_depth.depth());

                partitions_mig.synchronize_part(part, opt, ntk_mig);
            }

            stringstream ss = "MIG partitions = {";
            for (int i = 0; i < mig_parts.size(); i++) {
                ss << mig_parts.at(i) << " ";
            }
            ss << "}";
            spdlog::info(ss.str());

            partitions_mig.connect_outputs(ntk_mig);

            ntk_mig = mockturtle::cleanup_dangling(ntk_mig);
            mockturtle::depth_view ntk_depth2{ntk_mig};
            spdlog::info("new ntk size = {} and depth = {}", ntk_mig.num_gates(),
                       ntk_depth2.depth());
            spdlog::info("Finished optimization");

            mockturtle::write_verilog(ntk_mig, out_file);


        } else {
            spdlog::error("No AIG stored");
        }
    }

private:
    int num_parts = 0;
    std::string cnn_model{};
    std::string out_file{};
};

ALICE_ADD_COMMAND(test_aig_then_part, "Testing");
}
