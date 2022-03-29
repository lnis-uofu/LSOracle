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

#include "utility.hpp"

namespace alice
{
class function_stats_command : public alice::command
{
public:
    explicit function_stats_command(const environment::ptr &env)
        : command(env,
                  "Show counts of the effective gate function used in the stored network")
    {
        add_flag("--mig,-m", "Display stats for stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG (AIG is default)");
        add_flag("--xmg,-g", "Display stats for stored XMG (AIG is default)");
        add_flag("--aig,-a", "Display stats for stored AIG (AIG is default)");
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

	oracle::function_counts counts = oracle::node_functions(ntk);
        env->out() << "MAJ nodes internally = " << counts.maj_num << "\n";
        env->out() << "AND nodes internally = " << counts.and_num << "\n";
        env->out() << "OR nodes internally = " << counts.or_num << "\n";
        env->out() << "XOR3 nodes internally = " << counts.xor3_num << "\n";
        env->out() << "XOR nodes internally = " << counts.xor_num << "\n";
        env->out() << "XNOR nodes internally = " << counts.xnor_num << "\n";
        env->out() << "Unknown nodes internally = " << counts.unknown_num << "\n";
        env->out() << "Input nodes internally = " << counts.input_num << "\n";
    }
    void execute()
    {
        if (is_set("xag")) {
            dump_stats<mockturtle::xag_network>("XAG");
        } else if (is_set("xmg")) {
            dump_stats<mockturtle::xmg_network>("XMG");
        } else if (is_set("mig")) {
            dump_stats<mockturtle::mig_network>("MIG");
        } else {
            dump_stats<mockturtle::aig_network>("AIG");
        }

    }
private:
};

ALICE_ADD_COMMAND(function_stats, "Stats");
}
