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
#include <sys/stat.h>
#include "algorithms/optimization/exploder.hpp"

namespace alice
{
class exploderize_command : public alice::command
{

public:
    explicit exploderize_command(const environment::ptr &env)
        : command(env, "Perform exploded logic cone optimization.")
    {
        opts.add_option("--abc_exec", abc_exec,
                        "ABC executable, defaults to using path.");
        opts.add_option("--depth", target,
                        "Target depth to apply optimization.");
    }
protected:
    void execute()
    {
        synth<mockturtle::aig_network>("AIG");
    }

    template <typename network>
    void synth(string name)
    {
        if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
            env->err() << "No " << name << " stored\n";
            return;
        }
        mockturtle::names_view<network> ntk =
            *store<std::shared_ptr<mockturtle::names_view<network>>>().current();

        mockturtle::depth_view orig_depth(ntk);
        std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        oracle::exploderizer<network> boom(ntk, abc_exec);
        mockturtle::names_view<mockturtle::xmg_network> ntk_result = boom.exploderize(target);
        std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
        auto stop = std::chrono::high_resolution_clock::now();

        mockturtle::depth_view new_depth(ntk_result);
        env->out() << "Original ntk size = " << ntk.size() << " depth = " << orig_depth.depth() << std::endl;
        if (ntk_result.size() == ntk.size()
                && orig_depth.depth() == new_depth.depth()) {
            env->err() << "No change made to network" << std::endl;
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
        store<std::shared_ptr<mockturtle::names_view<mockturtle::xmg_network>>>().extend() =
			      std::make_shared<mockturtle::names_view<mockturtle::xmg_network>>(ntk_result);
    }

    string abc_exec{"abc"};
    uint32_t target = 0;
};
ALICE_ADD_COMMAND(exploderize, "Optimization");
}
