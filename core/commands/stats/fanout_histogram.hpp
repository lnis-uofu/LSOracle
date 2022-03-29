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
class fanout_histogram_command : public alice::command
{

public:
    explicit fanout_histogram_command(const environment::ptr &env)
        : command(env, "Displays a histogram of fanout of the stored network")
    {
        add_flag("--aig,-a", "Display stats for stored AIG (AIG is default)");
        add_flag("--mig,-m", "Display stats for stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG (AIG is default)");
        add_flag("--xmg,-g", "Display stats for stored XMG (AIG is default)");
    }

    template <typename ntk> void dump_stats(string name)
    {
        if (!store<ntk>().empty()) {
            auto &dag = *store<ntk>().current();
            mockturtle::fanout_view dag_view(dag);
            vector<uint32_t> fanout_histogram(33, 0);

            dag_view.foreach_node([this, &dag_view, &fanout_histogram](auto n) {
                uint32_t fanout = 0;
                dag_view.foreach_fanout(n, [&dag_view, &fanout, n](auto f) {
                    fanout++;
                });
                if (fanout >= fanout_histogram.size() - 1) {
                    fanout_histogram[fanout_histogram.size() - 1]++;
                } else {
                    fanout_histogram[fanout]++;
                }
            });
            env->out() << "Node counts by number of fanouts" << std::endl;;
            env->out() << "Fanout\tNodes" << std::endl;
            for (size_t i = 0; i < fanout_histogram.size() - 1; i++) {
                env->out() << i << "\t" << fanout_histogram[i] << std::endl;
            }
            env->out() << ">=" << (fanout_histogram.size() - 1) << "\t" <<
                       fanout_histogram[fanout_histogram.size() - 1] << std::endl;
        } else {
            env->err() << "There is not an " << name << " network stored.\n";
        }
    }
protected:
    void execute()
    {
        if (is_set("mig")) {
            dump_stats<mig_ntk>("MIG");
        } else if (is_set("xag")) {
            dump_stats<xag_ntk>("XAG");
        } else if (is_set("xmg")) {
            dump_stats<xmg_ntk>("XMG");
        } else {
            dump_stats<aig_ntk>("AIG");
        }
    }
private:
};

ALICE_ADD_COMMAND(fanout_histogram, "Stats");
}
