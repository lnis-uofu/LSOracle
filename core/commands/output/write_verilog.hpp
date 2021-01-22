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
  class write_verilog_command : public alice::command{

    public:
      explicit write_verilog_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into structural verilog" ){

        opts.add_option( "--filename,filename", filename, "Verilog file to write out to" )->required();
        add_flag("--mig,-m", "Read from the MIG network");
        add_flag("--xag,-x", "Read from the XAG network");
        add_flag("--skip-feedthrough", "Do not include feedthrough nets when writing out the file");
      }

    protected:
        void execute(){
        if(oracle::checkExt(filename, "v")){
          mockturtle::write_verilog_params ps;
          if(is_set("skip-feedthrough"))
            ps.skip_feedthrough = 1u;
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_verilog(mig, filename, ps);
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else if(is_set("xag")){
            if(!store<xag_ntk>().empty()){
              auto& xag = *store<xag_ntk>().current();
              mockturtle::write_verilog(xag, filename, ps);
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_verilog(aig, filename, ps);
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }
        else{
            std::cout << filename << " is not a valid verilog file\n";
        }
      }
    private:
      std::string filename{};
  };

  ALICE_ADD_COMMAND(write_verilog, "Output");
}