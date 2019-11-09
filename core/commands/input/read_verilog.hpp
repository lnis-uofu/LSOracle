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
  class read_verilog_command : public alice::command{

    public:
      explicit read_verilog_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an Verilog file" ){

        opts.add_option( "--filename,filename", filename, "Verilog file to read in" )->required();
        add_flag("--mig,-m", "Store Verilog file as MIG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(oracle::checkExt(filename, "v")){
          if(is_set("mig")){
            mockturtle::mig_network mig;
            mockturtle::names_view<mockturtle::mig_network> names_view{mig};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
            
            store<mig_ntk>().extend() = std::make_shared<mig_names>( names_view );
            std::cout << "MIG network stored" << std::endl;

            filename.erase(filename.end() - 2, filename.end());
            names_view._storage->net_name = filename;
          }
          else{
            mockturtle::aig_network aig;
            mockturtle::names_view<mockturtle::aig_network> names_view{aig};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
            
            store<aig_ntk>().extend() = std::make_shared<aig_names>( names_view );
            std::cout << "AIG network stored" << std::endl;

            filename.erase(filename.end() - 2, filename.end());
            names_view._storage->net_name = filename;
          }
        }
        else{
          std::cout << filename << " is not a valid Verilog file\n";
        }
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read_verilog, "Input");
}