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
    using mig_names = mockturtle::names_view<mockturtle::mig_network>;
template<typename T>
std::string basic_techmap(const std::string &tech_script, const T &optimal)
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
template std::string basic_techmap<aig_names>(const std::string &, const aig_names &);
template std::string basic_techmap<mig_names>(const std::string &, const mig_names &);

template <typename network> std::string get_po_name_or_default(const network &ntk, const typename network::signal &signal)
{
    int index = ntk.po_index(signal);

    if (ntk.has_output_name(index)) {
	return ntk.get_output_name(index);
    } else {
	int digits_out = std::to_string(ntk.num_pos()).length();
	std::cout << "missing output name for index " << index << std::endl;
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
	std::cout << "missing name for PI node " << node << std::endl;
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
	    return ntk.get_name(signal);
	} else {
	    std::cout << "missing name for non-PI node " << node << std::endl;
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
	std::cout << "missing name for RI node " << node << std::endl;
	int digits_in = std::to_string(ntk.num_registers()).length();
	return fmt::format("ri__{0:0{1}}", node, digits_in);
    }
}

template <typename network> void fix_names(partition_manager<mockturtle::names_view<network>> &partman,
				       partition_view<mockturtle::names_view<network>> &part,
				       const mockturtle::names_view<network> &ntk,
				       int index)
{
    part.foreach_pi([&part, &ntk](typename network::node n) {
	std::string name = get_node_name_or_default(ntk, n);
	part.set_name(part.make_signal(n), name);
    });
    part.foreach_po([&part, &ntk](typename network::signal s, int i) {
	std::string name = get_po_name_or_default(ntk, s);
	part.set_output_name(i, name);
    });
}

template <typename Ntk>
class noop: public optimizer<Ntk>
{
public:
    noop(partition_manager<mockturtle::names_view<Ntk>> partman, mockturtle::names_view<Ntk> ntk, int index): index(index), partman(partman), ntk(ntk)
    {
    }

    std::string optimizer_name()
    {
        return "noop";
    }

    partition_view<mockturtle::names_view<Ntk>> partition()
    {
	return partman.create_part(ntk, index);
    }

    std::string module_name()
    {
        return copy.get_network_name();
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
	partition_view<mockturtle::names_view<Ntk>> original = partman.create_part(ntk, index);
        original.set_network_name("partition_" + std::to_string(index));
	fix_names(partman, original, ntk, index);
	mockturtle::direct_resynthesis<mockturtle::names_view<Ntk>> resyn;
        copy = mockturtle::node_resynthesis<mockturtle::names_view<Ntk>, partition_view<mockturtle::names_view<Ntk>>>
                (original, resyn);
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
        int opt_size = part_depth.num_gates();
        int opt_depth = part_depth.depth();
        metric = node_depth{opt_size, opt_depth};
        return metric;
    }
private:
    mockturtle::names_view<Ntk> ntk;
    int index;
    partition_manager<mockturtle::names_view<Ntk>> partman;
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
optimizer<network> *optimize_depth(partition_manager<mockturtle::names_view<network>> partman,
				   mockturtle::names_view<network> ntk,
				   int index)
{
    std::cout << "Optimizing for depth" << std::endl;
    // todo this is gonna leak memory.
    std::vector<optimizer<network>*> optimizers;
    optimizers.emplace_back(new noop<network>(partman, ntk, index));
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

// TODO generic combine this with depth
template <typename network>
optimizer<network> *optimize_area(
				  partition_manager<mockturtle::names_view<network>> partman,
				  mockturtle::names_view<network> ntk,
				  int index)

{
    std::cout << "Optimizing for area" << std::endl;
    // todo this is gonna leak memory.
    std::vector<optimizer<network>*> optimizers;
    optimizers.emplace_back(new noop<network>(partman, ntk, index));
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
std::set<std::string> get_wire_names(
				     oracle::partition_manager<mockturtle::names_view<network>> &partitions,
				     mockturtle::names_view<network> &ntk)
{
    std::set<std::string> wires;
    int num_parts = partitions.get_part_num();
    ntk.foreach_register([&ntk, &wires](std::pair<typename network::signal, typename network::node> reg) {
	typename network::signal ri = reg.first;
	typename network::node ro = reg.second;
	wires.insert(get_node_name_or_default(ntk, ro));
	wires.insert(get_ri_name_or_default(ntk, ri));
    });
    for (int i = 0; i < num_parts; i++) {
	partition_view<mockturtle::names_view<network>> part = partitions.create_part(ntk, i);
	part.foreach_pi([&ntk, &wires](typename network::node n) {
	    if (!ntk.is_pi(n) && !ntk.is_constant(n)) {
		wires.insert(get_node_name_or_default(ntk, n));
	    }
	});

	part.foreach_po([&ntk, &wires](typename network::signal s) {
	    typename network::node n = ntk.get_node(s);
	    if (!ntk.is_pi(n) && !ntk.is_constant(n)) {
		wires.insert(get_node_name_or_default(ntk, n));
	    }
	});
    }
    return wires;
}

template <typename network>
void write_child(int index,
		 partition_manager<mockturtle::names_view<network>> &partman,
		 mockturtle::names_view<network> &ntk,
                 std::ofstream &verilog,
		 optimizer<network> *optimizer)
{
    std::cout << "writing out instance for partition " << index << std::endl;
    // verilog << optimize->module_name() << " " optimize->module_name() << "_inst (\n";
    verilog << "partition_" << index << " partition_" << index << "_inst (\n";

    partition_view<mockturtle::names_view<network>> part = partman.create_part(ntk, index);

    std::cout << "gathering pi names." << std::endl;
    part.foreach_pi([&part, &ntk, &verilog](typename network::node n) {
	std::string driver = get_node_name_or_default(ntk, n);
	verilog << "." << driver << "(" << driver << "),\n";
    });

    std::cout << "gathering po names." << std::endl;
    part.foreach_po([&part, &ntk, &verilog](typename network::signal s, auto i) {
	typename network::node n = ntk.get_node(s);
	std::string driver = get_node_name_or_default(ntk, n);
	verilog << "." << driver << "(" << driver << "),\n";
    });
    verilog.seekp(verilog.tellp() - 2L); // Truncate last comma
    verilog << "\n);\n" << std::endl;
}
void write_inverter(std::string abc_exec, std::string liberty, std::ofstream &verilog)
{
    // TODO techmap inverter
    verilog << "module mapped_register(D, Q, CLK);\n"
	    << "input D, CLK;\n"
	    << "output Q;\n"
	    << "sky130_fd_sc_hd__dfxtp_1 x (.CLK(CLK), .D(D), .Q(Q));\n"
	    << "endmodule\n" << std::endl;
}

void write_register(std::string abc_exec, std::string liberty, std::ofstream &verilog)
{
    // TODO techmap register
    verilog << "module mapped_inverter(A, Y);\n"
	    << "input A; output Y;\n"
	    << "sky130_fd_sc_hd__inv_1 i(.A(A), .Y(Y));\n"
	    << "endmodule\n" << std::endl;
}

template <typename network>
string techmap(
    mockturtle::names_view<network> ntk,
    oracle::partition_manager<mockturtle::names_view<network>> partitions,
    std::vector<optimizer<network>*> optimized,
    string abc_exec,
    string liberty_file)
{
    std::cout << "Starting techmap." << std::endl;
    // Write out verilog
    char *output = strdup("/tmp/lsoracle_XXXXXX.combined.v");
    if (mkstemps(output, 11) == -1) {
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
    std::cout << "sub-modules written" << std::endl;
    write_register(abc_exec, liberty_file, verilog);
    write_inverter(abc_exec, liberty_file, verilog);
    write_top(ntk, partitions, optimized, verilog);
    verilog.close();
    return output_file;

}

template <typename network>
void write_top(mockturtle::names_view<network> &ntk,
	       oracle::partition_manager<mockturtle::names_view<network>> &partitions,
	       std::vector<optimizer<network>*> &optimized,
	       std::ofstream &verilog)
{
    std::string name = ntk.get_network_name().size() != 0
                       ? ntk.get_network_name()
                       : "top";

    // gather output names
    std::vector<std::string> outputs;
    std::cout << "gathering po names." << std::endl;
    ntk.foreach_po([&outputs, &ntk](typename network::signal signal) {
	outputs.emplace_back(get_po_name_or_default(ntk, signal));
    });
    std::string output_names = join(", ", outputs);

    // gather input names
    std::vector<std::string> inputs;
    std::cout << "gathering pi names." << std::endl;
    ntk.foreach_pi([&inputs, &ntk](typename network::node node) {
	inputs.emplace_back(get_pi_name_or_default(ntk, node));
    });
    std::string input_names = join(", ", inputs);

    // gather wires
    std::set<std::string> wire_set = get_wire_names(partitions, ntk);
    std::vector<std::string> wires(wire_set.begin(), wire_set.end());
    std::string wire_names = join(", ", wires);

    verilog << fmt::format("module {}({}, {});\n",
                           name, input_names, output_names)
            << fmt::format("  input {};\n", input_names)
            << fmt::format("  output {};\n", output_names);
    if (wires.size() > 0) {
        verilog << fmt::format("  wire {};\n", wire_names);
    }

    // TODO assign constant wires.

    // generate registers.
    // TODO pass clock name in
    std::string clock = "clk";
    ntk.foreach_register([&ntk, &clock, &verilog](std::pair<typename network::signal, typename network::node> reg) {
	typename network::signal ri = reg.first;
	typename network::node ro = reg.second;
	std::string output = get_node_name_or_default(ntk, ro);
	std::string input = get_ri_name_or_default(ntk, ri);
	verilog << "mapped_register " << output << "_reg(.D(" << input << "), .Q(" << output << "), .CLK(" << clock << "))\n";
    });
    verilog << std::endl;

    // assign PO signal names to driver nodes. if complemented, create an inverter.
    ntk.foreach_po([&ntk, &verilog](typename network::signal signal) {
	std::string output = get_po_name_or_default(ntk, signal);
	std::string driver = get_node_name_or_default(ntk, ntk.get_node(signal));

        if (output != driver) {
	    if (ntk.is_complemented(signal)) {
		verilog << "mapped_inverter " << output << "_inv(.A(" << driver << "), .Y(" << output << "));\n";
	    } else {
		verilog << "assign " << output << " = " << driver << ";\n";
	    }
        }
    });
    ntk.foreach_register([&ntk, &verilog](std::pair<typename network::signal, typename network::node> reg) {
	typename network::signal ri = reg.first;
	std::string output = get_ri_name_or_default(ntk, ri);
	std::string driver = get_node_name_or_default(ntk, ntk.get_node(ri));

        if (output != driver) {
	    if (ntk.is_complemented(ri)) {
		verilog << "mapped_inverter " << output << "_inv(.A(" << driver << "), .Y(" << output << "));\n";
	    } else {
		verilog << "assign " << output << " = " << driver << ";\n";
	    }
        }
    });
    verilog << std::endl;

    // write out module instances
    for (int i = 0; i < partitions.get_part_num(); i++) {
        write_child(i, partitions, ntk, verilog, optimized[i]);
    }
    verilog << "endmodule" << std::endl;
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
    const string &liberty_file, const string &output_file, const string &abc_exec)   // todo use abc_exec
{
    int num_parts = partitions.get_part_num();
    std::vector<optimizer<network>*> optimized(num_parts);
    std::cout << "Finding optimizers." << std::endl;
    for (int i = 0; i < num_parts; i++) {
        std::cout << "partition " << i << std::endl;
        optimized[i] = optimize_area(partitions, ntk, i);
    }
    assert(num_parts == optimized.size());
    string verilog;
    while (true) {
        verilog = techmap(ntk, partitions, optimized, abc_exec, liberty_file);
        std::cout << "Wrote techmapped verilog to " << verilog << std::endl;
        size_t worst_part = run_timing(liberty_file, verilog, ntk, partitions,
                                       optimized);
        // TODO if this is worse than last result, rollback and finish.
        if (worst_part == -1
                || optimized[worst_part]->target() == optimization_strategy::depth) {
            std::cout << "met timing, or it's the best we can do." << std::endl;
            break; // met timing, or it's the best we can do.
        }
        optimized[worst_part] = optimize_depth(partitions, ntk, worst_part);
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
    const std::string &, const std::string &, const std::string &);
}

#endif
#endif
