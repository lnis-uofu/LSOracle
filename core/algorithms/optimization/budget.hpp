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
#include <mockturtle/mockturtle.hpp>
#include "algorithms/partitioning/partition_manager.hpp"
#include "utility.hpp"

namespace oracle
{

template <typename network>
mockturtle::names_view<network> budget_optimization(
    mockturtle::names_view<network> &ntk,
    oracle::partition_manager<mockturtle::names_view<network>> &partitions,
    const std::string &liberty_file, const std::string &output_file, const std::string &abc_exec);

struct node_depth {
    int nodes;
    int depth;
};

enum optimization_strategy { area, balanced, depth };

template<typename network>
class optimizer
{
public:
    // TODO don't keep the part_view
    virtual partition_view<mockturtle::names_view<network>> partition() = 0;
    /**
     * module name for the generated verilog.
     */
    virtual std::string module_name() = 0;
    /**
     * human readable name
     */
    virtual std::string optimizer_name() = 0;
    /**
     * Do any conversion necessary from original network type to the internal network type.
     */
    virtual void convert() = 0;
    /**
     * Perform optimization
     */
    virtual void optimize() = 0;
    /**
     * Calculate tech independent depth and nodes metrics.
     */
    virtual node_depth independent_metric() = 0;
    /**
     * Convert the optimized network back to the original network type.
     */
    virtual mockturtle::names_view<network> reconvert() = 0;
    /**
     * List the type of optimization: area, delay, or balanced.
     */
    virtual optimization_strategy target() = 0;
    /**
     * Techmap, then return a path to a file containing the techmapped verilog.
     */
    virtual std::string techmap(std::string liberty_file) = 0;
};
}

#endif
#endif
