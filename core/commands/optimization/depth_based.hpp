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

namespace alice
{
class depth_partition_command : public alice::command
{

public:
    explicit depth_partition_command(const environment::ptr &env)
        : command(env,
                  "Optimize partitions with MIGs or AIGs based on their logic depth.")
    {
        opts.add_option("--num_parts,p", num_parts,
                        "Number of partitions to create")->required();
        opts.add_option("--out,-o", out_file, "Verilog output")->required();
    }

protected:
    void execute()
    {

        //read AIG to generate hypergraph
        if (!store<mockturtle::aig_network>().empty()) {
            auto &ntk = store<mockturtle::aig_network>().current();

            mockturtle::depth_view depth{ntk};
            spdlog::info("Ntk size = {} and depth = {}", ntk.num_gates(), depth.depth());

            oracle::partition_manager<mockturtle::aig_network> partitions_aig(ntk,
                    num_parts);

            std::vector<int> aig_based;
            std::vector<int> mig_based;

            for (int i = 0; i < num_parts; i++) {
                //create partitions
                oracle::partition_view<mockturtle::aig_network> part_aig =
                    partitions_aig.create_part(ntk, i);
                mockturtle::depth_view part_depth{part_aig};
                spdlog::info("Part size = {} and depth = {}", part_aig.num_gates(), part_depth.depth());

                //evaluate partition optimizer based on partitions depth (considering 30% of total depth.. could be configurable as parameter
                if (part_depth.depth() > depth.depth() * 0.3)
                    mig_based.push_back(i);
                else
                    aig_based.push_back(i);
            }

            //Deal with AIG partitions
            for (int i = 0; i < aig_based.size(); i++) {

                spdlog::info("Assigning partition {} to AIG optimizer ", aig_based.at(i));
                oracle::partition_view<mockturtle::aig_network> part_aig =
                    partitions_aig.create_part(ntk, aig_based.at(i));

                spdlog::info("\nPartition {}", i);
                mockturtle::depth_view part_depth{part_aig};
                spdlog::info("part size = {} and depth = {}", part_aig.num_gates(), part_depth.depth());

                mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
                auto aig_opt = mockturtle::node_resynthesis<mockturtle::aig_network>(part_aig,
                               resyn_aig);
                mockturtle::aig_script aigopt;
                aig_opt = aigopt.run(aig_opt);

                mockturtle::depth_view part_aig_depth{aig_opt};
                spdlog::info("new part size = {} and depth = {}", aig_opt.num_gates(), part_aig_depth.depth());
                partitions_aig.synchronize_part(part_aig, aig_opt, ntk);
            }

            mockturtle::mig_npn_resynthesis resyn_mig;
            auto ntk_mig = mockturtle::node_resynthesis<mockturtle::mig_network>(ntk,
                           resyn_mig);
            spdlog::info("MIG created from AIG. There are {} latches on the new network ", ntk_mig.num_latches());
            spdlog::info("There are {} latches on the source", ntk.num_latches());

            oracle::partition_manager<mockturtle::mig_network> partitions_mig(ntk_mig,
                    partitions_aig.get_all_part_connections(),
                    partitions_aig.get_all_partition_inputs(),
                    partitions_aig.get_all_partition_outputs(),
                    partitions_aig.get_all_partition_regs(),
                    partitions_aig.get_all_partition_regin(), partitions_aig.get_part_num());

            partitions_aig.connect_outputs(ntk);

            // Deal with MIG partitions
            for (int i = 0; i < mig_based.size(); i++) {

                spdlog::info("Assigning partition {} to MIG optimizer", mig_based.at(i));
                oracle::partition_view<mockturtle::mig_network> part_mig =
                    partitions_mig.create_part(ntk_mig, mig_based.at(i));
                mockturtle::depth_view part_mig_depth{part_mig};
                spdlog::info("part size = {} and depth = {}", part_mig.num_gates(), part_mig_depth.depth());

                auto mig_opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part_mig,
                               resyn_mig);
                mockturtle::mig_script migopt;
                mig_opt = migopt.run(mig_opt);

                mockturtle::depth_view opt_mig_depth{mig_opt};
                spdlog::info("new part size = {} and depth = {}", mig_opt.num_gates(), opt_mig_depth.depth());
                spdlog::info("Synchronizing ");
                partitions_mig.synchronize_part(part_mig, mig_opt, ntk_mig);
            }
            partitions_mig.connect_outputs(ntk_mig);

            ntk_mig = mockturtle::cleanup_dangling(ntk_mig);
            mockturtle::depth_view ntk_depth2{ntk_mig};

            spdlog::info("new ntk size = {} and depth = {}", ntk_mig.num_gates(), ntk_depth2.depth());
            spdlog::info("new ntk number of latches = {}", ntk_mig.num_latches());

            mockturtle::write_verilog(ntk_mig, out_file);
        } else {
            spdlog::error("There is no stored AIG network");
        }
    }

private:
    int num_parts = 0;
    std::string out_file{};
};

ALICE_ADD_COMMAND(depth_partition, "Optimization");

}
