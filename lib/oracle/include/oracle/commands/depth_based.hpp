#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>

namespace alice
{
class depth_partition_command : public alice::command{

public:
  explicit depth_partition_command( const environment::ptr& env )
    : command( env, "Optimize partitions with MIGs or AIGs based on their logic depth." ){
    opts.add_option( "--num_parts,p", num_parts, "Number of partitions to create" )->required();
    opts.add_option( "--out,-o", out_file, "Verilog output" )->required();
  }

protected:
  void execute(){

    //read AIG to generate hypergraph
    if(!store<mockturtle::aig_network>().empty()) {
      auto& ntk = store<mockturtle::aig_network>().current();

      mockturtle::depth_view depth{ntk};
      std::cout << "Ntk size = " << ntk.num_gates() << " and depth = " << depth.depth() << "\n";

      oracle::partition_manager<mockturtle::aig_network> partitions_aig(ntk, num_parts);

      std::vector<int> aig_based;
      std::vector<int> mig_based;

      for (int i = 0; i < num_parts; i++) {
        //create partitions
        oracle::partition_view<mockturtle::aig_network> part_aig = partitions_aig.create_part(ntk, i);
        mockturtle::depth_view part_depth{part_aig};
        std::cout << "Part size = " << part_aig.num_gates() << " and depth = " << part_depth.depth() << "\n";

        //evaluate partition optimizer based on partitions depth (considering 30% of total depth.. could be configurable as parameter
        if(part_depth.depth() > depth.depth()*0.3)
          mig_based.push_back(i);
        else
          aig_based.push_back(i);
      }

      //Deal with AIG partitions
      for (int i = 0; i < aig_based.size(); i++) {

        std::cout << "Assigning partition " << aig_based.at(i) << " to AIG optimizer " << std::endl;
        oracle::partition_view<mockturtle::aig_network> part_aig = partitions_aig.create_part(ntk, aig_based.at(i));

        std::cout << "\nPartition " << i << "\n";
        mockturtle::depth_view part_depth{part_aig};
        std::cout << "part size = " << part_aig.num_gates() << " and depth = " << part_depth.depth() << "\n";

        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        auto aig_opt = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
        mockturtle::aig_script aigopt;
        aig_opt = aigopt.run(aig_opt);

        mockturtle::depth_view part_aig_depth{aig_opt};
        std::cout << "new part size = " << aig_opt.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";
        partitions_aig.synchronize_part(part_aig, aig_opt, ntk);
      }

      mockturtle::mig_npn_resynthesis resyn_mig;
      auto ntk_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( ntk, resyn_mig );
      std::cout << "MIG created from AIG. There are " << ntk_mig.num_latches() << " latches on the new network " << std::endl;
      std::cout << "There are " << ntk.num_latches() << " latches on the source " << std::endl;

       oracle::partition_manager<mockturtle::mig_network> partitions_mig(ntk_mig, partitions_aig.get_all_part_connections(), 
                    partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(),
                    partitions_aig.get_all_partition_regs(), partitions_aig.get_all_partition_regin(), partitions_aig.get_part_num());

      partitions_aig.connect_outputs(ntk);

      // Deal with MIG partitions
      for(int i = 0; i < mig_based.size(); i++){

        std::cout << "Assigning partition " << mig_based.at(i) << " to MIG optimizer " << std::endl;
        oracle::partition_view<mockturtle::mig_network> part_mig = partitions_mig.create_part(ntk_mig, mig_based.at(i));
        mockturtle::depth_view part_mig_depth{part_mig};
        std::cout << "part size = " << part_mig.num_gates() << " and depth = " << part_mig_depth.depth() << "\n";

        auto mig_opt = mockturtle::node_resynthesis<mockturtle::mig_network>( part_mig, resyn_mig );
        mockturtle::mig_script migopt;
        mig_opt = migopt.run(mig_opt);

        mockturtle::depth_view opt_mig_depth{mig_opt};
        std::cout << "new part size = " << mig_opt.num_gates() << " and depth = " << opt_mig_depth.depth() << "\n";
        std::cout << "Synchronizing " << std::endl;
        partitions_mig.synchronize_part(part_mig, mig_opt, ntk_mig);
      }
      partitions_mig.connect_outputs(ntk_mig);

      ntk_mig = mockturtle::cleanup_dangling( ntk_mig );
      mockturtle::depth_view ntk_depth2{ntk_mig};

      std::cout << "new ntk size = " << ntk_mig.num_gates() << " and depth = " << ntk_depth2.depth() << "\n";
      std::cout << "new ntk number of latches = " << ntk_mig.num_latches() << "\n";

      mockturtle::write_verilog(ntk_mig, out_file);
    }
    else{
      std::cout << "There is no stored AIG network\n";
    }
  }

private:
  int num_parts = 0;
  std::string out_file{};
};

ALICE_ADD_COMMAND(depth_partition, "Optimization");

}