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
class partition_detail_command : public alice::command
{

public:
    explicit partition_detail_command(const environment::ptr &env)
        : command(env, "Display all nodes in each Partition")
    {

        add_flag("--mig,-m", "Use stored MIG network (Default is AIG)");
        add_flag("--names,-n", "Use stored named networks (Default is Named AIG)");
    }

protected:
    void execute()
    {

        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {
                auto ntk = store<mig_ntk>().current();
                if (!store<part_man_mig_ntk>().empty()) {
                    auto partitions = store<part_man_mig_ntk>().current();
                    int num_part = partitions->get_part_num();
                    double node_num = 0.0;
                    for (int i = 0; i < num_part; i++) {
                        oracle::partition_view<mig_names> part = partitions->create_part(*ntk, i);
                        env->out() << "\n\nPartition " << i << "\n";
                        env->out() << "Number of PI = " << part.num_pis() << "\n";
                        env->out() << "Number of PO = " << part.num_pos() << "\n";
                        env->out() << "Number of latches = " << part.num_latches() << "\n";
                        env->out() << "Number of internal nodes = " << part.num_gates() << "\n";
                        env->out() << "Partition volume = " << double(part.num_gates()) / double(
                                       part.num_pis()) << "\n";
                        node_num += double(part.num_gates()) / double(part.num_pis());
                        // env->out() << "Inputs = {";
                        // part.foreach_pi([&](auto pi){
                        //   env->out() << pi << " ";
                        // });
                        // env->out() << "}\n";
                        // env->out() << "Outputs = {";
                        // part.foreach_po([&](auto conn, auto i){
                        //   env->out() << conn.index << " ";
                        // });
                        // env->out() << "}\n";
                        // env->out() << "Nodes = {";
                        // part.foreach_gate([&](auto node){
                        //   env->out() << node << " ";
                        // });
                        // env->out() << "}\n";
                        std::set<int> connected_parts = partitions->get_connected_parts(*ntk, i);
                        std::set<int>::iterator it;
                        env->out() << "connected partitions = {";
                        for (it = connected_parts.begin(); it != connected_parts.end(); ++it) {
                            env->out() << *it << " ";
                        }
                        env->out() << "}\n";
                    }
                    node_num = node_num / (double)num_part;
                    env->out() << "Average nodes per partition: " << node_num << "\n";
                } else {
                    env->err() << "MIG not partitioned yet\n";
                }
            } else {
                env->err() << "There is no MIG network stored\n";
            }
        } else {
            if (!store<aig_ntk>().empty()) {
                auto ntk = store<aig_ntk>().current();
                if (!store<part_man_aig_ntk>().empty()) {
                    auto partitions = store<part_man_aig_ntk>().current();
                    int num_part = partitions->get_part_num();
                    double node_num = 0.0;
                    for (int i = 0; i < num_part; i++) {
                        oracle::partition_view<aig_names> part = partitions->create_part(*ntk, i);
                        env->out() << "\n\nPartition " << i << "\n";
                        env->out() << "Number of PI = " << part.num_pis() << "\n";
                        env->out() << "Number of PO = " << part.num_pos() << "\n";
                        env->out() << "Number of latches = " << part.num_latches() << "\n";
                        env->out() << "Number of internal nodes = " << part.num_gates() << "\n";

                        env->out() << "Partition volume = " << double(part.num_gates()) / double(
                                       part.num_pis()) << "\n";
                        node_num += double(part.num_gates()) / double(part.num_pis());
                        env->out() << "Inputs = {";
                        part.foreach_pi([&](auto pi) {
                            env->out() << pi << " ";
                        });
                        env->out() << "}\n";
                        env->out() << "Outputs = {";
                        part.foreach_po([&](auto conn, auto i) {
                            env->out() << conn.index << " ";
                        });
                        env->out() << "}\n";
                        env->out() << "Nodes = {";
                        part.foreach_gate([&](auto node) {
                            env->out() << node << " ";
                        });
                        env->out() << "}\n";
                        std::set<int> connected_parts = partitions->get_connected_parts(*ntk, i);
                        std::set<int>::iterator it;
                        env->out() << "connected partitions = {";
                        for (it = connected_parts.begin(); it != connected_parts.end(); ++it) {
                            env->out() << *it << " ";
                        }
                        env->out() << "}\n";
                    }
                    node_num = node_num / (double)num_part;
                    env->out() << "Average partition volume: " << node_num << "\n";
                } else {
                    env->err() << "AIG not partitioned yet\n";
                }
            } else {
                env->err() << "There is no AIG network stored\n";
            }
        }
    }
private:
};

ALICE_ADD_COMMAND(partition_detail, "Partitioning");
}
