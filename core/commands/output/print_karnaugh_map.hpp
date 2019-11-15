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
  class print_karnaugh_command : public alice::command{

    public:
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit print_karnaugh_command( const environment::ptr& env )
          : command( env, "Prints all the partitioned truth tables as Karnaugh maps" ){

        opts.add_option("--directory,-d", directory, "Directory to write files to")->required();
        opts.add_option( "--filename,-f", filename, "Classification File to read from" );
        add_flag("--tensor,-t", "Write the k-maps to tensor dataset depending on <filename>");
        add_flag("--mig,-m", "Read from the MIG network and MIG partition manager for k-maps");

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
              std::cout << "Writing k-map images for stored AIG network\n";
              auto partitions = *store<part_man_aig_ntk>().current();
              partitions.write_karnaugh_maps(aig, directory);
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
      std::string filename{};
      std::string directory{};
  };

  ALICE_ADD_COMMAND(print_karnaugh, "Output");
}
