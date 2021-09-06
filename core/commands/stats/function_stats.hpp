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
class function_stats_command : public alice::command
{

public:
    explicit function_stats_command(const environment::ptr &env)
        : command(env,
                  "Show counts of the effective gate function used in the stored network")
    {
        add_flag("--mig,-m", "Display stats for stored MIG (MIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG (MIG is default)");
        add_flag("--xmg,-n", "Display stats for stored XMG (MIG is default)");
    }

protected:
    template <typename network> void dump_stats(string name)
    {
        if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
            env->err() << name << " network not stored\n";
            return;
        }
        auto ntk =
            *store<std::shared_ptr<mockturtle::names_view<network>>>().current();

        int maj_num = 0;
        int xor_num = 0;
        int xnor_num = 0;
        int xor3_num = 0;
        int and_num = 0;
        int or_num = 0;
        int input_num = 0;

        ntk.foreach_node([&](auto node) {
            if (ntk.is_pi(node)) {
                input_num++;
            } else if (ntk.is_maj(node)) {
                typename network::signal first_signal = ntk._storage->nodes[node].children[0];
                typename network::node first_fanin = ntk.get_node(first_signal);

                if (ntk.is_constant(first_fanin)) {
                    if (first_signal.complement) {
                        or_num++;
                    } else {
                        and_num++;
                    }
                } else {
                    maj_num++;
                }
            } else if (ntk.is_xor3(node)) {
                typename network::signal first_signal = ntk._storage->nodes[node].children[0];
                typename network::node first_fanin = ntk.get_node(first_signal);
                if (ntk.is_constant(first_fanin)) {
                    if (first_signal.complement) {
                        xnor_num++;
                    } else {
                        xor_num++;
                    }
                } else {
                    xor3_num++;
                }
            } else {
                env->err() << "No idea what this node is.";
            }
        });
        env->out() << "MAJ nodes internally = " << maj_num << "\n";
        env->out() << "AND nodes internally = " << and_num << "\n";
        env->out() << "OR nodes internally = " << or_num << "\n";
        env->out() << "XOR3 nodes internally = " << xor3_num << "\n";
        env->out() << "XOR nodes internally = " << xor_num << "\n";
        env->out() << "XNOR nodes internally = " << xnor_num << "\n";
    }
    void execute()
    {
        if (is_set("xag")) {
            dump_stats<mockturtle::xag_network>("XAG");
        } else if (is_set("xmg")) {
            dump_stats<mockturtle::xmg_network>("XMG");
        } else {
            dump_stats<mockturtle::mig_network>("MIG");
        }

    }
private:
};

ALICE_ADD_COMMAND(function_stats, "Stats");
}
