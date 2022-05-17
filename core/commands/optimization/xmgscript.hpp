/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2021 Laboratory for Nano Integrated Systems (LNIS)
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
class xmgscript_command : public alice::command
{

public:
    explicit xmgscript_command(const environment::ptr &env)
        : command(env, "Perform XMG based optimization")
    {

        //opts.add_option("--strategy", strategy, "Optimization strategy [0-4]");
    }

protected:
    void execute()
    {

        if (!store<xmg_ntk>().empty()) {
            auto &opt = *store<xmg_ntk>().current();
            mockturtle::depth_view<mockturtle::names_view<mockturtle::xmg_network>> xmg_depth(opt);
            spdlog::info("XMG logic depth {} nodes {}", xmg_depth.depth(),
                       xmg_depth.num_gates());
            auto start = std::chrono::high_resolution_clock::now();
            oracle::xmg_script xmgopt;
            opt = xmgopt.run(opt);
            auto stop = std::chrono::high_resolution_clock::now();

            mockturtle::depth_view<mockturtle::names_view<mockturtle::xmg_network>> new_xmg_depth(opt);
            spdlog::info("Final ntk size = {} and depth = {}", new_xmg_depth.num_gates(), new_xmg_depth.depth());
            spdlog::info("NDP = {}",opt.num_gates() * new_xmg_depth.depth());

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                            (stop - start);
            spdlog::info("Full Optimization: {}ms", duration.count());
            spdlog::info("Finished optimization");
        } else {
            spdlog::error("There is not an XMG network stored.");
        }
    }
}; //class
ALICE_ADD_COMMAND(xmgscript, "Optimization");
} //namespace alice
