#pragma once

#ifdef ENABLE_OPENSTA
#ifdef ENABLE_ABC
#include <mockturtle/mockturtle.hpp>
#include "algorithms/partitioning/partition_manager.hpp"

namespace oracle
{
template <typename network>
mockturtle::names_view<mockturtle::mig_network> budget_optimization(
    mockturtle::names_view<network> ntk,
    oracle::partition_manager<mockturtle::names_view<network>> partitions,
    std::string liberty_file, std::string output_file, std::string abc_exec);
}

#endif
#endif
