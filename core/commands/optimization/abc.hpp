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
#include "algorithms/optimization/abc.hpp"

namespace alice
{
class abc_part_command : public alice::command
{

public:
    explicit abc_part_command(const environment::ptr &env)
        : command(env, "Perform timing driven mixed synthesis.")
    {
        opts.add_option("--abc_exec", abc_exec,
                        "ABC executable, defaults to using path.");
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
        if (store<std::shared_ptr<oracle::partition_manager_junior<network>>>().empty()) {
            env->err() << name << " not partitioned yet\n";
            return;
        }

        oracle::partition_manager_junior<network> partitions_jr =
            *store<std::shared_ptr<oracle::partition_manager_junior<network>>>().current();

        mockturtle::depth_view orig_depth(partitions_jr.get_network());

        auto start = std::chrono::high_resolution_clock::now();
	    oracle::optimize_abc(partitions_jr, abc_exec);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                        (stop - start);
        env->out() << "Full Optimization: " << duration.count() << "ms\n";
        env->out() << "Finished optimization\n";
    }

    string abc_exec{"abc"};
};
ALICE_ADD_COMMAND(abc_part, "Optimization");
}
