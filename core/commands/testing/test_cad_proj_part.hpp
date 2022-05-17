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

class create_level_part_command : public alice::command
{

public:
    explicit create_level_part_command(const environment::ptr &env)
        : command(env, "Test Partition technique proposed in '04 ICCAD Paper")
    {

        add_flag("--mig,-m", "Use stored MIG (AIG network is default)");
    }

protected:
    void execute()
    {

        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        if (is_set("mig")) {
            if (!store<mockturtle::mig_network>().empty()) {
                auto ntk = store<mockturtle::mig_network>().current();
                oracle::level_partition_manager<mockturtle::mig_network> level_parts(ntk);
                store<oracle::level_partition_manager<mockturtle::mig_network>>().extend() =
                            level_parts;
            } else {
                spdlog::error("MIG network not stored");
            }
        } else {
            if (!store<mockturtle::aig_network>().empty()) {
                auto ntk = store<mockturtle::aig_network>().current();
                oracle::level_partition_manager<mockturtle::aig_network> level_parts(ntk);
                store<oracle::level_partition_manager<mockturtle::aig_network>>().extend() =
                            level_parts;
            } else {
                spdlog::error("AIG network not stored");
            }
        }
    }

private:

};

ALICE_ADD_COMMAND(create_level_part, "CAD_proj");

class gen_part_man_command : public alice::command
{

public:
    explicit gen_part_man_command(const environment::ptr &env)
        : command(env, "Test Partition technique proposed in '04 ICCAD Paper")
    {

        opts.add_option("--coarse,coarse", coarse,
                        "Number of level partitions to combine into partition for optimization")->required();
        add_flag("--mig,-m", "Use cad_proj with type MIG (AIG network is default)");
    }

protected:
    void execute()
    {

        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        if (is_set("mig")) {
            if (!store<mockturtle::mig_network>().empty()) {
                auto ntk = store<mockturtle::mig_network>().current();
                if (!store<oracle::level_partition_manager<mockturtle::mig_network>>().empty()) {
                    auto level_parts =
                        store<oracle::level_partition_manager<mockturtle::mig_network>>().current();
                    oracle::partition_manager<mockturtle::mig_network> partitions =
                        level_parts.generate_partitions(ntk, coarse);

                    store<oracle::partition_manager<mockturtle::mig_network>>().extend() =
                                partitions;

                    int num_partitions = partitions.get_part_num();
                    for (int i = 0; i < num_partitions; i++) {
                        spdlog::info("Partition " << i);
                        oracle::partition_view<mockturtle::mig_network> part = partitions.create_part(
                                    ntk, i);

                        auto opt = mockturtle::node_resynthesis<mockturtle::mig_network>(part,
                                   resyn_mig);
                        spdlog::info("size = " << opt.size());
                        std::set<mockturtle::mig_network::node> inputs = partitions.get_part_inputs(i);
                        typename std::set<mockturtle::mig_network::node>::iterator it;
                        stringstream ss;
                        ss << "Inputs = ";
                        for (it = inputs.begin(); it != inputs.end(); ++it) {
                            ss << *it << " ";
                        }
                        spdlog::info(ss.str());

                        std::set<mockturtle::mig_network::node> outputs = partitions.get_part_outputs(i);
                        stringstream ss;
                        ss << "Outputs = ";
                        for (it = outputs.begin(); it != outputs.end(); ++it) {
                            ss << *it << " ";
                        }
                        spdlog.info(ss.str());
                    }
                } else {
                    spdlog::error("Level parts not created for MIG");
                }
            } else {
                spdlog::error("MIG network not stored");
            }
        } else {
            if (!store<mockturtle::aig_network>().empty()) {
                auto ntk = store<mockturtle::aig_network>().current();
                if (!store<oracle::level_partition_manager<mockturtle::aig_network>>().empty()) {
                    auto level_parts =
                        store<oracle::level_partition_manager<mockturtle::aig_network>>().current();
                    oracle::partition_manager<mockturtle::aig_network> partitions =
                        level_parts.generate_partitions(ntk, coarse);

                    store<oracle::partition_manager<mockturtle::aig_network>>().extend() =
                                partitions;

                    // int num_partitions = partitions.get_part_num();
                    // for(int i = 0; i < num_partitions; i++){
                    //   // spdlog::info("Partition " << i);
                    //   oracle::partition_view<mockturtle::aig_network> part = partitions.create_part(ntk, i);

                    //   auto opt = mockturtle::node_resynthesis<mockturtle::aig_network>( part, resyn_aig );
                    //   // spdlog::info("size = " << opt.size());
                    //   std::set<mockturtle::aig_network::node> inputs = partitions.get_part_inputs(i);
                    //   typename std::set<mockturtle::aig_network::node>::iterator it;
                    //   spdlog::info("Inputs = ";
                    //   for(it = inputs.begin(); it != inputs.end(); ++it){
                    //     spdlog::info(*it << " ";
                    //   }
                    //   spdlog::info("");

                    //   std::set<mockturtle::aig_network::node> outputs = partitions.get_part_outputs(i);
                    //   spdlog::info("Outputs = ";
                    //   for(it = outputs.begin(); it != outputs.end(); ++it){
                    //     spdlog::info(*it << " ";
                    //   }
                    //   spdlog::info("");
                    // }
                } else {
                    spdlog::error("Level parts not created for AIG");
                }
            } else {
                spdlog::error("AIG network not stored");
            }
        }
    }

private:
    int coarse{};
};

ALICE_ADD_COMMAND(gen_part_man, "CAD_proj");
}
