template <typename network>
class mig_optimizer: public optimizer<network>
{
public:
    mig_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): index(index), partman(partman), ntk(ntk), strategy(target), abc_exec(abc_exec)
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
        mockturtle::window_view<mockturtle::names_view<network>> original = partman.partition(index);
        fix_names(partman, original, ntk, index);

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
    mockturtle::names_view<network> ntk;
    int index;
    partition_manager_junior<network> partman;
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
class migscript_optimizer: public mig_optimizer<network>
{
public:
    migscript_optimizer(partition_manager_junior<network> partman, mockturtle::names_view<network> ntk, int index, optimization_strategy target, const std::string &abc_exec): mig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    const std::string optimizer_name()
    {
        return "migscript";
    }

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        std::cout << "reapply " << this->index << std::endl;
        return new migscript_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }
    void optimize()
    {
        oracle::mig_script migopt;
        this->optimal = migopt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};

template <typename network>
class migscript2_optimizer: public mig_optimizer<network>
{
public:
    migscript2_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): mig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    const std::string optimizer_name()
    {
        return "migscript2";
    }

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new migscript2_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }
    void optimize()
    {
        oracle::mig_script2 migopt;
        this->optimal = migopt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};

template <typename network>
class migscript3_optimizer: public mig_optimizer<network>
{
public:
    migscript3_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): mig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new migscript3_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "migscript3";
    }

    void optimize()
    {
        oracle::mig_script3 migopt;
        this->optimal = migopt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};
