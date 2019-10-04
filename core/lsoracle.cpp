#if !defined ALICE_PYTHON
#define ALICE_SETTINGS_WITH_DEFAULT_OPTION true
#endif

#include <fdeep/fdeep.hpp>
#include <lorina/lorina.hpp>
#include <alice/alice.hpp>
#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>
#include <libkahypar.h>

/*** Algorithms ***/
#include "algorithms/optimization_scripts/aig_script.hpp"
#include "algorithms/optimization_scripts/aig_script2.hpp"
#include "algorithms/optimization_scripts/mig_script.hpp"
#include "algorithms/optimization_scripts/mig_script2.hpp"
#include "algorithms/optimization_scripts/mig_script3.hpp"
#include "algorithms/partitioning/partition_manager.hpp"
#include "algorithms/partitioning/partition_view.hpp"
#include "algorithms/partitioning/hyperg.hpp"
#include "algorithms/partitioning/cluster.hpp"
#include "algorithms/partitioning/seed_partitioner.hpp"
#include "algorithms/partitioning/fpga_seed_partitioner.hpp"
#include "algorithms/partitioning/slack_view.hpp"
#include "utility.hpp"

/*** Stores ***/
#include "store/aig.hpp"
#include "store/mig.hpp"
#include "store/xag.hpp"

/*** Commands ***/
//Input
#include "commands/input/read_aig.hpp"
#include "commands/input/read_blif.hpp"
#include "commands/input/read_verilog.hpp"

//Optimization
#include "commands/optimization/aigscript.hpp"
#include "commands/optimization/aigscript2.hpp"
#include "commands/optimization/migscript.hpp"
#include "commands/optimization/migscript2.hpp"
#include "commands/optimization/migscript3.hpp"
#include "commands/optimization/optimization.hpp"
#include "commands/optimization/depthr.hpp"
#include "commands/optimization/cut_e.hpp"
#include "commands/optimization/interleaving.hpp"
#include "commands/optimization/balance.hpp"
#include "commands/optimization/refactor.hpp"
// #include "commands/optimization/depth_partition.hpp"
// #include "commands/optimization/depth_based.hpp"
// #include "commands/optimization/mixed_2steps.hpp"

//LUT_Map
#include "commands/lut_map/lut_map.hpp"

//Output
#include "commands/output/write_verilog.hpp"
#include "commands/output/write_bench.hpp"
#include "commands/output/write_dot.hpp"
#include "commands/output/write_hypergraph.hpp"
#include "commands/output/show_ntk.hpp"
#include "commands/output/disjoint_clouds.hpp"
#include "commands/output/get_all_partitions.hpp"
#include "commands/output/print_karnaugh_map.hpp"

//Stats
#include "commands/stats/crit_path_stats.hpp"
#include "commands/stats/depth.hpp"
#include "commands/stats/get_cones.hpp"
#include "commands/stats/ntk_stats.hpp"

//Partitioning
#include "commands/partitioning/partitioning.hpp"
#include "commands/partitioning/partition_detail.hpp"

//Classification
#include "commands/classification/generate_truth_tables.hpp"

//Testing
#include "commands/testing/find_xor.hpp"
#include "commands/testing/find_part.hpp"
#include "commands/testing/get_fanout.hpp"
// #include "commands/testing/test_aig_then_part.hpp"
// #include "commands/testing/test_seed_partitioner.hpp"
// #include "commands/testing/test_fpga_seed.hpp"
#include "commands/testing/pattern_view.hpp"

#include "commands/sta.hpp"

ALICE_MAIN( lsoracle )