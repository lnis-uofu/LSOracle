#ifdef ENABLE_OPENSTA
#ifdef ENABLE_ABC

#include "algorithms/optimization/budget.hpp"
#include <stdlib.h>
#include <mockturtle/mockturtle.hpp>
#include "algorithms/partitioning/partition_manager.hpp"
#include "algorithms/optimization/mig_script.hpp"
#include "algorithms/optimization/aig_script.hpp"
#include "utility.hpp"
#include <sta/Sta.hh>
#include <sta/Network.hh>
#include <sta/Corner.hh>
#include <sta/VerilogReader.hh>
#include <sta/Liberty.hh>
#include <sta/ConcreteNetwork.hh>
#include <sta/PortDirection.hh>

namespace oracle
{
using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using aig_ntk = std::shared_ptr<aig_names>;
using part_man_aig = oracle::partition_manager<aig_names>;
using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using mig_ntk = std::shared_ptr<mig_names>;
using part_man_mig = oracle::partition_manager<mig_names>;
using part_man_mig_ntk = std::shared_ptr<part_man_mig>;


struct node_depth {
    int nodes;
    int depth;
};

enum optimization_strategy { area, balanced, depth };

template<typename network>
class optimizer
{
public:
    optimizer(partition_view<network> p);
    void convert();
    /**
     * Perform optimization
     */
    void optimize();
    node_depth independent_metric();
    double slack();
    network reconvert();
    optimization_strategy target();
    /**
     * Return a path to a file containing the techmapped verilog.
     */
    template<typename T>
    std::string basic_techmap(string tech_script, T optimal)
    {
        char *blif = strdup("/tmp/lsoracle_XXXXXX");
        assert(mkstemp(blif) != -1);
        std::string input_blif = std::string(blif);

        char *verilog = strdup("/tmp/lsoracle_XXXXXX");
        assert(mkstemp(verilog) != -1);
        std::string output_verilog = std::string(verilog);

        char *abc = strdup("/tmp/lsoracle_XXXXXX");
        assert(mkstemp(abc) != -1);
        std::string abc_script = std::string(abc);

        std::ofstream script(abc_script);
        script << tech_script << std::endl;
        script.close();

        mockturtle::write_blif_params ps;
        mockturtle::write_blif(optimal, output_verilog, ps);
        system(("abc -F " + abc_script + " -o " + output_verilog + " " +
                input_blif).c_str());
        return output_verilog;
    };
};

template <typename Ntk>
class noop: public optimizer<Ntk>
{
public:
    noop(mockturtle::names_view<Ntk> ntk): original(ntk)
    {
    }

    optimization_strategy target()
    {
        return optimization_strategy::balanced;
    }

    mockturtle::names_view<Ntk> reconvert()
    {
        return original;
    }

    void convert()
    {
    }

    mockturtle::names_view<Ntk> optimized()
    {
        return original;
    }

    void optimize()
    {
    }

    std::string techmap(std::string liberty_file)
    {
        string script =
            "read_lib " + liberty_file +
            "; strash; dch; map -B 0.9; topo; stime -c; buffer -c; upsize -c; dnsize -c";
        techmapped = basic_techmap(script, original);
        return techmapped;
    }

    node_depth independent_metric()
    {
        mockturtle::depth_view part_depth{original};
        int opt_size = original.num_gates();
        int opt_depth = part_depth.depth();
        metric = node_depth{opt_size, opt_depth};
        return metric;
    }


private:
    mockturtle::names_view<Ntk> original;
    node_depth metric;
    string techmapped;
};

template <typename Ntk>
class test: public optimizer<Ntk>
{
public:
    test(mockturtle::names_view<Ntk> ntk): original(ntk)
    {
    }

    optimization_strategy target()
    {
        return optimization_strategy::depth;
    }

    mockturtle::names_view<Ntk> reconvert()
    {
        return optimal;
    }

    void convert()
    {
        /* LUT mapping */
        mockturtle::mapping_view<mockturtle::names_view<Ntk>, true> mapped{original};
        mockturtle::lut_mapping_params ps;
        ps.cut_enumeration_ps.cut_size = 4;
        mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::names_view<Ntk>, true>, true>
        (mapped, ps);

        /* collapse into k-LUT network */
        const auto klut =
            *mockturtle::collapse_mapped_network<mockturtle::klut_network>(mapped);

        /* node resynthesis */
        mockturtle::mig_npn_resynthesis resyn;
        converted = mockturtle::node_resynthesis<mockturtle::mig_network>(klut, resyn);
    }

    mockturtle::mig_network optimized()
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
        techmapped = this->basic_techmap(script, optimal);
        return techmapped;
    }

private:
    mockturtle::names_view<Ntk> original;
    mockturtle::names_view<mockturtle::mig_network> optimal;
    mockturtle::names_view<mockturtle::mig_network> converted;
    node_depth metric;
    string techmapped;
};
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
optimizer<network> optimize_by(partition_view<network> part,
                               std::vector<optimizer<network>> optimizers, F better)
{
    optimizer<network> best = noop<network>(part);
    for (auto opt = optimizers.begin(); opt != optimizers.end(); opt++) {
        opt.convert();
        opt.optimize();
        if (better(best.independent_metric(), opt.indepent_metric())) {
            best = opt;
        }
    }
    return best;
}

template <typename network>
optimizer<network> optimize_depth(partition_view<network> part)
{
    std::vector<optimizer<network>> optimizers;
    optimizers.append(test<network>(part));
    return optimize_by(part, optimizers, [](node_depth prev, node_depth next) {
        return next.depth < prev.depth;
    });
}

template <typename network>
optimizer<network> optimize_area(partition_view<network> part)
{
    std::vector<optimizer<network>> optimizers;
    optimizers.append(test<network>(part));
    return optimize_by(part, optimizers, [](node_depth prev, node_depth next) {
        return next.nodes < prev.nodes;
    });
}

template <typename network>
string techmap(
    mockturtle::names_view<network> ntk,
    oracle::partition_manager<mockturtle::names_view<network>> partitions,
    std::vector<optimizer<network>> optimized,
    string liberty_file)
{
    char *output = strdup("/tmp/lsoracle_XXXXXX");
    assert(mkstemp(output) != -1);
    std::string output_file = std::string(output);

    std::ofstream verilog(abc_script);
    verilog << "# Generated by LSOracle" << std::endl;

    // add all partition modules to verilog file.
    int num_parts = partitions.get_part_num();
    for (int i = 0; i < num_parts; i++) {
        std::string module_file = optimized[i].techmap(liberty_file);
        std::ifstream module(module_file);
        verilog << module.rdbuf();
        verilog << std::endl;
        module.close();
    }
    // TODO what about registers?
    verilog << "module " << ntk.get_network_name() << "(";
    ntk.foreach_pi([&verilog](network::signal n) {
        verilog << ntk.get_name(n) << "," << std::endl;
    });
    ntk.foreach_po([&verilog](network::node n) {
        verilog << ntk.get_output_name(n) << "," << std::endl;
    });
    verilog.seekp(verilog.tellp() - 2); // Truncate last comma
    verilog << std::endl << ");" << std::endl;

    ntk.foreach_pi([&verilog](network::signal n) {
        verilog << "input " << ntk.get_name(n) << ";" << std::endl;
    });
    ntk.foreach_po([&verilog](network::node n) {
        verilog << "output " << ntk.get_output_name(n) << ";" << std::endl;
    });

    // todo write wires

    // write instances.
    for (int i = 0; i < num_parts; i++) {
        // TODO these need to be the same as the techmapped.
        verilog << "partition_" << i << " partition_" << i << "_inst (" << std::endl;
        oracle::partition_view<mockturtle::names_view<network>> part =
                    partitions.create_part(ntk, i);
        ntk.foreach_pi([&verilog](network::signal n) {
            std::string name = "TODO"; // TODO
            verilog << "." << name << "(" << name << ")," << std::endl;
        });
        ntk.foreach_po([&verilog](network::node n) {
            std::string name = "TODO"; // TODO
            verilog << "." << name << "(" << name << ")," << std::endl;
        });
        verilog.seekp(verilog.tellp() - 2); // Truncate last comma
        verilog << std::endl << ");" << std::endl;
    }

    /*
      TODO

      identify all connections between partitions, create a wire for each connection.
    { has_name(signal), get_name(signal) }
      reference all partition modules.
     */
    verilog << "endmodule;" << std::endl;
    verilog.close();
}

template<typename network>
size_t run_timing(std::string liberty_file,
                  std::string verilog_file,
                  mockturtle::names_view<network> ntk,
                  oracle::partition_manager<mockturtle::names_view<network>> partitions,
                  std::vector<optimizer<network>> optimized)
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
    // TODO
    throw 0;
}

template <typename network> mig_names optimize(
    mockturtle::names_view<network> ntk,
    oracle::partition_manager<mockturtle::names_view<network>> partitions,
    string liberty_file, string output_file)
{
    int num_parts = partitions.get_part_num();
    std::vector<optimizer<network>> optimized(num_parts);
    for (int i = 0; i < num_parts; i++) {
        partition_view<network> part = partitions.create_part(ntk, i);
        optimized[i] = optimize_area(part);
    }
    string verilog;
    while (true) {
        verilog = techmap(ntk, partitions, optimized, liberty_file);
        size_t worst_part = run_timing(liberty_file, verilog, ntk, partitions,
                                       optimized);
        if (worst_part == -1
                || optimized[worst_part].target() == optimization_strategy::depth) {
            break; // met timing, or it's the best we can do.
        }
        partition_view<network> part = partitions.create_part(ntk, worst_part);
        optimized[worst_part] = optimize_depth(part);
    }
    // TODO area recovery.
    for (int i = 0; i < num_parts; i++) {
        partition_view<network> part = partitions.create_part(ntk, i);
        network opt = optimized[i].reconvert();
        partitions.synchronize_part(part, opt, ntk);
    }
    // TODO copy verilog to output_file.
    partitions.connect_outputs(ntk);
    ntk = mockturtle::cleanup_dangling(ntk);
    return ntk;
}
}

#endif
#endif
