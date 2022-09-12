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
namespace oracle {
#include <string>
#include <mockturtle/mockturtle.hpp>
#include "algorithms/optimization/optimizers/techmapping.hpp"
#include "algorithms/optimization/xag_script.hpp"

template <typename network>
class xag_optimizer: public optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    xag_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): index(index), original(original), strategy(target), abc_exec(abc_exec)
    {
    }

    std::string get_network_name()
    {
        return "partition_" + std::to_string(index);
    }

    xmg_names export_superset()
    {
        mockturtle::direct_resynthesis<xmg_names> resyn;
        return mockturtle::node_resynthesis<xmg_names, xag_names>(optimal, resyn);
    }

        optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new xag_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    void convert()
    {
        mockturtle::xag_npn_resynthesis<xag_names> resyn;
        converted = mockturtle::node_resynthesis<xag_names, partition> (original, resyn);
        converted.set_network_name("partition_" + std::to_string(index));
    }

    xag_names optimized()
    {
        return optimal;
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_xag_opt_depth{optimal};
        int xag_opt_size = optimal.num_gates();
        int xag_opt_depth = part_xag_opt_depth.depth();
        metric = node_depth{xag_opt_size, xag_opt_depth};
        return metric;
    }

    std::string techmap(const std::string &liberty_file, const std::string &temp_prefix)
    {
        if (techmapped.empty()) {

            std::string script =
                "read_lib " + liberty_file +
                "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
            techmapped = basic_techmap<xag_names> (
                script, abc_exec, optimal,
                fmt::format("{}.{}.partition_{}", temp_prefix, this->optimizer_name(), index));
        }
        return techmapped;
    }

    const std::string optimizer_name()
    {
        return "xagscript";
    }

    void optimize()
    {
        oracle::xag_script opt;
        this->optimal = opt.run(this->converted);
        this->optimal.set_network_name(this->original.get_network_name());
    }

    optimization_strategy target()
    {
        return strategy;
    }

protected:
    int index;
    partition original;
    xag_names optimal;
    xag_names converted;
    node_depth metric;
    std::string techmapped;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class xag_optimizer<mockturtle::xag_network>;

}
