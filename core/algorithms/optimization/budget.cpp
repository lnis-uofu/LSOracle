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
#ifdef ENABLE_OPENSTA
#ifdef ENABLE_ABC

#include <stdlib.h>
#include <mockturtle/mockturtle.hpp>
#include <sta/Sta.hh>
#include <sta/ConcreteNetwork.hh>
#include <sta/Corner.hh>
#include <sta/Graph.hh>
#include <sta/Liberty.hh>
#include <sta/Network.hh>
#include <sta/PathRef.hh>
#include <sta/PortDirection.hh>
#include <sta/TimingArc.hh>
#include <sta/PatternMatch.hh>
#include <sta/VerilogReader.hh>
#include <sta/StaMain.hh>

#include "algorithms/output/verilog_utilities.hpp"

namespace sta {
extern const char *tcl_inits[];
}
extern "C" {
extern int Sta_Init(Tcl_Interp *interp);
}


#include <filesystem>
#include <fmt/format.h>
#include <regex>
#include <tcl.h>
#include "algorithms/optimization/budget.hpp"
#include "algorithms/optimization/mig_script.hpp"
#include "algorithms/optimization/mig_script2.hpp"
#include "algorithms/optimization/mig_script3.hpp"
#include "algorithms/optimization/aig_script.hpp"
#include "algorithms/optimization/aig_script2.hpp"
#include "algorithms/optimization/aig_script3.hpp"
#include "algorithms/optimization/aig_script4.hpp"
#include "algorithms/optimization/aig_script5.hpp"
#include "algorithms/optimization/xag_script.hpp"
#include "algorithms/optimization/xmg_script.hpp"
#include "algorithms/partitioning/slack_view.hpp"
#include "utility.hpp"
// TODO replace "pi/po" with "ci/co"
namespace oracle
{
using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using xag_names = mockturtle::names_view<mockturtle::xag_network>;
using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
using xmg_manager = partition_manager_junior<mockturtle::xmg_network>;
using xmg_partition = mockturtle::window_view<mockturtle::names_view<mockturtle::xmg_network>>;

template<typename T>
std::string basic_techmap(const std::string &tech_script, const std::string &abc_exec, const T &optimal, const std::string &temp_prefix)
{
    std::cout << "starting basic techmapping" << std::endl;
    std::string input_blif, output_verilog, abc_script;
    if (temp_prefix.empty()) {
        char *blif = strdup("/tmp/lsoracle_XXXXXX.blif");
        if (mkstemps(blif, 5) == -1) {
            throw std::exception();
        }
        input_blif = std::string(blif);

        char *verilog = strdup("/tmp/lsoracle_XXXXXX.v");
        if (mkstemps(verilog, 2) == -1) {
            throw std::exception();
        }
        output_verilog = std::string(verilog);

        char *abc = strdup("/tmp/lsoracle_XXXXXX.abc");
        if (mkstemps(abc, 4) == -1) {
            throw std::exception();
        }
        abc_script = std::string(abc);
    } else {
        input_blif = fmt::format("{}.{}.tech.blif", temp_prefix, optimal.get_network_name());
        output_verilog = fmt::format("{}.{}.tech.v", temp_prefix, optimal.get_network_name());
        abc_script = fmt::format("{}.{}.tech.abc", temp_prefix, optimal.get_network_name());
    }

    std::cout << "generated blif " << input_blif << std::endl;
    std::cout << "writing output to " << output_verilog << std::endl;
    std::cout << "generated ABC script " << abc_script << std::endl;

    std::ofstream script(abc_script);
    script << "print_fanio; " << tech_script << "; print_fanio;" << std::endl;
    script.close();
    std::cout << "calling ABC" << std::endl;
    mockturtle::write_blif_params ps;
    ps.skip_feedthrough = 1u;
    mockturtle::write_blif(optimal, input_blif, ps);
    int code = system((abc_exec + " -F " + abc_script +
                       " -o " + output_verilog +
                       " " + input_blif).c_str());
    assert(code == 0);
    std::cout << "done techmapping" << std::endl;
    // TODO close everything
    return output_verilog;
};
template std::string basic_techmap<aig_names>(const std::string &, const std::string &, const aig_names &, const std::string &);
template std::string basic_techmap<xag_names>(const std::string &, const std::string &, const xag_names &, const std::string &);
template std::string basic_techmap<mig_names>(const std::string &, const std::string &, const mig_names &, const std::string &);
template std::string basic_techmap<xmg_names>(const std::string &, const std::string &, const xmg_names &, const std::string &);

template <typename network> std::string get_po_name_or_default(const network &ntk, const typename network::signal &signal)
{
    int index = ntk.po_index(signal);

    if (ntk.has_output_name(index)) {
        return ntk.get_output_name(index);
    } else {
        int digits_out = std::to_string(ntk.num_pos()).length();
        // std::cout << "missing output name for index " << index << std::endl;
        return fmt::format("po__{0:0{1}}", index, digits_out);
    }
}

template <typename network>
std::string get_pi_name_or_default(const network &ntk, const typename network::node &node)
{
    typename network::signal signal = ntk.make_signal(node);

    if (ntk.has_name(signal)) {
        return ntk.get_name(signal);
    } else {
        // std::cout << "missing name for PI node " << node << std::endl;
        int digits_in = std::to_string(ntk.num_pis()).length();
        return fmt::format("pi__{0:0{1}}", node, digits_in);
    }
}

template <typename network>
std::string get_node_name_or_default(const network &ntk, const typename network::node &node)
{
    if (ntk.is_pi(node)) {
        return get_pi_name_or_default(ntk, node);
    } else {
        typename network::signal signal = ntk.make_signal(node);
        if (ntk.has_name(signal)) {
            return  ntk.get_name(signal);
        } else {
            // std::cout << "missing name for non-PI node " << node << std::endl;
            int digits_gate = std::to_string(ntk.num_gates()).length();
            return fmt::format("node__{0:0{1}}", node, digits_gate);
        }
    }
}

template <typename network>
std::string get_ri_name_or_default(const network &ntk, const typename network::signal &signal)
{
    if (ntk.has_name(signal)) {
        return ntk.get_name(signal);
    } else {
        typename network::node node = ntk.get_node(signal);
        // std::cout << "missing name for RI node " << node << std::endl;
        int digits_in = std::to_string(ntk.num_registers()).length();
        return fmt::format("ri__{0:0{1}}", node, digits_in);
    }
}

template <typename network> void fix_names(partition_manager_junior<network> &partman,
                                           mockturtle::window_view<mockturtle::names_view<network>> &part,
                                           int index)
{
    mockturtle::names_view<network> ntk = partman.get_network();
    part.foreach_pi([&part, &ntk](typename network::node n) {
        std::string name = get_node_name_or_default(ntk, n);
        part.set_name(part.make_signal(n), name);
    });
    int feedthrough = 0;
    part.foreach_po([&part, &ntk, &feedthrough](typename network::signal s, int i) {
        typename network::node n = part.get_node(s);
        if (ntk.is_pi(n)) {
            feedthrough++;
            // skip feedthroughs
            return;
        }
        std::string name = get_node_name_or_default(ntk, n) + (s.complement ? "_c" : "");
        part.set_output_name(i, name);
    });
    if (feedthrough > 0 ) {
        std::cout << "Skipped renaming for " << feedthrough << " feedthrough." << std::endl;
    }
}

template <typename network>
mockturtle::window_view<mockturtle::names_view<network>> fix_names2(partition_manager_junior<network> &partman, int index)
{
    mockturtle::window_view<mockturtle::names_view<network>> part = partman.partition(index);
    mockturtle::names_view<network> ntk = partman.get_network();
    part.foreach_pi([&part, &ntk](typename network::node n) {
        std::string name = get_node_name_or_default(ntk, n);
        part.set_name(part.make_signal(n), name);
    });
    int feedthrough = 0;
    part.foreach_po([&part, &ntk, &feedthrough](typename network::signal s, int i) {
        typename network::node n = part.get_node(s);
        if (ntk.is_pi(n)) {
            feedthrough++;
            // skip feedthroughs
            return;
        }
        std::string name = get_node_name_or_default(ntk, n);
        part.set_output_name(i, name);
    });
    if (feedthrough > 0 ) {
        std::cout << "Skipped renaming for " << feedthrough << " feedthrough." << std::endl;
    }
    return part;
}

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
            string script =
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
            string script =
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
    string techmapped;
    string name;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class mig_optimizer<mockturtle::aig_network>;

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

            string script =
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
    string techmapped;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class aig_optimizer<mockturtle::aig_network>;


template< typename network>
class abc_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    abc_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(index, original, target, abc_exec) {}

    const std::string optimizer_name()
    {
        return "abc resyn2";
    }

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new abc_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    void optimize()
    {
        char *blif_name_char = strdup("/tmp/lsoracle_XXXXXX.blif");
        if (mkstemps(blif_name_char, 5) == -1) {
            throw std::exception();
        }
        std::string blif_name = std::string(blif_name_char);
        std::cout << "writing blif to " << blif_name  << std::endl;

        char *blif_output_name_char = strdup("/tmp/lsoracle_XXXXXX_optimized.blif");
        if (mkstemps(blif_output_name_char, 15) == -1) {
            throw std::exception();
        }
        std::string blif_output_name = std::string(blif_output_name_char);
        std::cout << "writing abc output to " << blif_output_name  << std::endl;

        mockturtle::write_blif_params ps;
        ps.skip_feedthrough = 1u;
        mockturtle::write_blif(this->converted, blif_name, ps);
        std::string script = "abc -c \"read_blif " + blif_name + "; resyn2; write_blif " + blif_output_name + " \"";
        int code = system((script).c_str());
        assert(code == 0);
        std::cout << "optimized with abc" << std::endl;

        mockturtle::names_view<mockturtle::klut_network> klut;
        lorina::return_code read_blif_return_code = lorina::read_blif(blif_output_name, mockturtle::blif_reader(klut));
        assert(read_blif_return_code == lorina::return_code::success);
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
        mockturtle::node_resynthesis(this->optimal, klut, resyn);
        this->optimal.set_network_name(this->converted.get_network_name());
    }
};

template <typename network>
class xag_optimizer: public optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    xag_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): index(index), original(original), strategy(target), abc_exec(abc_exec)
    {
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

            string script =
                "read_lib " + liberty_file +
                "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
            techmapped = basic_techmap<xag_names> (
                script, abc_exec, optimal, temp_prefix);
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
    string techmapped;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class xag_optimizer<mockturtle::xag_network>;

template <typename network>
class xmg_optimizer: public optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    xmg_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): index(index), original(original), strategy(target), abc_exec(abc_exec)
    {
    }

        optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new xmg_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    xmg_names export_superset()
    {
        mockturtle::direct_resynthesis<xmg_names> resyn;
        return mockturtle::node_resynthesis<xmg_names, xmg_names>(optimal, resyn);
    }

    void convert()
    {
        mockturtle::xmg_npn_resynthesis resyn;
        converted = mockturtle::node_resynthesis<xmg_names, partition>(original, resyn);
        converted.set_network_name("partition_" + std::to_string(index));
    }

    xmg_names optimized()
    {
        return optimal;
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_xmg_opt_depth{optimal};
        int xmg_opt_size = optimal.num_gates();
        int xmg_opt_depth = part_xmg_opt_depth.depth();
        metric = node_depth{xmg_opt_size, xmg_opt_depth};
        return metric;
    }

    std::string techmap(const std::string &liberty_file, const std::string &temp_prefix)
    {
        if (techmapped.empty()) {
            string script =
                "read_lib " + liberty_file +
                "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
            techmapped = basic_techmap<xmg_names> (
                script, abc_exec, optimal, temp_prefix);
        }
        return techmapped;
    }

    const std::string optimizer_name()
    {
        return "xmgscript";
    }

    void optimize()
    {
        oracle::xmg_script opt;
        this->optimal = opt.run(this->converted);
    }

    optimization_strategy target()
    {
        return strategy;
    }

protected:
    int index;
    partition original;
    xmg_names optimal;
    xmg_names converted;
    node_depth metric;
    string techmapped;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class xmg_optimizer<mockturtle::xmg_network>;

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

template <typename network>
class aigscript_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
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

template <typename T>
class optimization_strategy_comparator {
public:
    // Comparator function
    virtual bool operator()(optimizer<T> &a, optimizer<T> &b) = 0;
    virtual const string name() = 0;
};

template <typename T>
class ndp_strategy : public optimization_strategy_comparator<T>
{
    bool operator()(optimizer<T> &a, optimizer<T> &b)
    {
        node_depth x = a.independent_metric();
        node_depth y = b.independent_metric();

        return x.nodes * x.depth < y.nodes * y.depth;
    }
    const string name()
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
    const string name()
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
     const string name()
     {
         return "node";
     }
};

template <typename network>
optimizer<network> *optimize(optimization_strategy_comparator<network> &comparator,
                             optimization_strategy strategy,
                             partition_manager_junior<network> &partman,
                             int index,
                             const std::string &abc_exec)

{
    std::cout << "******************************** optimizing partition " << index << " ********************************" << std::endl;
    std::cout << "Optimizing based on strategy " << comparator.name() << std::endl;
    // mockturtle::window_view<mockturtle::names_view<network>> orig = partman.partition(index);
    // mockturtle::depth_view part_depth(orig);
    // std::cout << "Original depth " << part_depth.depth() << " gates " << part_depth.num_gates() << " size " << part_depth.size() << std::endl;
    // todo this is gonna leak memory.
    // const mockturtle::window_view<mockturtle::names_view<network>> part = partman.partition(index);
    // todo remove double network.
    // fix_names(partman, part, partman.get_network(), index);
    const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partman, index);
    std::vector<optimizer<network>*>optimizers {
        new noop<network>(index, part, strategy, abc_exec),
        new migscript_optimizer<network>(index, part, strategy, abc_exec),
        new migscript2_optimizer<network>(index, part, strategy, abc_exec),
        new migscript3_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript2_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript3_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript4_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript5_optimizer<network>(index, part, strategy, abc_exec),
        new xmg_optimizer<network>(index, part, strategy, abc_exec),
        new xag_optimizer<network>(index, part, strategy, abc_exec),
        // new abc_optimizer<network>(index, part, strategy, abc_exec),
   };
    optimizer<network> *best = nullptr;
    for (auto opt = optimizers.begin(); opt != optimizers.end(); opt++) {
        std::cout << "running optimization " << (*opt)->optimizer_name() << std::endl;
        // std::cout << "converting network" << std::endl;
        (*opt)->convert();
        // std::cout << "trying to optimize" << std::endl;
        (*opt)->optimize();
        // std::cout << "checking tech independent metrics." << std::endl;
        node_depth result = (*opt)->independent_metric();
        std::cout << "result depth " << result.depth
                  << " size " << result.nodes << std::endl;

        if (best == nullptr) {
            best = *opt;
            continue;
        }

        if (comparator(**opt, *best)) {
            best = *opt;
            //std::cout << "found a better result" << std::endl;
            continue;
        }
    }
    std::cout << "using " << best->optimizer_name() << " for " << index << std::endl;

    return best;

}

string join(std::string delim, std::set<string> input)
{
    std::vector<std::string> data(input.begin(), input.end());
    std::stringstream ss;
    if (input.size() == 0) {
        return "";
    } else if (input.size() == 1) {
        return *data.begin();
    } else {
        for (auto i = data.begin(); i != (data.end() - 1); i++) {
            ss << *i << delim;
        }
        ss << *(data.end() - 1);
        return ss.str();
    }
}


template <typename network>
std::set<std::string> get_wire_names(
                                     oracle::partition_manager_junior<network> &partitions,
                                     mockturtle::names_view<network> &ntk)
{
    std::set<std::string> wires;
    int num_parts = partitions.count();
    ntk.foreach_register([&ntk, &wires](std::pair<typename network::signal, typename network::node> reg) {
        typename network::signal ri = reg.first;
        typename network::node ro = reg.second;
        wires.insert(escape_id(get_node_name_or_default(ntk, ro)));
        wires.insert(escape_id(get_ri_name_or_default(ntk, ri)));
    });
    for (int i = 0; i < num_parts; i++) {
        mockturtle::window_view<mockturtle::names_view<network>> part = partitions.partition(i);
        part.foreach_pi([&ntk, &wires](typename network::node n) {
            if (!ntk.is_pi(n) && !ntk.is_constant(n)) {
                wires.insert(escape_id(get_node_name_or_default(ntk, n)));
            }
        });

        part.foreach_po([&ntk, &wires](typename network::signal s) {
            typename network::node n = ntk.get_node(s);
            if (!ntk.is_pi(n) && !ntk.is_constant(n)) {
                wires.insert(escape_id(get_node_name_or_default(ntk, n)));
            }
        });
    }
    return wires;
}

template <typename network>
void write_child(int index,
                 partition_manager_junior<network> &partman,
                 std::ofstream &verilog,
                 optimizer<network> *optimizer)
{
    std::cout << "writing out instance for partition " << index << std::endl;
    verilog << "partition_" << index << " partition_" << index << "_inst (\n";
    mockturtle::names_view<network> &ntk = partman.get_network();
    mockturtle::window_view<mockturtle::names_view<network>> part = partman.partition(index);

    part.foreach_pi([&part, &ntk, &verilog](typename network::node n) {
        std::string driver = escape_id(get_node_name_or_default(ntk, n));
        verilog << "." << driver << "(" << driver << "),\n";
    });

    part.foreach_po([&part, &ntk, &verilog](typename network::signal s, auto i) {
        typename network::node n = ntk.get_node(s);
        std::string driver = escape_id(get_node_name_or_default(ntk, n));
        verilog << "." << driver << "(" << driver << "),\n";
    });
    verilog.seekp(verilog.tellp() - 2L); // Truncate last comma
    verilog << "\n);\n" << std::endl;
}
void write_register(std::string abc_exec, std::string liberty, std::ofstream &verilog)
{
    // TODO techmap register
    verilog << "module mapped_register(D, Q, CLK);\n"
            << "input D, CLK;\n"
            << "output Q;\n"
            << "wire B;\n"
            << "sky130_fd_sc_hd__dfxtp_1 fflop (.CLK(CLK), .D(D), .Q(B));\n"
            << "sky130_fd_sc_hd__buf_8 buffer (.A(B), .X(Q));\n"
            << "endmodule\n" << std::endl;
}

void write_inverter(std::string abc_exec, std::string liberty, std::ofstream &verilog)
{
    // TODO techmap inverter
    verilog << "module mapped_inverter(A, Y);\n"
            << "input A; output Y;\n"
            << "sky130_fd_sc_hd__inv_1 i(.A(A), .Y(Y));\n"
            << "endmodule\n" << std::endl;
}

template <typename network>
string techmap(
    oracle::partition_manager_junior<network> partitions,
    std::vector<optimizer<network>*> optimized,
    const string &abc_exec,
    const string &liberty_file,
    const string &clock,
    const string &temp_prefix)
{
    std::cout << "Starting techmap." << std::endl;
    // Write out verilog
    char *output = strdup("/tmp/lsoracle_XXXXXX.combined.v");
    if (mkstemps(output, 11) == -1) {
        throw std::exception();
    }
    std::string output_file = std::string(output);
    // std::string output_file = fmt::format("{}.work.v", temp_prefix);
    std::cout << "Writing out to " << output_file << std::endl;

    std::ofstream verilog(output_file);
    verilog << "// Generated by LSOracle" << std::endl;

    // add all partition modules to verilog file.
    int num_parts = partitions.count();
    for (int i = 0; i < num_parts; i++) {
        std::cout << "******************************** techmapping partition " << i << " ********************************" << std::endl;
        optimizer<network> *opt = optimized[i];
        std::cout << "using optimizer " << opt->optimizer_name() << std::endl;
        std::string module_file = opt->techmap(liberty_file, temp_prefix);
        std::cout << "writing results" << std::endl;
        verilog << "// Partition " << i << std::endl;
        std::ifstream module(module_file);
        verilog << module.rdbuf();
        verilog << std::endl;
        module.close();
    }
    std::cout << "sub-modules written" << std::endl;
    verilog << "// Register" << std::endl;
    write_register(abc_exec, liberty_file, verilog);
    verilog << "// Inverter" << std::endl;
    write_inverter(abc_exec, liberty_file, verilog);
    verilog << "// Top" << std::endl;
    write_top(partitions, optimized, verilog, clock);

    verilog.close();
    return output_file;

}

void print_path(sta::ConcreteInstance *i)
{
    if (sta::ConcreteInstance *p = i->parent()) {
        print_path(p);
    }
    std::cout << "/" << i->name();
}

const std::regex inst_reg("partition_([0-9]+)_inst");
int get_partition_from_inst(sta::ConcreteInstance *i)
{
    std::smatch m;
    std::string name = std::string(i->name());
    if (std::regex_search(name, m, inst_reg)) {
         return std::stoi(m[1]);
    } else if (sta::ConcreteInstance *p = i->parent()) {
        return get_partition_from_inst(p);
    } else {
        return -1;
    }
}


template <typename network>
void write_top(oracle::partition_manager_junior<network> &partitions,
               std::vector<optimizer<network>*> &optimized,
               std::ofstream &verilog,
               const std::string &clock)
{
    // TODO escape names
    mockturtle::names_view<network> &ntk = partitions.get_network();
    std::string name = ntk.get_network_name().size() != 0
        ? escape_id(ntk.get_network_name())
        : "top";

    int inv_sequence = 0;
    int reg_sequence = 0;

    // gather output names
    std::set<std::string> outputs;
    // std::cout << "gathering po names." << std::endl;
    ntk.foreach_po([&outputs, &ntk](typename network::signal signal) {
        outputs.insert(escape_id(get_po_name_or_default(ntk, signal)));
    });
    std::string output_names = join(", ", outputs);

    // gather input names
    std::set<std::string> inputs;
    // std::cout << "gathering pi names." << std::endl;
    ntk.foreach_pi([&inputs, &ntk](typename network::node node) {
        inputs.insert(escape_id(get_pi_name_or_default(ntk, node)));
    });
    std::string input_names = join(", ", inputs);

    // gather wires
    std::set<std::string> wires = get_wire_names(partitions, ntk);
    std::string wire_names = join(", ", wires);

    verilog << fmt::format("module {}({}, {});\n",
                           name, input_names, output_names)
            << fmt::format("  input {};\n", input_names)
            << fmt::format("  output {};\n", output_names);
    if (wires.size() > 0) {
        verilog << fmt::format("  wire {};\n", wire_names);
    }

    // assign constant wires.
    // verilog << "wire " << escape_id(get_pi_name_or_default(ntk, ntk.get_node(ntk.get_constant(false)))) << ";\n";
    // verilog << "assign " << escape_id(get_pi_name_or_default(ntk, ntk.get_node(ntk.get_constant(false)))) << " = 1'b0;\n";
    // generate registers.
    ntk.foreach_register([&ntk, &clock, &verilog, &reg_sequence](std::pair<typename network::signal, typename network::node> reg) {
        typename network::signal ri = reg.first;
        typename network::node ro = reg.second;
        std::string output = escape_id(get_node_name_or_default(ntk, ro));
        std::string input = escape_id(get_ri_name_or_default(ntk, ri));
        verilog << "mapped_register reg__" << reg_sequence++ << " (.D(" << input << "), .Q(" << output << "), .CLK(" << escape_id(clock) << "));\n";
    });
    verilog << std::endl;

    // assign PO signal names to driver nodes. if complemented, create an inverter.
    std::set<std::string> assigns;

    ntk.foreach_po([&ntk, &assigns, &inv_sequence](typename network::signal signal) {
        std::string output = escape_id(get_po_name_or_default(ntk, signal));
        std::string driver = escape_id(get_node_name_or_default(ntk, ntk.get_node(signal)));

        if (output != driver) {
            if (ntk.is_complemented(signal)) {
                assigns.insert(fmt::format("mapped_inverter po_inv__{2} (.A({1}), .Y({0}));", output, driver, inv_sequence++));
            } else {
                // assigns.insert(fmt::format("assign {0} = {1};", output, driver));
                assigns.insert(fmt::format("sky130_fd_sc_hd__buf_8 po_buffer__{2} (.A({1}), .X({0}));", output, driver, inv_sequence++));
            }
        }
    });
    for (auto i = assigns.begin(); i != assigns.end(); i++) {
        verilog << *i << "\n";
    }

    std::set<std::string> regs;
    ntk.foreach_register([&ntk, &regs, &inv_sequence](std::pair<typename network::signal, typename network::node> reg) {
        typename network::signal ri = reg.first;
        std::string output = escape_id(get_ri_name_or_default(ntk, ri));
        std::string driver = escape_id(get_node_name_or_default(ntk, ntk.get_node(ri)));

        if (output != driver) {
            if (ntk.is_complemented(ri)) {
                regs.insert(fmt::format("mapped_inverter reg_inv__{2} (.A({1}), .Y({0}));", output, driver, inv_sequence++));

            } else {
                regs.insert(fmt::format("sky130_fd_sc_hd__buf_8 reg_buffer__{2} (.A({1}), .X({0}));", output, driver, inv_sequence++));
                // regs.insert(fmt::format("assign {0} = {1};", output, driver));
            }
        }
    });
    for (auto i = regs.begin(); i != regs.end(); i++) {
        verilog << *i << "\n";
    }
    verilog << std::endl;

    // write out module instances
    for (int i = 0; i < partitions.count(); i++) {
        write_child(i, partitions, verilog, optimized[i]);
    }
    verilog << "endmodule" << std::endl;
}

size_t run_timing(sta::LibertyLibrary *lib,
                  const std::string &liberty_file,
                  const std::string &verilog_file,
                  const std::string &sdc_file,
                  const std::string &design,
                  const int parts,
                  const std::vector<optimization_strategy> &optimized)
{
    bool read_ver = sta::readVerilogFile(verilog_file.c_str(),
    // bool read_ver = sta::readVerilogFile("/home/snelgrov/code/lsoracle/benchmarks/picorv32/picorv32_lsoracle.mapped.v",
                                         sta::Sta::sta()->networkReader());
    assert(read_ver); // << "failed to read verilog";
    bool linked = sta::Sta::sta()->linkDesign(design.c_str());
    assert(linked); // << "Failed to link";

    std::cout << "Attempting to read sdc " << sdc_file << std::endl;
    std::string read_sdc = "sta::read_sdc " + sdc_file;
    int a = Tcl_Eval(sta::Sta::sta()->tclInterp(), read_sdc.c_str());
    assert(a == 0);
    int b = Tcl_Eval(sta::Sta::sta()->tclInterp(), "sta::report_checks > /tmp/test.monkey");
    assert(b == 0);

    std::cout << "running timing" << std::endl;
    // sta::MinPeriodCheck *min = sta::Sta::sta()->minPeriodSlack();
    // // sta::MinPeriodCheck min{nullptr, nullptr};
    // sta::MinPeriodCheckSeq viol = sta::Sta::sta()->minPeriodViolations();
    // sta::Sta::sta()->reportChecks(&viol, true);
    // //    sta::Sta::sta()->reportCheck(min, true);

    // sta::MaxSkewCheck *skew = sta::Sta::sta()->maxSkewSlack();
    // sta::MaxSkewCheckSeq skew_viol = sta::Sta::sta()->maxSkewViolations();
    // sta::MaxSkewCheck skew;

    // sta::Sta::sta()->reportChecks(&skew_viol, true);
    //sta::Sta::sta()->reportCheck(skew, true);
    sta::Slack worst_slack;
    sta::Vertex *vertex;
    sta::Sta::sta()->worstSlack(sta::MinMax::max(), worst_slack, vertex);
    sta::PathRef worst_path_arrival;
    sta::PathRef worst_path_slack;

    sta::Sta::sta()->vertexWorstArrivalPath(vertex, sta::MinMax::max(), worst_path_arrival);
    sta::Sta::sta()->vertexWorstSlackPath(vertex, sta::MinMax::max(), worst_path_slack);

    sta::ConcreteNetwork *net = reinterpret_cast<sta::ConcreteNetwork*>(sta::Sta::sta()->networkReader());
    sta::ConcreteInstance *top = reinterpret_cast<sta::ConcreteInstance*>
                                 (net->topInstance());
    sta::CellSeq cells;
    sta::PatternMatch *pattern = new sta::PatternMatch("**", false, false, nullptr);
    net->findCellsMatching(reinterpret_cast<const sta::Library*>(lib), pattern, &cells);
    std::cout << "Number of cells " << cells.size() << std::endl;
    double area = 0;
    for (sta::Cell *cell: cells) {
        sta::ConcreteCell *ce = reinterpret_cast<sta::ConcreteCell*>(cell);
        sta::LibertyCell *c = ce->libertyCell();
        area += c->area();
    }
    std::cout << "Area " << area << std::endl;

    std::vector<float> budget(parts, 0.0);
    sta::PathRef second = worst_path_slack;
    sta::Arrival arrival = second.arrival(sta::Sta::sta());
    sta::TimingArc *arc;
    while(!second.isNull()) {
        sta::ConcretePin *pin_2 = reinterpret_cast<sta::ConcretePin*>(second.pin(sta::Sta::sta()));
        std::cout << "found pin " << pin_2->name();
        std::cout << " slack " << second.slack(sta::Sta::sta());
        std::cout << " arrival " << second.arrival(sta::Sta::sta());
        if (sta::ConcreteInstance *inst = pin_2->instance()) {
            std::cout << " on instance ";
            print_path(inst);
        }
        if (pin_2->net()) {
            std::cout << " on net " << pin_2->net()->name();
        }
        if (pin_2->term()) {
            std::cout << " on term " << pin_2->term()->name();
        }
        std::cout << std::endl;

        int index = get_partition_from_inst(pin_2->instance());
        if (index >= 0) {
            budget[index] += arrival - second.arrival(sta::Sta::sta());
        }
        arrival = second.arrival(sta::Sta::sta());
        second.prevPath(sta::Sta::sta(), second, arc);
    }

    if (worst_path_slack.slack(sta::Sta::sta()) >= 0.0) {
        return -1;
    }

    int max = -2;
    float worst = 0;
    for (int i = 0; i < parts; i++) {
        if (budget[i] > worst && optimized[i] != optimization_strategy::depth) { // if this is already fully optimized, move to next worst.
            worst = budget[i];
            max = i;
        }
    }

    // net->clear();
    // net->deleteTopInstance();
    return max;
}

void reset_sta()
{
    sta::Sta::sta()->clear();
    //sta::deleteAllMemory();

    sta::Sta *test = new sta::Sta;
    sta::Sta::setSta(test);
    //sta::initSta();
    sta::Sta::sta()->makeComponents();
    Tcl_Interp *tcl_interp = Tcl_CreateInterp();
    test->setTclInterp(tcl_interp);
    Sta_Init(tcl_interp);
    sta::evalTclInit(tcl_interp, sta::tcl_inits);
    Tcl_Eval(tcl_interp, "sta::define_sta_cmds");
    Tcl_Eval(tcl_interp, "namespace import sta::*");
}


template <typename network>
const int worst_indep(oracle::partition_manager_junior<network> &partitions,
                      std::vector<optimization_strategy> &optimized)
{
    oracle::slack_view<mockturtle::names_view<network>> slack(partitions.get_network());
  auto critical_path = slack.get_critical_path(partitions.get_network()); // TODO why does this pass itself back
  std::vector<int> budget(partitions.count(), 0);
  for (auto i = critical_path.begin(); i != critical_path.end(); i++) {
    budget[partitions.node_partition(*i)] += 1;
  }

  int max = -2;
  float worst = 0;
  for (int i = 0; i < partitions.count(); i++) {
    if (budget[i] > worst && optimized[i] != optimization_strategy::depth) { // if this is already fully optimized, move to next worst.
      worst = budget[i];
      max = i;
    }
  }
  return max;
}

template <typename network>
xmg_names setup_output(
        oracle::partition_manager_junior<network> &partitions_in,
        std::vector<optimizer<network>*> &optimized)
{
    int num_parts = partitions_in.count();
    mockturtle::direct_resynthesis<xmg_names> resyn;
    mockturtle::names_view<network> &ntk = partitions_in.get_network();
    xmg_names ntk_out = mockturtle::node_resynthesis<xmg_names, mockturtle::names_view<network>>(ntk, resyn);
    mockturtle::node_map<int, xmg_names> partitions_out_map(ntk_out);
    partitions_in.get_network().foreach_node([&](auto n){
        int i = ntk.node_to_index(n);
        partitions_out_map[ntk_out.index_to_node(i)] = partitions_in.node_partition(n);
    });
    xmg_manager partitions_out(ntk_out, partitions_out_map, partitions_in.count());

    for (int i = 0; i < num_parts; i++) {
        std::cout << "Partition " << i << " " << optimized[i]->optimizer_name() << " ";
        switch (optimized[i]->target()) {
        case optimization_strategy::depth: std::cout << "depth";
            break;
        case optimization_strategy::balanced: std::cout << "balanced";
            break;
        case optimization_strategy::size: std::cout << "size";
            break;
        }
        std::cout << std::endl;
        if (optimized[i]->optimizer_name() != "noop") {
            // const xmg_partition part = fix_names2(partitions_out, i);
            const xmg_partition part = partitions_out.partition(i);
            optimizer<mockturtle::xmg_network> *optim = optimized[i]->reapply(i, part);
            optim->convert();
            optim->optimize();

            xmg_names opt = optim->export_superset();
            partitions_out.integrate(i, partitions_out.partition(i), opt);
        }
    }
    partitions_out.substitute_nodes();
    std::cout << "Finished connecting outputs" << std::endl;
    return partitions_out.get_network();
}

/*
 * Mixed synthesis followed by XMG resynthesis and combination.
 */
template <typename network> xmg_names budget_optimization(
    oracle::partition_manager_junior<network> &partitions,
    const string &liberty_file,
    const string &sdc_file, const string &clock,
    const string &output_file, const string &abc_exec, const string &temp_prefix)
{
    sta::Corner *corner = new sta::Corner("tt", 0);
    sta::MinMaxAll *minmax = sta::MinMaxAll::all();
    sta::LibertyLibrary *lib = sta::Sta::sta()->readLiberty(liberty_file.c_str(),
                               corner,
                               minmax,
                               true);
    assert(lib != nullptr);// << "failed to read liberty library"

    int num_parts = partitions.count();
    std::vector<optimizer<network>*> optimized(num_parts);
    for (int i = 0; i < num_parts; i++) {
        n_strategy<network> strategy;
        optimized[i] = optimize(strategy, optimization_strategy::size, partitions, i, abc_exec);
    }
    assert(num_parts == optimized.size());

    string verilog;
    while (true) {
        //reset_sta(); // todo not cleaning up
        verilog = techmap(partitions, optimized, abc_exec, liberty_file, clock, temp_prefix);
        std::cout << "Wrote techmapped verilog to " << verilog << std::endl;
        std::vector<optimization_strategy> strats(optimized.size(), optimization_strategy::size);
        for (int i = 0; i < optimized.size(); i++) {
            strats[i] = optimized[i]->target();
        }
        const std::string design = partitions.get_network().get_network_name() != "" ? partitions.get_network().get_network_name() : "top";
        size_t worst_part = run_timing(lib, liberty_file, verilog, sdc_file, design, partitions.count(), strats);
        // TODO if this is worse than last result, rollback and finish.
        if (worst_part == -1) {
            std::cout << "met timing" << std::endl;
            break;
        }
        if (worst_part == -2) {  // TODO this is terrible way to use return value
            std::cout << "exhausted depth optimization for critical path" << std::endl;
            break;
        }
        if (optimized[worst_part]->target() == optimization_strategy::size) {
            ndp_strategy<network> strategy;
            optimized[worst_part] = optimize(strategy, optimization_strategy::balanced, partitions, worst_part, abc_exec);
        } else if (optimized[worst_part]->target() == optimization_strategy::balanced) {
            d_strategy<network> strategy;
            optimized[worst_part] = optimize(strategy, optimization_strategy::depth, partitions, worst_part, abc_exec);
        } else if (optimized[worst_part]->target() == optimization_strategy::depth) {
            std::cout << "previous result was already the best we can do." << std::endl;
            break; // met timing, or it's the best we can do.
        } else {
            throw "exhausted types";
        }
    }

    std::filesystem::copy(verilog, output_file, std::filesystem::copy_options::overwrite_existing);

    // Output network
    return setup_output(partitions, optimized);
}


/*
Mixed synthesis followed by XMG resynthesis and combiniation
*/
template <typename network> xmg_names optimization_simple(
    oracle::partition_manager_junior<network> &partitions,
    const string &liberty_file,
    const string &sdc_file, const string &clock,
    const string &output_file, const string &abc_exec)
{
    int num_parts = partitions.count();
    std::vector<optimizer<network>*> optimized(num_parts);
    for (int i = 0; i < num_parts; i++) {
        n_strategy<network> strategy;
        optimized[i] = optimize(strategy, optimization_strategy::size, partitions, i, abc_exec);
    }
    assert(num_parts == optimized.size());

    while (true) {
        std::vector<optimization_strategy> strats(optimized.size(), optimization_strategy::size);
        for (int i = 0; i < optimized.size(); i++) {
            strats[i] = optimized[i]->target();
        }
        const std::string design = partitions.get_network().get_network_name() != "" ? partitions.get_network().get_network_name() : "top";
        size_t worst_part = worst_indep<network>(partitions, strats);
        // TODO if this is worse than last result, rollback and finish.
        if (worst_part == -1) {
            std::cout << "met timing" << std::endl;
            break;
        }
        if (worst_part == -2) {  // TODO this is terrible way to use return value
            std::cout << "exhausted depth optimization for critical path" << std::endl;
            break;
        }
        if (optimized[worst_part]->target() == optimization_strategy::size) {
            ndp_strategy<network> strategy;
            optimized[worst_part] = optimize(strategy, optimization_strategy::balanced, partitions, worst_part, abc_exec);
        } else if (optimized[worst_part]->target() == optimization_strategy::balanced) {
            d_strategy<network> strategy;
            optimized[worst_part] = optimize(strategy, optimization_strategy::depth, partitions, worst_part, abc_exec);
        } else if (optimized[worst_part]->target() == optimization_strategy::depth) {
            std::cout << "previous result was already the best we can do." << std::endl;
            break; // met timing, or it's the best we can do.
        } else {
            throw "exhausted types";
        }
    }

    // Output network
    return setup_output(partitions, optimized);
}

/*
 * Mixed synthesis optimization followed by XMG resynthesis and combination.
 */
template <typename network>
xmg_names optimization_redux (
    oracle::partition_manager_junior<network> &partitions,
    const string &liberty_file,
    const string &sdc_file, const string &clock,
    const string &output_file, const string &abc_exec,
    optimization_strategy strategy)
{
  int num_parts = partitions.count();
  std::vector<optimizer<network>*> optimized(num_parts);
  optimization_strategy_comparator<network> *target;
  switch (strategy) {
  case optimization_strategy::depth: std::cout << "depth";
      target = new d_strategy<network>();
      break;
  case optimization_strategy::balanced: std::cout << "balanced";
      target = new ndp_strategy<network>();
      break;
  case optimization_strategy::size: std::cout << "size";
      target = new n_strategy<network>();
      break;
  }
  std::cout << std::endl;
  for (int i = 0; i < num_parts; i++) {
    optimized[i] = optimize(*target, strategy, partitions, i, abc_exec);
  }
  delete target;
  assert(num_parts == optimized.size());

  return setup_output(partitions, optimized);
}

/**************** Template instances ****************/
template xmg_names
budget_optimization<mockturtle::aig_network>
(
    oracle::partition_manager_junior<mockturtle::aig_network> &,
    const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);

template xmg_names
optimization_redux<mockturtle::aig_network>
(
    oracle::partition_manager_junior<mockturtle::aig_network> &,
    const std::string &, const std::string &, const std::string &, const std::string &, const std::string &,
    const optimization_strategy);

template xmg_names
optimization_simple<mockturtle::aig_network>
(
    oracle::partition_manager_junior<mockturtle::aig_network> &,
    const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);

}
#endif
#endif
