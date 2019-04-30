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
          if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            if(!store<oracle::partition_manager<mockturtle::aig_network>>().empty()){
              std::cout << "Writing k-map images for stored AIG network\n";
              auto partitions = store<oracle::partition_manager<mockturtle::aig_network>>().current();
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

  ALICE_ADD_COMMAND(print_karnaugh, "IO");
}
