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
ALICE_COMMAND(depthr, "Optimization", "Logic depth oriented MIG rewriting")
{
    if (!store<mig_ntk>().empty()) {
        auto &mig = *store<mig_ntk>().current();
        spdlog::info("Mig gates {}", mig.num_gates());

        //to compute at level
        mockturtle::depth_view mig_depth{mig};

        spdlog::info("Mig level {} mig gates {}", mig_depth.depth(), mig.num_gates());

        mockturtle::mig_algebraic_depth_rewriting_params pm;
        mockturtle::mig_algebraic_depth_rewriting(mig_depth, pm);

        mig = mockturtle::cleanup_dangling(mig);
        spdlog::info("Mig level after algebraic rewriting {} Mig gates {}", mig_depth.depth(), mig.num_gates());
    } else {
        spdlog::error("There is not an MIG network stored.");
    }
}
}
