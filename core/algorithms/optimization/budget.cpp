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
#include <sta/VerilogReader.hh>
#include <sta/StaMain.hh>
namespace sta {
extern const char *tcl_inits[];
}
extern "C" {
extern int Sta_Init(Tcl_Interp *interp);
}


#include <fmt/format.h>
#include <tcl.h>
#include <regex>
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
	typename network::node n = part.get_node(s);
	std::string name = get_node_name_or_default(ntk, n);
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
        // original.set_network_name("partition_" + std::to_string(index)); // TODO not working?
	fix_names(partman, original, ntk, index);
	mockturtle::direct_resynthesis<mockturtle::names_view<Ntk>> resyn;
        copy = mockturtle::node_resynthesis<mockturtle::names_view<Ntk>, partition_view<mockturtle::names_view<Ntk>>>
                (original, resyn);
	copy.set_network_name("partition_" + std::to_string(index));

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
        return basic_techmap(script, copy);
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
	// Compare on basis of roll number
	node_depth x = a.independent_metric();
	node_depth y = b.independent_metric();

        return x.nodes * x.depth < y.nodes * y.depth;
    }
    const string name()
    {
	return "node-depth product";
    }
};

// TODO generic combine this with depth
template <typename network>
optimizer<network> *optimize(optimization_strategy_comparator<network> &comparator,
			     partition_manager<mockturtle::names_view<network>> &partman,
			     mockturtle::names_view<network> &ntk,
			     int index)

{
    std::cout << "Optimizing based on strategy " << comparator.name() << std::endl;
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

        if (comparator(**opt, *best)) {
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
		  // TODO sdc file
                  mockturtle::names_view<network> &ntk,
                  oracle::partition_manager<mockturtle::names_view<network>> &partitions,
                  std::vector<optimizer<network>*> &optimized)
{
    const std::string design = ntk.get_network_name() != "" ? ntk.get_network_name() : "top";
    sta::Corner *corner = new sta::Corner("tt", 0);
    sta::MinMaxAll *minmax = sta::MinMaxAll::all();

    sta::LibertyLibrary *lib = sta::Sta::sta()->readLiberty(liberty_file.c_str(),
                               corner,
                               minmax,
                               true);
    assert(lib != nullptr);// << "failed to read liberty library"
    bool read_ver = sta::readVerilogFile(verilog_file.c_str(),
    // bool read_ver = sta::readVerilogFile("/home/snelgrov/code/lsoracle/benchmarks/picorv32/picorv32_lsoracle.mapped.v",
                                         sta::Sta::sta()->networkReader());
    assert(read_ver); // << "failed to read verilog";
    bool linked = sta::Sta::sta()->linkDesign(design.c_str());
    assert(linked); // << "Failed to link";

    std::cout << "Attempting to read sdc" << std::endl;
    int a = Tcl_Eval(sta::Sta::sta()->tclInterp(), "sta::read_sdc /home/snelgrov/code/lsoracle/dummy.sdc");
    assert(a == 0);
    int b = Tcl_Eval(sta::Sta::sta()->tclInterp(), "sta::report_checks > /tmp/test.monkey");
    assert(b == 0);

    // int d = Tcl_Eval(sta::Sta::sta()->tclInterp(), "puts \"hello world!\"");

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

    if (worst_path_slack.slack(sta::Sta::sta()) >= 0.0) {
	return -1;
    }

    std::vector<float> budget(partitions.get_part_num(), 0.0);
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
    int max = -1;
    float worst = 0;
    for (int i = 0; i < partitions.get_part_num(); i++) {
	if (budget[i] > worst) {
	    worst = budget[i];
	    max = i;
	}
    }
    // sta::ConcreteNetwork *net = reinterpret_cast<sta::ConcreteNetwork*>(sta::Sta::sta()->networkReader());
    // sta::ConcreteInstance *top = reinterpret_cast<sta::ConcreteInstance*>
    //                              (net->topInstance());
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

template <typename network> mockturtle::names_view<network> budget_optimization(
    mockturtle::names_view<network> &ntk,
    oracle::partition_manager<mockturtle::names_view<network>> &partitions,
    const string &liberty_file, const string &output_file, const string &abc_exec)   // todo use abc_exec
// todo clock signal name
{
    int num_parts = partitions.get_part_num();
    std::vector<optimizer<network>*> optimized(num_parts);
    std::cout << "Finding optimizers." << std::endl;
    for (int i = 0; i < num_parts; i++) {
        std::cout << "partition " << i << std::endl;
	ndp_strategy<network> strategy;
        optimized[i] = optimize(strategy, partitions, ntk, i);
    }
    assert(num_parts == optimized.size());

    string verilog;
    while (true) {
	//reset_sta(); // todo not cleaning up
        verilog = techmap(ntk, partitions, optimized, abc_exec, liberty_file);
        std::cout << "Wrote techmapped verilog to " << verilog << std::endl;
        size_t worst_part = run_timing(liberty_file, verilog, ntk, partitions,
                                       optimized);
        // TODO if this is worse than last result, rollback and finish.
        if (worst_part == -1) {
	    std::cout << "met timing" << std::endl;
	    break;
	}
	if (optimized[worst_part]->target() == optimization_strategy::depth) {
            std::cout << "previous result was already the best we can do." << std::endl;
            break; // met timing, or it's the best we can do.
        }
        optimized[worst_part] = optimize_depth(partitions, ntk, worst_part);
    }
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
