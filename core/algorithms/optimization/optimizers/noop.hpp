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
#include "algorithms/optimization/optimizers.hpp"
namespace oracle {

template <typename network>
class noop: public optimizer<network>
{
    using names = mockturtle::names_view<network>;
    using partition = mockturtle::window_view<names>;
    // using manager = partition_manager_junior<network>;

public:
    noop(int index, const partition &part, optimization_strategy target, const std::string &abc_exec): index(index), original(part), strategy(target), abc_exec(abc_exec)
    {
    }

    const std::string optimizer_name()
    {
        return "noop";
    }


    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new noop<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }


    // mockturtle::window_view<mockturtle::names_view<network>> partition()
    // {
    //         return partman.create_part(ntk, index);
    // }

    optimization_strategy target()
    {
        return strategy;
    }

    xmg_names export_superset()
    {
        mockturtle::direct_resynthesis<xmg_names> resyn;
        return mockturtle::node_resynthesis<xmg_names, names>(copy, resyn);
    }

    /*
     * Do direct resynthesis to create a copy.
     */
    void convert()
    {
        // partition original = partman.partition(index);
        // // original.set_network_name("partition_" + std::to_string(index)); // TODO not working?
        mockturtle::direct_resynthesis<names> resyn;
        copy = mockturtle::node_resynthesis<names, partition> (original, resyn);
        copy.set_network_name("partition_" + std::to_string(index));
    }

    names optimized()
    {
        return copy;
    }

    void optimize()
    {
    }

    std::string techmap(const std::string &liberty_file, const std::string &temp_prefix)
    {
        if (techmapped.empty()) {
            std::string script =
                "read_lib " + liberty_file +
                "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c;";
            techmapped = basic_techmap(script, abc_exec, copy, temp_prefix);
        }
        return techmapped;
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_depth(copy);
        int opt_size = part_depth.num_gates();
        int opt_depth = part_depth.depth();
        metric = node_depth{opt_size, opt_depth};
        return metric;
    }
private:
    int index;
    partition original;
    names copy;
    node_depth metric;
    optimization_strategy strategy;
    std::string techmapped;
    const std::string &abc_exec;
};
template class noop<mockturtle::aig_network>;
template class noop<mockturtle::mig_network>;
template class noop<mockturtle::xag_network>;
template class noop<mockturtle::xmg_network>;
};
