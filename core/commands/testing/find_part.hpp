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
class find_part_command : public alice::command
{

public:
    explicit find_part_command(const environment::ptr &env)
        : command(env, "Classify partitions and perform corresponding optimization")
    {

        opts.add_option("--node,-n", nodeIdx, "Node to find partition of")->required();
        add_flag("--mig,-m", "Look at stored MIG");
    }

protected:
    void execute()
    {
        int partition = 0;

        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {

                auto ntk = *store<mig_ntk>().current();

                if (!store<part_man_mig_ntk>().empty()) {

                    auto partitions = *store<part_man_mig_ntk>().current();

                    mockturtle::mig_network::node node_to_find = ntk.node_to_index(nodeIdx);
                    int num_partitions = partitions.get_part_num();
                    for (int i = 0; i < num_partitions; i++) {
                        oracle::partition_view<mig_names> part = partitions.create_part(ntk, i);
                        auto nodes = part.get_node_list();
                        if (std::find(nodes.begin(), nodes.end(), node_to_find) != nodes.end()) {
                            env->out() << "Found in partition " << i << " ";
                            if (part.is_pi(node_to_find)) {
                                env->out() << "Input\n";
                            } else if (is_po(part, node_to_find)) {
                                env->out() << "Output\n";
                            } else if (part.is_pi(node_to_find) && is_po(part, node_to_find)) {
                                env->out() << "Input and Output\n";
                            } else {
                                env->out() << "\n";
                            }
                        }
                    }

                } else {
                    env->err() << "MIG not partitioned yet\n";
                }
            } else {
                env->err() << "No MIG stored\n";
            }
        } else {
            if (!store<aig_ntk>().empty()) {

                auto ntk = *store<aig_ntk>().current();

                if (!store<part_man_aig_ntk>().empty()) {

                    auto partitions = *store<part_man_aig_ntk>().current();

                    mockturtle::aig_network::node node_to_find = ntk.node_to_index(nodeIdx);
                    int num_partitions = partitions.get_part_num();
                    for (int i = 0; i < num_partitions; i++) {
                        oracle::partition_view<aig_names> part = partitions.create_part(ntk, i);
                        auto nodes = part.get_node_list();
                        if (std::find(nodes.begin(), nodes.end(), node_to_find) != nodes.end()) {
                            env->out() << "Found in partition " << i << " ";
                            if (part.is_pi(node_to_find)) {
                                env->out() << "Input\n";
                            } else if (is_po(part, node_to_find)) {
                                env->out() << "Output\n";
                            } else if (part.is_pi(node_to_find) && is_po(part, node_to_find)) {
                                env->out() << "Input and Output\n";
                            } else {
                                env->out() << "\n";
                            }
                        }
                    }

                } else {
                    env->err() << "AIG not partitioned yet\n";
                }
            } else {
                env->err() << "No AIG stored\n";
            }
        }

    }
private:
    int nodeIdx = 0;
};

ALICE_ADD_COMMAND(find_part, "Testing");
}
