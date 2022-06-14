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
#include "algorithms/optimization/resynthesis.hpp"



namespace alice
{
class partition_detail_command : public alice::command
{

public:
    explicit partition_detail_command(const environment::ptr &env)
        : command(env, "Display all nodes in each Partition")
    {
        add_flag("--aig,-a", "Partition stored AIG (Default)");
        add_flag("--mig,-m", "Partition stored MIG");
        add_flag("--xag,-x", "Partition stored XAG");
        add_flag("--xmg,-g", "Partition stored XMG");
    }

protected:
    template <typename network>
    void partition_det(string name){
        if (!store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
            if (!store<std::shared_ptr<oracle::partition_manager_junior<network>>>().empty()) {
                mockturtle::names_view<network> ntk = *store<std::shared_ptr<mockturtle::names_view<network>>>().current();
                if (!store<std::shared_ptr<oracle::partition_manager_junior<network>>>().empty()) {
                    oracle::partition_manager_junior<network> partitions_jr =*store<std::shared_ptr<oracle::partition_manager_junior<network>>>().current();
                    mockturtle::depth_view orig_depth(partitions_jr.get_network());
                    int num_parts = partitions_jr.count();
                    double node_num = 0.0;
                    for (int i =0; i<num_parts;i++){
                        env->err() << "partitions_jr.get_network().size() \n" <<partitions_jr.get_network().size();
                        
                        env->out() << "\n\nPartition " << "\n";
                        env->out() << "Number of PI = " << partitions_jr.get_network().num_pis() << "\n";
                        env->out() << "Number of PO = " << partitions_jr.get_network().num_pos() << "\n";
                        env->out() << "Number of latches = " << partitions_jr.get_network().num_latches() << "\n";
                        env->out() << "Number of internal nodes = " << partitions_jr.get_network().num_gates() << "\n";
                        env->out() << "Partition volume = " << double(partitions_jr.get_network().num_gates()) / double(
                                            partitions_jr.get_network().num_pis()) << "\n";
                        node_num += double(partitions_jr.get_network().num_gates()) / double(partitions_jr.get_network().num_pis());
                        env->out() << "Inputs = {";
                        partitions_jr.get_network().foreach_pi([&](auto pi) {
                            env->out() << pi << " ";
                        });
                        env->out() << "}\n";
                        env->out() << "Outputs = {";
                        
                        partitions_jr.get_network().foreach_po([&](auto conn, auto i) {
                            env->out() << conn.index << " ";
                        });
                        env->out() << "}\n";
                        env->out() << "Nodes = {";
                        partitions_jr.get_network().foreach_gate([&](auto node) {
                            env->out() << node << " ";
                        });
                        env->out() << "}\n";
                    }
                    node_num = node_num / (double)num_parts;
                    env->out() << "Average nodes per partition: " << node_num << "\n";

                } else {
                    env->err() << name <<" not partitioned yet\n";
                }
            }
        } else {
            env->err() << "There is no "<<name<<" network stored\n";
        }
    

    }
    void execute()
    {   
        if (is_set("mig")) {
            partition_det<mockturtle::mig_network>("MIG");
        } else if (is_set("xag")) {
            partition_det<mockturtle::xag_network>("XAG");
        } else if (is_set("xmg")) {
            partition_det<mockturtle::xmg_network>("XMG");
        } else {
            partition_det<mockturtle::aig_network>("AIG");
        }
    }
private:
};

ALICE_ADD_COMMAND(partition_detail, "Partitioning");
}
