#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

namespace oracle{
  /*
   * Planning:
   * get cone length from the get_cones command
   * setup a partition_manager with just that partition.
   * run depth optimization on the partition.
   */
  using aig_names = mockturtle::names_view<mockturtle::aig_network>;
  using aig_ntk = std::shared_ptr<aig_names>;
  using part_man_aig = oracle::partition_manager<aig_names>;
  using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

  using mig_names = mockturtle::names_view<mockturtle::mig_network>;
  using mig_ntk = std::shared_ptr<mig_names>;
  using part_man_mig = oracle::partition_manager<mig_names>;
  using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

  mig_names optimization(aig_names ntk_aig, part_man_aig partitions_aig, unsigned threshold){
    // run synthesis
    auto ntk_mig = *aig_to_mig(ntk_aig, 1);
    oracle::partition_manager<mig_names> partitions_mig(ntk_mig,
                                                        partitions_aig.get_all_part_connections(),
                                                        partitions_aig.get_all_partition_inputs(),
                                                        partitions_aig.get_all_partition_outputs(),
                                                        partitions_aig.get_all_partition_regs(),
                                                        partitions_aig.get_all_partition_regin(),
                                                        partitions_aig.get_part_num());
    while true {
        // setup partition
        int longest_po = longest(partitions_mig);
        int partition = setup_partitions(longest_po, partitions_mig);

        // mixed synthesis
        // AIG
        oracle::partition_view<mig_names> aig_part = partitions_mig.create_part(ntk_mig, partition);
        auto aig_pre_part = *part_to_mig(aig_part, 1);
        auto aig_opt = *mig_to_aig(aig_pre_part);
        oracle::aig_script aigopt;
        aig_opt = aigopt.run(aig_opt);
        mockturtle::depth_view aig_depth_view{aig_opt};
        int aig_size = aig_opt.num_gates();
        int aig_depth = aig_depth_view.depth();

        // MIG
        oracle::partition_view<mig_names> mig_part = partitions_mig.create_part(ntk_mig, partition);
        auto mig_opt = *part_to_mig(mig_part, 0);
        oracle::mig_script migopt;
        mig_opt = migopt.run(mig_opt);
        mockturtle::depth_view mig_depth_view{mig_opt};
        int mig_size = mig_opt.num_gates();
        int mig_depth = mig_depth_view.depth();

        // assign best
        if (aig_depth <= threshold && mig_depth <= threshold) {
          if (aig_size < mig_size) {
            partitions_mig.synchronize_part(part, aig_opt, ntk_mig);;
          } else {
            partitions_mig.synchronize_part(part, mig_opt, ntk_mig);;
          }
        } else if(aig_depth <= threshold && mig_depth > threshold) {
          partitions_mig.synchronize_part(part, aig_opt, ntk_mig);;
        } else if (aig_depth > threshold && mig_depth <= threshold) {
          partitions_mig.synchronize_part(part, mig_opt, ntk_mig);;
        } else {
          break;
        }
      }
    partitions_mig.connect_outputs(ntk_mig);

    mockturtle::depth_view ntk_before_depth2{ntk_mig};

    ntk_mig = mockturtle::cleanup_dangling( ntk_mig );
  }

  int longest(

}
