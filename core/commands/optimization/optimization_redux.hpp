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
#ifdef ENABLE_OPENSTA
#ifdef ENABLE_ABC

#include <alice/alice.hpp>
#include <mockturtle/mockturtle.hpp>
#include <sys/stat.h>
#include "algorithms/optimization/budget.hpp"

namespace alice
{
class optimization_redux_command : public alice::command
{

public:
    explicit optimization_redux_command(const environment::ptr &env)
        : command(env, "Perform timing driven mixed synthesis.")
    {
        opts.add_option("--abc_exec", abc_exec,
                        "ABC executable, defaults to using path.");
        opts.add_flag("--ndp", "Node Depth Product target");
        opts.add_flag("--nodes", "Node Count target");
        opts.add_flag("--depth", "Depth target");
    }
protected:
    void execute()
    {
        if (store<aig_ntk>().empty()) {
            env->err() << "No AIG stored\n";
            return;
        }
        if (store<part_man_aig_ntk>().empty()) {
            env->err() << "AIG not partitioned yet\n";
            return;
        }
        oracle::optimization_strategy strategy;
        if (is_set("depth")) {
            strategy = oracle::optimization_strategy::depth;
        } else if (is_set("nodes")) {
            strategy = oracle::optimization_strategy::size;
        } else {
            strategy = oracle::optimization_strategy::balanced;
        }
        oracle::optimization_strategy::balanced;
        auto ntk_aig = *store<aig_ntk>().current();
        mockturtle::depth_view orig_depth(ntk_aig);
        auto partitions_aig = *store<part_man_aig_ntk>().current();
        auto start = std::chrono::high_resolution_clock::now();
        mockturtle::names_view<mockturtle::xmg_network> ntk_result =
            oracle::optimization_redux<mockturtle::aig_network>(
                ntk_aig, partitions_aig,
                liberty_file, sdc_file, clock_name,
		output_file, abc_exec, strategy);
        auto stop = std::chrono::high_resolution_clock::now();

        mockturtle::depth_view new_depth(ntk_result);
        if (ntk_result.size() == ntk_aig.size()
                && orig_depth.depth() == new_depth.depth()) {
            env->err() << "No change made to network" << std::endl;
            return;
        }

        env->out() << "Final ntk size = " << ntk_result.num_gates() << " and depth = "
                   << new_depth.depth() << "\n";
        env->out() << "Final number of latches = " << ntk_result.num_latches() << "\n";
        env->out() << "Node Depth Product = "
                   << ntk_result.num_gates() * new_depth.depth()
                   << "\n";
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                        (stop - start);
        env->out() << "Full Optimization: " << duration.count() << "ms\n";
        env->out() << "Finished optimization\n";
        store<xmg_ntk>().extend() = std::make_shared<xmg_names>(ntk_result);
    }
    string liberty_file;
    string output_file;
    string sdc_file;
    string clock_name;
    string abc_exec{"abc"};
};
ALICE_ADD_COMMAND(optimization_redux, "Optimization");
}

#endif
#endif
