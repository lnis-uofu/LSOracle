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
#include <string>
#include <mockturtle/mockturtle.hpp>
// #include "algorithms/optimization/optimizers/strategy.hpp"
// #include "algorithms/optimization/optimizers/aig.hpp"
// #include "algorithms/optimization/optimizers/mig.hpp"
// #include "algorithms/optimization/optimizers/xag.hpp"
// #include "algorithms/optimization/optimizers/xmg.hpp"
// #include "algorithms/optimization/optimizers/abc.hpp"

namespace oracle {

enum optimization_strategy { size, balanced, depth };

struct node_depth {
    int nodes;
    int depth;
};

template<typename network>
class optimizer
{
public:
    /**
     * human readable name
     */
    virtual const std::string optimizer_name() = 0;
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
     * List the type of optimization: area, delay, or balanced.
     */
    virtual optimization_strategy target() = 0;
    /**
     * Techmap, then return a path to a file containing the techmapped verilog.
     */
    virtual std::string techmap(const std::string &liberty_file, const std::string &temp_prefix) = 0;
    /**
     * convert the network to the superset.
     */
    virtual mockturtle::names_view<mockturtle::xmg_network> export_superset() = 0;
    /**
     * Reapply this optimization to a different network.
     */
    virtual optimizer<mockturtle::xmg_network> *reapply(int index, const mockturtle::window_view<mockturtle::names_view<mockturtle::xmg_network>> &part) = 0;
};

template <typename T>
class optimization_strategy_comparator {
public:
    // Comparator function
    virtual bool operator()(optimizer<T> &a, optimizer<T> &b) = 0;
    virtual const std::string name() = 0;
};

template <typename network>
optimizer<network> *optimize(optimization_strategy_comparator<network> &comparator,
                             optimization_strategy strategy,
                             const mockturtle::window_view<mockturtle::names_view<network>> &part,
                             int index,
                             const std::string &abc_exec);

template <typename T>
class ndp_strategy : public optimization_strategy_comparator<T>
{
    bool operator()(optimizer<T> &a, optimizer<T> &b)
    {
        node_depth x = a.independent_metric();
        node_depth y = b.independent_metric();

        return x.nodes * x.depth < y.nodes * y.depth;
    }
    const std::string name()
    {
        return "node-depth product";
    }
};

template <typename T>
class d_strategy : public optimization_strategy_comparator<T>
{
    bool operator()(optimizer<T> &a, optimizer<T> &b)
    {
        node_depth x = a.independent_metric();
        node_depth y = b.independent_metric();

        return x.depth < y.depth;
    }
    const std::string name()
    {
        return "depth";
    }
};

 template <typename T>
 class n_strategy : public optimization_strategy_comparator<T>
 {
     bool operator()(optimizer<T> &a, optimizer<T> &b)
     {
         node_depth x = a.independent_metric();
         node_depth y = b.independent_metric();

         return x.nodes < y.nodes;
     }
     const std::string name()
     {
         return "node";
     }
};

};
