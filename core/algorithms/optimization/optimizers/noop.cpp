template <typename network>
class noop: public optimizer<network>
{
public:
    noop(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): index(index), partman(partman), ntk(ntk), strategy(target), abc_exec(abc_exec)
    {
    }

    const std::string optimizer_name()
    {
        return "noop";
    }


    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new noop<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
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
        return mockturtle::node_resynthesis<xmg_names, mockturtle::names_view<network>>
                (copy, resyn);
    }

    /*
     * Do direct resynthesis to create a copy.
     */
    void convert()
    {
        mockturtle::window_view<mockturtle::names_view<network>> original = partman.partition(index);
        // original.set_network_name("partition_" + std::to_string(index)); // TODO not working?
        fix_names(partman, original, ntk, index);
        mockturtle::direct_resynthesis<mockturtle::names_view<network>> resyn;
        copy = mockturtle::node_resynthesis<mockturtle::names_view<network>, mockturtle::window_view<mockturtle::names_view<network>>>
                (original, resyn);
        copy.set_network_name("partition_" + std::to_string(index));
    }

    mockturtle::names_view<network> optimized()
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
    mockturtle::names_view<network> ntk;
    int index;
    partition_manager_junior<network> partman;
    mockturtle::names_view<network> copy;
    node_depth metric;
    optimization_strategy strategy;
    std::string techmapped;
    const std::string &abc_exec;
};

template class noop<mockturtle::aig_network>;
template class noop<mockturtle::mig_network>;
template class noop<mockturtle::xag_network>;
template class noop<mockturtle::xmg_network>;
