template <typename network>
class xag_optimizer: public optimizer<network>
{
public:
    xag_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): index(index), partman(partman), ntk(ntk), strategy(target), abc_exec(abc_exec)
    {
    }

    xmg_names export_superset()
    {
        mockturtle::direct_resynthesis<xmg_names> resyn;
        return mockturtle::node_resynthesis<xmg_names, xag_names>
                (optimal, resyn);
    }

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new xag_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }
    void convert()
    {
        mockturtle::window_view<mockturtle::names_view<network>> original = partman.partition(index);
        fix_names(partman, original, ntk, index);

        mockturtle::xag_npn_resynthesis<xag_names> resyn;
        converted =
            mockturtle::node_resynthesis<xag_names, mockturtle::window_view<mockturtle::names_view<network>>>
            (original, resyn);
        converted.set_network_name("partition_" + std::to_string(index));
        assert(converted.get_network_name() == "partition_" + std::to_string(index));
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
    mockturtle::names_view<network> ntk;
    int index;
    partition_manager_junior<network> partman;
    xag_names optimal;
    xag_names converted;
    node_depth metric;
    string techmapped;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class xag_optimizer<mockturtle::xag_network>;
