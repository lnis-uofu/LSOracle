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
  class write_command : public alice::command{

    public:
      explicit write_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to write a stored network into a specific file type" ){

        opts.add_option( "--filename,filename", filename, "File to write to in [.blif, .dot, .hpg, .bench, .v]" )->required();
        add_flag("--mig,-m", "Write out stored MIG network (AIG network is default)");
        add_flag("--klut,-k", "Write out stored KLUT network (AIG network is default)");
        add_flag("--xag,-x", "Write out stored XAG network (AIG network is default)");
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
              std::cout << "MIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else if(is_set("klut")){
            if(!store<klut_ntk>().empty()){
              auto& klut = *store<klut_ntk>().current();
              mockturtle::write_blif(klut, filename, ps);
              std::cout << "KLUT network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an KLUT network stored.\n";
            }
          }
          else if(is_set("xag")){
            if(!store<xag_ntk>().empty()){
              auto& xag = *store<xag_ntk>().current();
              mockturtle::write_blif(xag, filename, ps);
              std::cout << "XAG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an XAG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_blif(aig, filename, ps);
              std::cout << "AIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }

        }
        else if(oracle::checkExt(filename, "dot")){
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_dot(mig, filename);
              std::cout << "MIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else if(is_set("klut")){
            if(!store<klut_ntk>().empty()){
              auto& klut = *store<klut_ntk>().current();
              mockturtle::write_dot(klut, filename);
              std::cout << "KLUT network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an KLUT network stored.\n";
            }
          }
          else if(is_set("xag")){
            if(!store<xag_ntk>().empty()){
              auto& xag = *store<xag_ntk>().current();
              mockturtle::write_dot(xag, filename);
              std::cout << "XAG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an XAG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_dot(aig, filename);
              std::cout << "AIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }

        }
        else if(oracle::checkExt(filename, "hpg")){
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();  
              oracle::hypergraph<mig_names> t(mig);
              t.get_hypergraph(mig);
              t.dump(filename);
              std::cout << "MIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is no MIG network stored\n";
            }
          }
          else if(is_set("klut")){
            if(!store<klut_ntk>().empty()){
              auto& klut = *store<klut_ntk>().current();  
              oracle::hypergraph<klut_names> t(klut);
              t.get_hypergraph(klut);
              t.dump(filename);
              std::cout << "KLUT network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an KLUT network stored.\n";
            }
          }
          else if(is_set("xag")){
            if(!store<xag_ntk>().empty()){
              auto& xag = *store<xag_ntk>().current();  
              oracle::hypergraph<xag_names> t(xag);
              t.get_hypergraph(xag);
              t.dump(filename);
              std::cout << "XAG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an XAG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();  
              oracle::hypergraph<aig_names> t(aig);
              t.get_hypergraph(aig);
              t.dump(filename);
              std::cout << "AIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }

        }
        else if(oracle::checkExt(filename, "bench")){

          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_bench(mig, filename);
              std::cout << "MIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else if(is_set("klut")){
            if(!store<klut_ntk>().empty()){
              auto& klut = *store<klut_ntk>().current();
              mockturtle::write_bench(klut, filename);
              std::cout << "KLUT network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an KLUT network stored.\n";
            }
          }
          else if(is_set("xag")){
            if(!store<xag_ntk>().empty()){
              auto& xag = *store<xag_ntk>().current();
              mockturtle::write_bench(xag, filename);
              std::cout << "XAG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an XAG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_bench(aig, filename);
              std::cout << "AIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }
        else if(oracle::checkExt(filename, "v")){
          mockturtle::write_verilog_params ps;
          if(is_set("skip-feedthrough"))
            ps.skip_feedthrough = 1u;
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_verilog(mig, filename, ps);
              std::cout << "MIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            } 
          }
          else if(is_set("klut")){
            std::cout << "Writing a KLUT network to a Verilog file is not currently supported\n";
          }
          else if(is_set("xag")){
            if(!store<xag_ntk>().empty()){
              auto& xag = *store<xag_ntk>().current();
              mockturtle::write_verilog(xag, filename, ps);
              std::cout << "XAG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an XAG network stored.\n";
            } 
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_verilog(aig, filename, ps);
              std::cout << "AIG network written to " << filename << "\n";
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            } 
          }
        }
        else{
            std::cout << filename << " is not a valid output file. Accepted file extensions are .blif, .bench, and .v\n";
        }
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(write, "Output");
}