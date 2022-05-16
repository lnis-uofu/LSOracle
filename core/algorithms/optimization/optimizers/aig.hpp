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
#include "algorithms/optimization/aig_script.hpp"
#include "algorithms/optimization/aig_script2.hpp"
#include "algorithms/optimization/aig_script3.hpp"
#include "algorithms/optimization/aig_script4.hpp"
#include "algorithms/optimization/aig_script5.hpp"

#include "algorithms/optimization/optimizers/techmapping.hpp"

namespace oracle {
    using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
    using aig_names = mockturtle::names_view<mockturtle::aig_network>;
    using xmg_partition = mockturtle::window_view<mockturtle::names_view<mockturtle::xmg_network>>;

template <typename network>
class aig_optimizer: public optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;

public:
    aig_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): index(index), original(original), strategy(target), abc_exec(abc_exec)
    {
    }

    xmg_names export_superset()
    {
        mockturtle::direct_resynthesis<xmg_names> resyn;
        return mockturtle::node_resynthesis<xmg_names, aig_names>(optimal, resyn);
    }

    void convert()
    {
        mockturtle::xag_npn_resynthesis<aig_names> resyn;
        converted = mockturtle::node_resynthesis<aig_names, partition> (original, resyn);
        converted.set_network_name("partition_" + std::to_string(index));
    }

    aig_names optimized()
    {
        return optimal;
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_aig_opt_depth{optimal};
        int aig_opt_size = optimal.num_gates();
        int aig_opt_depth = part_aig_opt_depth.depth();
        metric = node_depth{aig_opt_size, aig_opt_depth};
        return metric;
    }

    std::string techmap(const std::string &liberty_file, const std::string &temp_prefix)
    {
        if (techmapped.empty()) {

            std::string script =
                "read_lib " + liberty_file +
                "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
            techmapped = basic_techmap<aig_names> (
                script, abc_exec, optimal, temp_prefix);
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
    aig_names optimal;
    aig_names converted;
    node_depth metric;
    std::string techmapped;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class aig_optimizer<mockturtle::aig_network>;



template <typename network>
class aigscript_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
    using xmg_partition = mockturtle::window_view<mockturtle::names_view<mockturtle::xmg_network>>;
public:
    aigscript_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(index, original, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new aigscript_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript";
    }

    void optimize()
    {
        oracle::aig_script opt;
        this->optimal = opt.run(this->converted);
    }
};

template <typename network>
class aigscript2_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    aigscript2_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(index, original, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new aigscript2_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript2";
    }

    void optimize()
    {
        oracle::aig_script2 opt;
        this->optimal = opt.run(this->converted);
    }
};

template <typename network>
class aigscript3_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    aigscript3_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(index, original, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new aigscript3_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript3";
    }

    void optimize()
    {
        oracle::aig_script3 opt;
        this->optimal = opt.run(this->converted);
    }
};

template <typename network>
class aigscript4_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    aigscript4_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(index, original, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new aigscript4_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript4";
    }

    void optimize()
    {
        oracle::aig_script4 opt;
        this->optimal = opt.run(this->converted);
    }
};

template <typename network>
class aigscript5_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    aigscript5_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(index, original, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new aigscript5_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript5";
    }

    void optimize()
    {
        oracle::aig_script5 opt;
        this->optimal = opt.run(this->converted);
    }
};
};
