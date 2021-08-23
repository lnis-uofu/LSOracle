#pragma once

#ifdef ENABLE_STA
#ifdef ENABLE_ABC

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

mig_names budget_optimization(aig_names ntk_aig, part_man_aig partitions_aig,
                              string liberty_file);
}

template<typename network, typename metric>
class optimizer
{
public:
    optimizer(partition_view<network> p): part(network) {}

    /**
     * Perform optimization
     */
    network optimized()
    {
        if (result == nullptr) {
            result = optimize();
        }
        return result[0];
    }

    metric independent_metric()
    {
        if (result == nullptr) {
            result = optimize();
        }
        return result[1];
    }

    /**
     * Return a path to a file containing the techmapped verilog.
     */
    string techmap();
    int slack();
protected:
    std::tuple<network, metric> optimize(partition_view<network> part);
private:
    partition_view<network> part;
    std::tuple<network, metric> *result;
};

struct node_depth {
    int node;
    int depth;
};


#endif ENABLE_ABC
#endif ENABLE_STA
