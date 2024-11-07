template <typename network>
class aig_optimizer: public optimizer<network>
{
public:
    aig_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): index(index), partman(partman), ntk(ntk), strategy(target), abc_exec(abc_exec)
    {
    }

    xmg_names export_superset()
    {
        mockturtle::direct_resynthesis<xmg_names> resyn;
        return mockturtle::node_resynthesis<xmg_names, aig_names>(optimal, resyn);
    }

    void convert()
    {
        mockturtle::window_view<mockturtle::names_view<network>> original = partman.partition(index);
        fix_names(partman, original, ntk, index);

        mockturtle::xag_npn_resynthesis<aig_names> resyn;
        converted =
            mockturtle::node_resynthesis<aig_names, mockturtle::window_view<mockturtle::names_view<network>>>
            (original, resyn);
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
    mockturtle::names_view<network> ntk;
    int index;
    partition_manager_junior<network> partman;
    aig_names optimal;
    aig_names converted;
    node_depth metric;
    string techmapped;
    optimization_strategy strategy;
    const std::string &abc_exec;
};
template class aig_optimizer<mockturtle::aig_network>;

template <typename network>
class aigscript_optimizer: public aig_optimizer<network>
{
public:
    aigscript_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new aigscript_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript";
    }

    void optimize()
    {
        oracle::aig_script opt;
        this->optimal = opt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};

template <typename network>
class aigscript2_optimizer: public aig_optimizer<network>
{
public:
    aigscript2_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new aigscript2_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript2";
    }

    void optimize()
    {
        oracle::aig_script2 opt;
        this->optimal = opt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};

template <typename network>
class aigscript3_optimizer: public aig_optimizer<network>
{
public:
    aigscript3_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new aigscript3_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript3";
    }

    void optimize()
    {
        oracle::aig_script3 opt;
        this->optimal = opt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};

template <typename network>
class aigscript4_optimizer: public aig_optimizer<network>
{
public:
    aigscript4_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new aigscript4_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript4";
    }

    void optimize()
    {
        oracle::aig_script4 opt;
        this->optimal = opt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};

template <typename network>
class aigscript5_optimizer: public aig_optimizer<network>
{
public:
    aigscript5_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new aigscript5_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }

    const std::string optimizer_name()
    {
        return "aigscript5";
    }

    void optimize()
    {
        oracle::aig_script5 opt;
        this->optimal = opt.run(this->converted);
        assert(this->optimal.get_network_name() == this->converted.get_network_name());

    }
};
