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
#include <sta/Network.hh>
#include <sta/Corner.hh>
#include <sta/VerilogReader.hh>
#include <sta/Liberty.hh>
#include <sta/ConcreteNetwork.hh>
#include <sta/PortDirection.hh>
#include <fmt/format.h>
#include "algorithms/optimization/budget.hpp"
#include "algorithms/partitioning/partition_manager.hpp"
#include "algorithms/optimization/mig_script.hpp"
#include "algorithms/optimization/aig_script.hpp"
#include "utility.hpp"

namespace oracle
{
using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using aig_ntk = std::shared_ptr<aig_names>;
using part_man_aig = oracle::partition_manager<aig_names>;
using part_man_aig_ntk = std::shared_ptr<part_man_aig>;
using part_view_aig =
    partition_view<mockturtle::names_view<mockturtle::aig_network>>;
using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using mig_ntk = std::shared_ptr<mig_names>;
using part_man_mig = oracle::partition_manager<mig_names>;
using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

template<typename T>
std::string basic_techmap(string tech_script, T optimal)
{
    std::cout << "starting basic techmapping" << std::endl;
    char *blif = strdup("/tmp/lsoracle_XXXXXX.blif");
    if (mkstemps(blif, 5) == -1) {
        throw std::exception();
    }
    std::string input_blif = std::string(blif);
    std::cout << "generated blif " << input_blif << std::endl;

    char *verilog = strdup("/tmp/lsoracle_XXXXXX.v");
    if (mkstemps(verilog, 2) == -1) {
        throw std::exception();
    }
    std::string output_verilog = std::string(verilog);
    std::cout << "writing output to " << output_verilog << std::endl;

    char *abc = strdup("/tmp/lsoracle_XXXXXX.abc");
    if (mkstemps(abc, 4) == -1) {
        throw std::exception();
    }
    std::string abc_script = std::string(abc);
    std::cout << "generated ABC script " << abc_script << std::endl;

    std::ofstream script(abc_script);
    script << tech_script << std::endl;
    script.close();
    std::cout << "calling ABC" << std::endl;
    mockturtle::write_blif_params ps;
    ps.module_name = optimal.get_network_name();
    std::cout << "got module name " << ps.module_name << std::endl;
    mockturtle::write_blif(optimal, input_blif, ps);
    int code = system(("abc -F " + abc_script +
                       " -o " + output_verilog +
                       " " + input_blif).c_str());
    assert(code == 0);
    std::cout << "done techmapping" << std::endl;
    // TODO close everything
    return output_verilog;
};
template std::string basic_techmap<aig_names> (std::string, aig_names);
template std::string basic_techmap<mig_names>(std::string, mig_names);

template <typename Ntk>
class noop: public optimizer<Ntk>
{
public:
    noop(partition_view<mockturtle::names_view<Ntk>> ntk): original(ntk)
    {
    }

    std::string optimizer_name()
    {
        return "noop";
    }

    partition_view<mockturtle::names_view<Ntk>> partition()
    {
        return original;
    }

    std::string module_name()
    {
        return original.get_network_name();
    }

    optimization_strategy target()
    {
        return optimization_strategy::balanced;
    }

    mockturtle::names_view<Ntk> reconvert()
    {
        return copy;
    }

    /*
     * Do direct resynthesis to create a copy.
     */
    void convert()
    {
	// mockturtle::topo_view topo(original);
	// mockturtle::write_blif(original, original.get_network_name() + "_original.blif");
	// mockturtle::write_verilog(original, original.get_network_name() + "_original.v");
        mockturtle::direct_resynthesis<mockturtle::names_view<Ntk>> resyn;
        copy = mockturtle::node_resynthesis<mockturtle::names_view<Ntk>, partition_view<mockturtle::names_view<Ntk>>>
                (original, resyn);
        // std::cout << "noop original partition size " << original.num_gates() <<
        //           std::endl;
	// std::cout << "topo_view partition size" << topo.num_gates() << std::endl;
        // std::cout << "noop copy partition size " << copy.num_gates() << std::endl;
	// mockturtle::write_verilog(copy, copy.get_network_name() + "_copy.v");
	// mockturtle::write_blif(copy, copy.get_network_name() + "_copy.blif");
    }


    mockturtle::names_view<Ntk> optimized()
    {
        return copy;
    }

    void optimize()
    {
    }

    std::string techmap(std::string liberty_file)
    {
        string script =
            "read_lib " + liberty_file +
            "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
        techmapped = basic_techmap(script, copy);
        return techmapped;
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_depth(copy);
        int opt_size = original.num_gates();
        int opt_depth = part_depth.depth();
        metric = node_depth{opt_size, opt_depth};
        return metric;
    }
private:
    partition_view<mockturtle::names_view<Ntk>> original;
    mockturtle::names_view<Ntk> copy;
    node_depth metric;
    string techmapped;
};

template class noop<mockturtle::aig_network>;

template <typename Ntk>
class mig_optimizer: public optimizer<Ntk>
{
public:
    mig_optimizer(partition_view<mockturtle::names_view<Ntk>> ntk): original(ntk)
    {
    }

    std::string optimizer_name()
    {
        return "basic migscript optimizer";
    }

    std::string module_name()
    {
        return original.get_network_name();
    }

    optimization_strategy target()
    {
        return optimization_strategy::depth;
    }

    mockturtle::names_view<Ntk> reconvert()
    {
        /* LUT mapping */
        mockturtle::mapping_view<mockturtle::names_view<mockturtle::mig_network>, true>
        mapped{optimal};
        mockturtle::lut_mapping_params ps;
        ps.cut_enumeration_ps.cut_size = 3;
        mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::names_view<mockturtle::mig_network>, true>, true>
        (mapped, ps);

        /* collapse into k-LUT network */
        const auto klut =
            *mockturtle::collapse_mapped_network<mockturtle::names_view<mockturtle::klut_network>>
            (mapped);

        /* node resynthesis */
        mockturtle::direct_resynthesis<mockturtle::names_view<Ntk>> resyn;
	// mockturtle::xag_npn_resynthesis<mockturtle::names_view<Ntk>> resyn;
        return mockturtle::node_resynthesis<mockturtle::names_view<Ntk>>(klut, resyn);
    }

    partition_view<mockturtle::names_view<Ntk>> partition()
    {
        return original;
    }

    void convert()
    {
        mockturtle::mig_npn_resynthesis resyn;
        converted =
            mockturtle::node_resynthesis<mockturtle::names_view<mockturtle::mig_network>>
            (original, resyn);
    }

    mockturtle::names_view<mockturtle::mig_network> optimized()
    {
        return optimal;
    }

    void optimize()
    {
        oracle::mig_script migopt;
        optimal = migopt.run(converted);
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_mig_opt_depth{optimal};
        int mig_opt_size = optimal.num_gates();
        int mig_opt_depth = part_mig_opt_depth.depth();
        metric = node_depth{mig_opt_size, mig_opt_depth};
        return metric;
    }

    std::string techmap(std::string liberty_file)
    {
        string script =
            "read_lib " + liberty_file +
            "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
        techmapped = basic_techmap<mockturtle::names_view<mockturtle::mig_network>> (
                         script, optimal);
        return techmapped;
    }

private:
    partition_view<mockturtle::names_view<Ntk>> original;
    mockturtle::names_view<mockturtle::mig_network> optimal;
    mockturtle::names_view<mockturtle::mig_network> converted;
    node_depth metric;
    string techmapped;
};
template class mig_optimizer<mockturtle::aig_network>;

/*
class ndp_aig_optimizer: optimizer<mockturtle::mig_network, node_depth>
{
std::tuple<mockturtle::mig_network, node_depth> optimize(
    partition_view<mockturtle::mig_network> part)
{
    oracle::aig_script aigopt;
    auto opt_aig = *mig_to_aig(part);
    opt_aig = aigopt.run(opt_aig);
    mockturtle::depth_view part_aig_opt_depth{opt_aig};
    int aig_opt_size = opt_aig.num_gates();
    int aig_opt_depth = part_aig_opt_depth.depth();
    auto opt_aig_mig = *aig_to_mig(opt_aig, 0);
    return std::tuple{opt_aig_mig, node_depth{aig_opt_size, aig_opt_depth}};
}
};

class ndp_mig_optimizer: optimizer<mockturtle::mig_network, node_depth>
{
std::tuple<mockturtle::mig_network, node_depth> optimize()
{
    auto opt_mig = *part_to_mig(part, 0);
    oracle::mig_script migopt;
    opt_mig = migopt.run(opt_mig);
    mockturtle::depth_view part_mig_opt_depth{opt_mig};
    int mig_opt_size = opt_mig.num_gates();
    int mig_opt_depth = part_mig_opt_depth.depth();
    return std::tuple{opt_mig, node_depth{mig_opt_size, mig_opt_depth}};
}
};

class noop_optimizer: optimizer<mockturtle::mig_network, node_depth>
{
std::tuple<mockturtle::mig_network, node_depth> optimize()
{
    auto opt_mig = *part_to_mig(part, 0);
    mockturtle::depth_view part_mig_opt_depth{opt_mig};
    int mig_opt_size = opt_mig.num_gates();
    int mig_opt_depth = part_mig_opt_depth.depth();
    return std::tuple{opt_mig, node_depth{mig_opt_size, mig_opt_depth}};
}
};
*/

template <typename network, typename F>
optimizer<network> *optimize_by(partition_view<mockturtle::names_view<network>>
                                part,
                                std::vector<optimizer<network>*> optimizers, F better)
{
}

template <typename network>
optimizer<network> *optimize_depth(
    partition_view<mockturtle::names_view<network>> part)
{
    std::cout << "Optimizing for depth" << std::endl;
    // todo this is gonna leak memory.
    std::vector<optimizer<network>*> optimizers;
    optimizers.emplace_back(new noop<network>(part));
    // optimizers.emplace_back(new mig_optimizer<network>(part));
    optimizer<network> *best = nullptr;
    for (auto opt = optimizers.begin(); opt != optimizers.end(); opt++) {
        std::cout << "running optimization " << (*opt)->optimizer_name() << std::endl;
        std::cout << "converting network" << std::endl;
        (*opt)->convert();
        std::cout << "trying to optimize" << std::endl;
        (*opt)->optimize();
        std::cout << "checking tech independent metrics." << std::endl;
        node_depth result = (*opt)->independent_metric();
        std::cout << "result depth " << result.depth
                  << " size " << result.nodes << std::endl;

        if (best == nullptr) {
            best = *opt;
            continue;
        }
        // TODO stash value earlier and don't call this twice
        if ((*opt)->independent_metric().depth < best->independent_metric().depth) {
            best = *opt;
            std::cout << "found a better result" << std::endl;
            continue;
        }
    }
    return best;
}

template <typename network>
optimizer<network> *optimize_area(
    partition_view<mockturtle::names_view<network>> part)
{
    std::cout << "Optimizing for area" << std::endl;
    // todo this is gonna leak memory.
    std::vector<optimizer<network>*> optimizers;
    optimizers.emplace_back(new noop<network>(part));
    // optimizers.emplace_back(new mig_optimizer<network>(part));
    optimizer<network> *best = nullptr;
    for (auto opt = optimizers.begin(); opt != optimizers.end(); opt++) {
        std::cout << "running optimization " << (*opt)->optimizer_name() << std::endl;
        std::cout << "converting network" << std::endl;
        (*opt)->convert();
        std::cout << "trying to optimize" << std::endl;
        (*opt)->optimize();
        std::cout << "checking tech independent metrics." << std::endl;
        node_depth result = (*opt)->independent_metric();
        std::cout << "result depth " << result.depth
                  << " size " << result.nodes << std::endl;

        if (best == nullptr) {
            best = *opt;
            continue;
        }

        if ((*opt)->independent_metric().nodes < best->independent_metric().nodes) {
            best = *opt;
            std::cout << "found a better result" << std::endl;
            continue;
        }
    }
    return best;

}

string join(std::string delim, std::vector<string> data)
{
    std::stringstream ss;
    for (auto i = data.begin(); i != (data.end() - 1); i++) {
        ss << *i << delim;
    }
    ss << *(data.end() - 1);
    return ss.str();
}

template <typename network>
std::map<std::string, std::string> get_assigns(
    mockturtle::names_view<network> ntk)
{
    std::map<std::string, std::string> assigns;
    ntk.foreach_po([&ntk, &assigns](typename network::signal n, int i) {
        int index = ntk.po_index(n);

        if (ntk.has_output_name(index) && ntk.has_name(n)
                && ntk.get_output_name(index) != ntk.get_name(n)) {
            assigns[ntk.get_output_name(index)] = ntk.get_name(n);
        }
    });
    // TODO check for signal complements.
    return assigns;
}

template <typename network>
std::vector<std::string> get_po_names(mockturtle::names_view<network> ntk)
{
    std::vector<std::string> outputs;

    int digits_out = std::to_string(ntk.num_pos()).length();
    std::cout << "gathering po names." << std::endl;
    ntk.foreach_po([&outputs, &ntk, &digits_out](typename network::signal n, int i) {
        int index = ntk.po_index(n);
        if (ntk.has_output_name(index)) {
            std::string name = ntk.get_output_name(index);
            // std::cout << "found po " << name
            //           <<  " " << index << std::endl;
            outputs.emplace_back(name);
        } else {
            std::cout << "missing name for po " << ntk.po_index(n) << std::endl;
            outputs.emplace_back(fmt::format("po{0:0{1}}", i, digits_out));
        }
    });
    return outputs;
}

template <typename network>
std::vector<std::string> get_pi_names(mockturtle::names_view<network> ntk)
{
    std::vector<std::string> inputs;
    int digits_in  = std::to_string(ntk.num_pis()).length();

    std::cout << "gathering pi names." << std::endl;
    ntk.foreach_pi([&inputs, &ntk, &digits_in](typename network::node n,
    int i) {
        typename network::signal s = ntk.make_signal(n);
        if (ntk.has_name(s)) {
            std::string name = ntk.get_name(s);
            // std::cout << "found input " << name
            //           << " for node " << n << std::endl;
            inputs.emplace_back(name);
        } else {
            std::cout << "missing name for pi " << n << std::endl;
            inputs.emplace_back(fmt::format("pi{0:0{1}}", i, digits_in));
        }
    });
    return inputs;
}

template <typename network>
std::set<std::string> get_wire_names(
    oracle::partition_manager<mockturtle::names_view<network>> partitions
)
{
    int digits_in  = std::to_string(partitions.num_pis()).length();
    int digits_out = std::to_string(partitions.num_pos()).length();
    // for each po/pi on a partition, if it is not a top po/pi, add to list.
    std::set<std::string> wires;
    for (const auto [part, nodes] : partitions.get_all_partition_inputs()) {
        for (const auto node : nodes) {
            if (!partitions.is_pi(node)) {
                typename network::signal sig = partitions.make_signal(node);
                if (partitions.has_name(sig)) {
                    std::string name = partitions.get_name(sig);
                    std::cout << "found input wire " << name
                              << " for node " << node << std::endl;
                    wires.insert(name);
                } else {
                    std::cout << "missing name for pi wire " << node << std::endl;
                    wires.insert(fmt::format("wi{0:0{1}}", node, digits_in));
                }
            }
        }
    }
    for (const auto [part, nodes] : partitions.get_all_partition_outputs()) {
        for (const auto node : nodes) {
            typename network::signal sig = partitions.make_signal(node);
            if (!is_po(partitions, node)) {
                if (partitions.has_name(sig)) {
                    std::string name = partitions.get_name(sig);
                    std::cout << "found output wire " << name
                              << " for node " << node << std::endl;
                    wires.insert(name);
                } else {
                    std::cout << "missing name for po wire " << node << std::endl;
                    wires.insert(fmt::format("wi{0:0{1}}", node, digits_out));
                }
            }
        }
    }

    return wires;
}

template <typename network>
void write_child(int index,
		 optimizer<network>* optimize,
                 std::ofstream &verilog)
{
    std::cout << "writing out instance for partition " << index << std::endl;
    verilog << optimize->module_name() << " partition_" << index << "_inst (\n";
    oracle::partition_view<mockturtle::names_view<network>> part =
                optimize->partition();
    // std::vector<std::string> inputs = get_pi_names(part);  // TODO
    // std::vector<std::string> outputs = get_po_names(part);  // TODO
    int digits_in  = std::to_string(part.num_pis()).length();

    std::cout << "gathering pi names." << std::endl;
    part.foreach_pi([&part, &digits_in, &verilog](typename network::node n,
    int i) {
        typename network::signal s = part.make_signal(n);
        if (part.has_name(s)) {
            std::string name = part.get_name(s);
            // std::cout << "found input " << name
            //           << " for node " << n << std::endl;
	    verilog << "." << name << "(" << name << "),\n";
	} else {
            std::string name = fmt::format("wi{0:0{1}}", i, digits_in);
            std::cout << "missing name for pi " << n << std::endl;
	    verilog << "." << name << "(" << name << "),\n";
        }
    });

    int digits_out = std::to_string(part.num_pos()).length();
    std::cout << "gathering po names." << std::endl;
    part.foreach_po([&part, &digits_out, &verilog](typename network::signal n, int i) {
        int index = part.po_index(n);
        if (part.has_output_name(index)) {
            std::string name = part.get_output_name(index);
            // std::cout << "found po " << name
            //           <<  " " << index << std::endl;
	    verilog << "." << name << "(" << name << "),\n";
        } else if (part.has_name(n)) {
            std::string name = part.get_name(n);
            // std::cout << "found po " << name
            //           <<  " " << index << std::endl;
	    verilog << "." << name << "(" << name << "),\n";
	} else {
            std::cout << "missing name for po " << part.get_node(n) << std::endl;
            std::string name = fmt::format("wi{0:0{1}}", i, digits_out);

	    verilog << "." << name << "(" << name << "),\n";
        }
    });
    verilog.seekp(verilog.tellp() - 2L); // Truncate last comma
    verilog << "\n);\n" << std::endl;
}

template <typename network>
string techmap(
    mockturtle::names_view<network> ntk,
    oracle::partition_manager<mockturtle::names_view<network>> partitions,
    std::vector<optimizer<network>*> optimized,
    string liberty_file)
{
    std::cout << "Starting techmap." << std::endl;
    // Write out verilog
    char *output = strdup("/tmp/lsoracle_XXXXXX.v");
    if (mkstemps(output, 2) == -1) {
        throw std::exception();
    }

    std::string output_file = std::string(output);
    std::cout << "Writing out to " << output_file << std::endl;

    std::ofstream verilog(output_file);
    verilog << "// Generated by LSOracle" << std::endl;

    // add all partition modules to verilog file.
    int num_parts = partitions.get_part_num();
    for (int i = 0; i < num_parts; i++) {
        std::cout << "techmapping partition " << i << std::endl;
        optimizer<network> *opt = optimized[i];
        std::cout << "using optimizer " << opt->optimizer_name() << std::endl;
        std::string module_file = opt->techmap(liberty_file);
        std::cout << "writing results" << std::endl;
        std::ifstream module(module_file);
        verilog << module.rdbuf();
        verilog << std::endl;
        module.close();
    }
    std::cout << "modules written to " << output_file << std::endl;

    // gather output names
    std::vector<std::string> outputs = get_po_names(ntk);

    // gather input names
    std::vector<std::string> inputs = get_pi_names(ntk);

    // gather wires
    std::set<std::string> wire_set = get_wire_names(partitions);
    std::vector<std::string> wires(wire_set.begin(), wire_set.end());

    std::string output_names = join(", ", outputs);
    std::string input_names = join(", ", inputs);
    std::string wire_names = join(", ", wires);

    std::string name = ntk.get_network_name().size() != 0
                       ? ntk.get_network_name()
                       : "top";
    verilog << fmt::format("module {}({}, {});\n",
                           name, input_names, output_names)
            << fmt::format("  input {};\n", input_names)
            << fmt::format("  output {};\n", output_names);
    if (wires.size() > 0) {
        verilog << fmt::format("  wire {};\n", wire_names);
    }

    // TODO assign constant wires.
    // assign po signal names to po output names.
    std::map<std::string, std::string> assigns = get_assigns(ntk);
    for (const auto& [left, right] : assigns) {
        verilog << "assign " << left << " = " << right << ";\n";
    }

    // write out module instances
    for (int i = 0; i < partitions.get_part_num(); i++) {
        write_child(i, optimized[i], verilog);
    }
    verilog << "endmodule;" << std::endl;
    verilog.close();
    return output_file;
}

template<typename network>
size_t run_timing(std::string liberty_file,
                  std::string verilog_file,
                  mockturtle::names_view<network> ntk,
                  oracle::partition_manager<mockturtle::names_view<network>> partitions,
                  std::vector<optimizer<network>*> optimized)
{
    const char *design = "top";

    sta::Corner *corner = new sta::Corner("tt", 0);
    sta::MinMaxAll *minmax = sta::MinMaxAll::all();
    bool read_ver = sta::readVerilogFile(verilog_file.c_str(),
                                         sta::Sta::sta()->networkReader());
    assert(read_ver); // << "failed to read verilog";
    sta::LibertyLibrary *lib = sta::Sta::sta()->readLiberty(liberty_file.c_str(),
                               corner,
                               minmax,
                               true);
    assert(lib != nullptr);// << "failed to read liberty library";

    bool linked = sta::Sta::sta()->linkDesign(design);
    assert(linked); // << "Failed to link";

    sta::NetworkReader *net = sta::Sta::sta()->networkReader();
    sta::ConcreteInstance *top = reinterpret_cast<sta::ConcreteInstance*>
                                 (net->topInstance());
    sta::Sta::sta()->minPeriodSlack();
    sta::Sta::sta()->minPeriodViolations();
    // get worst path.
    // if meet slack, finish
    return -1;
    // otherwise find partition with most nodes on path
}

template <typename network> mockturtle::names_view<network> budget_optimization(
    mockturtle::names_view<network> &ntk,
    oracle::partition_manager<mockturtle::names_view<network>> &partitions,
    string liberty_file, string output_file, string abc_exec)   // todo use abc_exec
{
    add_default_names(ntk);

    int num_parts = partitions.get_part_num();
    std::vector<optimizer<network>*> optimized(num_parts);
    std::cout << "Finding optimizers." << std::endl;
    for (int i = 0; i < num_parts; i++) {
        std::cout << "partition " << i << std::endl;
        partition_view<mockturtle::names_view<network>> part = partitions.create_part(
                    ntk, i);
        part.set_network_name("partition_" + std::to_string(i));
        optimized[i] = optimize_area(part);
    }
    assert(num_parts == optimized.size());
    string verilog;
    while (true) {
        verilog = techmap(ntk, partitions, optimized, liberty_file);
        std::cout << "Wrote techmapped verilog to " << verilog << std::endl;
        size_t worst_part = run_timing(liberty_file, verilog, ntk, partitions,
                                       optimized);
        // TODO if this is worse than last result, rollback and finish.
        if (worst_part == -1
                || optimized[worst_part]->target() == optimization_strategy::depth) {
            std::cout << "met timing, or it's the best we can do." << std::endl;
            break; // met timing, or it's the best we can do.
        }
        partition_view<mockturtle::names_view<network>> part = partitions.create_part(
                    ntk, worst_part);
        part.set_network_name("partition_" + std::to_string(worst_part));
        optimized[worst_part] = optimize_depth(part);
    }
    // TODO area recovery.
    for (int i = 0; i < num_parts; i++) {
        partition_view<mockturtle::names_view<network>> part = partitions.create_part(
                    ntk, i);
	mockturtle::names_view<network> opt = optimized[i]->reconvert();
        partitions.synchronize_part(part, opt, ntk);
    }
    // TODO copy verilog to output_file.
    partitions.connect_outputs(ntk);
    ntk = mockturtle::cleanup_dangling(ntk);
    return ntk;
}

template mockturtle::names_view<mockturtle::aig_network>
budget_optimization<mockturtle::aig_network>
(
    mockturtle::names_view<mockturtle::aig_network> &,
    oracle::partition_manager<mockturtle::names_view<mockturtle::aig_network>> &,
    std::string, std::string, std::string);

}

#endif
#endif
