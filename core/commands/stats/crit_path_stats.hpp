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
class crit_path_stats_command : public alice::command
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

    explicit crit_path_stats_command(const environment::ptr &env)
        : command(env,
                  "Determines the number of Majority nodes and AND nodes along the critical path in MIG.\nDEPRECATED: please use critical_path command.")
    {
        add_flag("--mig,-m", "Display stats for stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG (AIG is default)");
        add_flag("--xmg,-g", "Display stats for stored XMG (AIG is default)");
        add_flag("--aig,-a", "Display stats for stored AIG (AIG is default)");
    }

protected:
    void execute()
    {

        if (!store<mig_ntk>().empty()) {
            auto ntk = *store<mig_ntk>().current();

            oracle::slack_view<mig_names> slack{ntk};
            auto critical_path = slack.get_critical_path(ntk);
            int maj_num = 0;
            int and_num = 0;
            int input_num = 0;

            env->out() << "Critical path size = " << critical_path.size() << "\n";
            for (auto curr_node : critical_path) {
                if (ntk.is_pi(curr_node)) {
                    input_num++;
                } else if (ntk.is_constant(ntk.get_node(
                                               ntk._storage->nodes[curr_node].children[0]))) {
                    and_num++;
                } else {
                    maj_num++;
                }
                // env->out() << "Node = " << curr_node << "\n";
                // ntk.foreach_fanin(curr_node, [&](auto child, auto i){
                //   env->out() << "child[" << i << "] = " << child.index << "\n";
                // });
            }

            env->out() << "MAJ nodes on critical path = " << maj_num << "\n";
            env->out() << "AND nodes on critical path = " << and_num << "\n";
            env->out() << "INPUTS on critical path = " << input_num << "\n";

        } else {
            env->err() << "MIG network not stored\n";
        }

    }
private:
};

ALICE_ADD_COMMAND(crit_path_stats, "Stats");
}
