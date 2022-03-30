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
#include "algorithms/optimization/resynthesis.hpp"
#include "algorithms/partitioning/slack_view.hpp"
#include "utility.hpp"
#include "algorithms/optimization/optimizers.hpp"

// TODO replace "pi/po" with "ci/co"
namespace oracle
{
using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using xag_names = mockturtle::names_view<mockturtle::xag_network>;
using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
using xmg_manager = partition_manager_junior<mockturtle::xmg_network>;
using xmg_partition = mockturtle::window_view<mockturtle::names_view<mockturtle::xmg_network>>;

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

string join(std::string delim, std::set<string> input)
{
    std::vector<std::string> data(input.begin(), input.end());
    if (input.size() == 0) {
        return "";
    } else if (input.size() == 1) {
        return *data.begin();
    } else {
	std::stringstream ss;
        for (auto i = data.begin(); i != (data.end() - 1); i++) {
            ss << *i << delim;
        }
        ss << *(data.end() - 1);
        return ss.str();
    }
}


template <typename network>
std::set<std::string> get_wire_names(oracle::partition_manager_junior<network> &partitions,
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

template <typename network>
string techmap(
    oracle::partition_manager_junior<network> partitions,
    std::vector<optimizer<network>*> optimized,
    const string &abc_exec,
    const string &liberty_file,
    const string &mappings_file,
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

    verilog << "// Mappings" << std::endl;
    std::ifstream mappings(mappings_file);
    verilog << mappings.rdbuf();
    verilog << std::endl;
    mappings.close();

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
template <typename network> xmg_names optimize_timing(
    oracle::partition_manager_junior<network> &partitions,
    const string &liberty_file, const std::string &mapping_file,
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
        const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partitions, i);
        optimized[i] = optimize(strategy, optimization_strategy::size, part, i, abc_exec);
    }
    assert(num_parts == optimized.size());

    string verilog;
    while (true) {
        //reset_sta(); // todo not cleaning up
        verilog = techmap(partitions, optimized, abc_exec, liberty_file, mapping_file, clock, temp_prefix);
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
        if (worst_part == -2) {
            std::cout << "exhausted depth optimization for critical path" << std::endl;
            break;
        }
        if (optimized[worst_part]->target() == optimization_strategy::size) {
            ndp_strategy<network> strategy;
            const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partitions, worst_part);
            optimized[worst_part] = optimize(strategy, optimization_strategy::balanced, part, worst_part, abc_exec);
        } else if (optimized[worst_part]->target() == optimization_strategy::balanced) {
            d_strategy<network> strategy;
            const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partitions, worst_part);

            optimized[worst_part] = optimize(strategy, optimization_strategy::depth, part, worst_part, abc_exec);
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
template <typename network> xmg_names optimize_resynthesis(
    oracle::partition_manager_junior<network> &partitions, const string &abc_exec)
{
    int num_parts = partitions.count();
    std::vector<optimizer<network>*> optimized(num_parts);
    for (int i = 0; i < num_parts; i++) {
        n_strategy<network> strategy;
        const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partitions, i);

        optimized[i] = optimize(strategy, optimization_strategy::size, part, i, abc_exec);
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
        if (worst_part == -2) {
            std::cout << "exhausted depth optimization for critical path" << std::endl;
            break;
        }
        if (optimized[worst_part]->target() == optimization_strategy::size) {
            ndp_strategy<network> strategy;
            const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partitions, worst_part);

            optimized[worst_part] = optimize(strategy, optimization_strategy::balanced, part, worst_part, abc_exec);
        } else if (optimized[worst_part]->target() == optimization_strategy::balanced) {
            d_strategy<network> strategy;
            const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partitions, worst_part);
            optimized[worst_part] = optimize(strategy, optimization_strategy::depth, part, worst_part, abc_exec);
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
xmg_names optimize_basic (
    oracle::partition_manager_junior<network> &partitions,
    const string &abc_exec,
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
            const mockturtle::window_view<mockturtle::names_view<network>> part = fix_names2(partitions, i);

    optimized[i] = optimize(*target, strategy, part, i, abc_exec);
  }
  delete target;
  assert(num_parts == optimized.size());

  return setup_output(partitions, optimized);
}

/**************** Template instances ****************/
template xmg_names
optimize_timing<mockturtle::aig_network>
(
    oracle::partition_manager_junior<mockturtle::aig_network> &,
    const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);

template xmg_names
optimize_basic<mockturtle::aig_network>
(
    oracle::partition_manager_junior<mockturtle::aig_network> &,
    const std::string &,
    const optimization_strategy);

template xmg_names
optimize_resynthesis<mockturtle::aig_network>
(
    oracle::partition_manager_junior<mockturtle::aig_network> &,
    const std::string &);

}
#endif
#endif
