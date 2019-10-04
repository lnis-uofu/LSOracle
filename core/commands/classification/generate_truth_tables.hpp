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


namespace alice
{
  class generate_truth_tables_command : public alice::command {

    public:
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>; 

      explicit generate_truth_tables_command( const environment::ptr& env )
          : command( env, "Generates the truth tables for every cone in all partitions" ){

        add_flag("--mig,-m", "Read from the MIG network and MIG partition manager for truth table generation");

      }

    protected:
      void execute(){

        if(is_set("mig")){
          std::cout << "MIG networks not supported yet\n";
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto aig = *store<aig_ntk>().current();
            if(!store<part_man_aig_ntk>().empty()){
              std::cout << "Generating truth tables for stored AIG network\n";
              auto partitions = *store<part_man_aig_ntk>().current();
              partitions.generate_truth_tables(aig);
            }
            else{
              std::cout << "AIG not partitioned yet\n";
            }
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
    private:
  };

  ALICE_ADD_COMMAND(generate_truth_tables, "Classification");
}
