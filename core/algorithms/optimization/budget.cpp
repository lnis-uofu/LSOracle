#ifdef ENABLE_STA
#ifdef ENABLE_ABC

#include "budget.hpp"
#include <stdlib.h>

namespace oracle
{
template <typename Ntk>
class test
{
public:
    test(Ntk ntk): original(ntk)
    {
    }

    mig_network convert()
    {
        if (converted != nullptr) {
            /* LUT mapping */
            mapping_view<ntk, true> mapped{ntk};
            lut_mapping_params ps;
            ps.cut_enumeration_ps.cut_size = 4;
            lut_mapping<mapping_view<Ntk, true>, true>(mapped, ps);

            /* collapse into k-LUT network */
            const auto klut = *collapse_mapped_network<klut_network>(mapped);

            /* node resynthesis */
            mig_npn_resynthesis resyn;
            converted = node_resynthesis<mig_network>(klut, resyn);
        }
        return converted;
    }

    void optimize()
    {
        if (optimized != nullptr) {
            oracle::mig_script migopt;
            optimized = migopt.run(converted);
        }
        return optimized;
    }

    node_depth ndp()
    {
        if (metric != nullptr) {
            mockturtle::depth_view part_mig_opt_depth{optimized};
            int mig_opt_size = optimized.num_gates();
            int mig_opt_depth = part_mig_opt_depth.depth();
            metric = node_depth{mig_opt_size, mig_opt_depth};
        }
        return metric;
    }

    std::string techmap(std::string liberty_file)
    {
        if (techmapped != nullptr) {
            char *blif = strdup("/tmp/lsoracle_XXXXXX");
            if (mkstemp(blif) == -1) {
                throw std::exception();
            }
            std::string input_blif = std::string(blif);

            char *verilog = strdup("/tmp/lsoracle_XXXXXX");
            if (mkstemp(verilog) == -1) {
                throw std::exception();
            }
            std::string output_verilog = std::string(verilog);

            char *abc = strdup("/tmp/lsoracle_XXXXXX");
            if (mkstemp(abc) == -1) {
                throw std::exception();
            }
            std::string abc_script = std::string(abc);
            std::ofstream script(abc_script);
            //TODO specify liberty file.
            script << "read_lib " << liberty_file << std::endl;
            script << "strash" << std::endl;
            script << "dch" << std::endl;
            script << "map -B 0.9" << std::endl;
            script << "topo" << std::endl;
            script << "stime -c" << std::endl;
            script << "buffer -c" << std::endl;
            script << "upsize -c" << std::endl;
            script << "dnsize -c" << std::endl;
            script.close();

            mockturtle::write_blif_params ps;
            mockturtle::write_blif(optimized, output_verilog, ps);
            system("abc", "-F", abc_script.c_str(),
                   "-o", output_verilog.c_str(),
                   input_blif.c_str())

            techmapped = output_verilog;
        }
        return techmapped;
    }

    double timing(std::string liberty_file, std::string verilog_file)
    {
        const char *design = "top";

        sta::Sta *test = new sta::Sta;
        sta::Sta::setSta(test);
        sta::initSta();
        test->makeComponents();
        sta::Corner *corner = new sta::Corner("tt", 0);
        sta::MinMaxAll *minmax = sta::MinMaxAll::all();
        bool read_ver = sta::readVerilogFile(verilog_file.c_str(),
                                             test->networkReader());
        assert(read_ver) << "failed to read verilog";
        sta::LibertyLibrary *lib = test->readLiberty(liberty_file.c_str(), corner,
                                   minmax,
                                   true);
        assert(lib != nullptr) << "failed to read liberty library";

        bool linked = test->linkDesign(design);
        assert(linked) << "Failed to link";

        sta::NetworkReader *net = test->networkReader();
        sta::ConcreteInstance *top = reinterpret_cast<sta::ConcreteInstance*>
                                     (net->topInstance());

    }

private:
    Ntk original;
    mockturtle::mig_network optimized;
    mockturtle::mig_network converted;
    node_depth metric;
    string techmapped;
};

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

string combine_verilog(std::vector<string> modules)
{
    throw 42;
}

template <typename network, typename metric>
optimizer<network, metric> optimize_area(partition_view<network> part)
{
    throw 42;
}

template <typename network>
string techmap(
    std::vector<optimizer<network, node_depth>> partitions,
    string liberty_file)
{

}

template <typename network> string optimize(network ntk,
        partition_manager<network> partitions,
        string liberty_file)
{
    int num_parts = partitions.get_part_num();
    std::vector<optimizer<network, node_depth>> optimized(num_parts);
    for (int i = 0; i < num_parts; i++) {
        partition_view<network> part = partitions.create_part(ntk, i);
        optimized[i] = optimize_area(part);
    }
    string verilog = techmap(optimized);
    size_t last_worst = -1;
    while (true) {
        auto result = run_timing(verilog);
        if (result.slack > 0) {
            break;
        }
        size_t worst_part = find_worst(partitions, optimized, result);
        if (worst_part != last_worst) {
            partition_view<network> part = partitions.create_part(ntk, i);
            optimized[worst_part.id()] = optimize_depth(part);
            last_worst = worst_part;
        }
        verilog = techmap(optimized);
    }
    return verilog;
}

mig_names budget_optimization(aig_names ntk_aig, part_man_aig partitions_aig)
{
    auto ntk_mig = *aig_to_mig(ntk_aig, 1);

    partition_manager<mig_names> partitions_mig(ntk_mig,
            partitions_aig.get_all_part_connections(),
            partitions_aig.get_all_partition_inputs(),
            partitions_aig.get_all_partition_outputs(),
            partitions_aig.get_all_partition_regs(),
            partitions_aig.get_all_partition_regin(), partitions_aig.get_part_num());
    return optimize(ntk_mig, partitions_mig);
}
}

#endif ENABLE_ABC
#endif ENABLE_STA
