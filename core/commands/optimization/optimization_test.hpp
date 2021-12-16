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
// #ifdef ENABLE_OPENSTA
// #ifdef ENABLE_ABC

#include <alice/alice.hpp>
#include <mockturtle/mockturtle.hpp>
#include <sys/stat.h>
//#include "algorithms/optimization/budget.hpp"

namespace alice
{
class optimization_test_command : public alice::command
{

public:
    explicit optimization_test_command(const environment::ptr &env)
        : command(env, "Do a thing.")
    {
        // opts.add_option("--abc_exec", abc_exec,
        //                 "ABC executable, defaults to using path.");
        opts.add_option("--partition,-p", index,
                        "index of partition to test opt.");
        // opts.add_flag("--ndp", "Node Depth Product target");
        // opts.add_flag("--nodes", "Node Count target");
        // opts.add_flag("--depth", "Depth target");
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
        auto ntk_aig = *store<aig_ntk>().current();
        mockturtle::depth_view orig_depth(ntk_aig);
        auto partitions_view = *store<part_man_aig_ntk>().current();
        auto start = std::chrono::high_resolution_clock::now();
        oracle::partition_view<mockturtle::names_view<mockturtle::aig_network>> original = partitions_view.create_part(ntk_aig, index);
        mockturtle::direct_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn;
        mockturtle::names_view<mockturtle::aig_network> copy = mockturtle::node_resynthesis<mockturtle::names_view<mockturtle::aig_network>, oracle::partition_view<mockturtle::names_view<mockturtle::aig_network>>>
                (original, resyn);

        auto stop = std::chrono::high_resolution_clock::now();
        env->out() << "Finished optimization\n";
    }
    string liberty_file;
    string output_file;
    string sdc_file;
    string clock_name;
    int index;
    string abc_exec{"abc"};
};
ALICE_ADD_COMMAND(optimization_test, "Optimization test");
}

// #endif
// #endif
