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
          if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            if(!store<oracle::partition_manager<mockturtle::aig_network>>().empty()){
              std::cout << "Generating truth tables for stored AIG network\n";
              auto partitions = store<oracle::partition_manager<mockturtle::aig_network>>().current();
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
