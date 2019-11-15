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
  class write_blif_command : public alice::command{

    public:
      explicit write_blif_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into BLIF format" ){

        opts.add_option( "--filename,filename", filename, "BLIF file to write out to" )->required();
        add_flag("--mig,-m", "Read from the MIG network");
        add_flag("--skip-feedthrough", "Do not include feedthrough nets when writing out the file");
      }

    protected:
      void execute(){
        if(oracle::checkExt(filename, "blif")){
          mockturtle::write_blif_params ps;
          if(is_set("skip-feedthrough"))
            ps.skip_feedthrough = 1u;
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_blif(mig, filename, ps);
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_blif(aig, filename, ps);
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }
        else{
            std::cout << filename << " is not a valid BLIF file\n";
        }
      }
    private:
      std::string filename{};
  };

  ALICE_ADD_COMMAND(write_blif, "Output");
}