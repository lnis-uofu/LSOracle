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
#include "algorithms/optimization/optimizers/techmapping.hpp"
#include "algorithms/optimization/mig_script.hpp"
#include "algorithms/optimization/mig_script2.hpp"
#include "algorithms/optimization/mig_script3.hpp"

namespace oracle {
template <typename network>
class mig_optimizer: public optimizer<network>
{
    using names = mockturtle::names_view<network>;
    using partition = mockturtle::window_view<names>;
    using manager = mockturtle::window_view<names>;

public:
    mig_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): index(index), original(original), strategy(target), abc_exec(abc_exec)
    {
    }

    xmg_names export_superset()
    {
        mockturtle::direct_resynthesis<xmg_names> resyn;
        return mockturtle::node_resynthesis<xmg_names, mig_names>
                (optimal, resyn);
    }

    void convert()
    {
        // mockturtle::mig_npn_resynthesis resyn;
        mockturtle::direct_resynthesis<mig_names> resyn;
        converted =
            mockturtle::node_resynthesis<mig_names, mockturtle::window_view<mockturtle::names_view<network>>>
            (original, resyn);
        converted.set_network_name("partition_" + std::to_string(index));

    }

    mig_names optimized()
    {
        return optimal;
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_mig_opt_depth{optimal};
        int mig_opt_size = optimal.num_gates();
        int mig_opt_depth = part_mig_opt_depth.depth();
        metric = node_depth{mig_opt_size, mig_opt_depth};
        return metric;
    }

    std::string techmap(const std::string &liberty_file, const std::string &temp_prefix)
    {
        if (techmapped.empty()) {
            std::string script =
                "read_lib " + liberty_file +
                "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
            techmapped = basic_techmap<mig_names> (script, abc_exec, optimal, temp_prefix);
        }
        return techmapped;
    }

    optimization_strategy target()
    {
        return strategy;
    }
protected:
    int index;
    partition original;
    mig_names optimal;
    mig_names converted;
    node_depth metric;
    std::string techmapped;
    std::string name;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class mig_optimizer<mockturtle::aig_network>;

template <typename network>
class migscript_optimizer: public mig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    migscript_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): mig_optimizer<network>(index, original, target, abc_exec) {}

    const std::string optimizer_name()
    {
        return "migscript";
    }

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new migscript_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    void optimize()
    {
        oracle::mig_script migopt;
        this->optimal = migopt.run(this->converted);
    }
};

template <typename network>
class migscript2_optimizer: public mig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    migscript2_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): mig_optimizer<network>(index, original, target, abc_exec) {}

    const std::string optimizer_name()
    {
        return "migscript2";
    }

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new migscript2_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    void optimize()
    {
        oracle::mig_script2 migopt;
        this->optimal = migopt.run(this->converted);
    }
};

template <typename network>
class migscript3_optimizer: public mig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    migscript3_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): mig_optimizer<network>(index, original, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new migscript3_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "migscript3";
    }

    void optimize()
    {
        oracle::mig_script3 migopt;
        this->optimal = migopt.run(this->converted);
    }
};
};
