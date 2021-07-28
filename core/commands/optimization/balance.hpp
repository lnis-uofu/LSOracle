#include <alice/alice.hpp>
#include <mockturtle/mockturtle.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
  class balance_command : public alice::command{

    public:
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit balance_command( const environment::ptr& env )
        : command( env, "Balances the stored network in order to reduce the depth" ) {

          add_flag("--mig,-m", "Partitions stored MIG network (AIG network is default)");
        }

    protected:
      void execute(){
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            // auto& ntk = store<mockturtle::mig_network>().current(); 
            // oracle::balancing( ntk );
            std::cout << "Not implemented yet\n";
          }
          else{
            std::cout << "There is no MIG network stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto& ntk = *store<aig_ntk>().current();
            mockturtle::sop_rebalancing<mockturtle::aig_network> balfn;
            mockturtle::balancing_params bs;
            bs.cut_enumeration_ps.cut_size = 4u;

            ntk = mockturtle::balancing(ntk, {balfn}, bs);
            ntk = mockturtle::cleanup_dangling(ntk);

            mockturtle::depth_view depth{ntk};
            std::cout << "Final ntk size = " << ntk.num_gates() << " and depth = " << depth.depth() << "\n";
          }
          else{
            std::cout << "There is no AIG network stored\n";
          }
        }
      }
    
  };

  ALICE_ADD_COMMAND(balance, "Optimization");
}
