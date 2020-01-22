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
  class write_hypergraph_command : public alice::command{

  public:
    explicit write_hypergraph_command( const environment::ptr& env )
        : command( env, "Output current stored network in a hypergraph representation to be used by hMetis" ){

      opts.add_option( "--filename,filename", filename, "hypergraph file to write to" )->required();
      add_flag("--mig,-m", "Use stored MIG network (Default is AIG)");
    }

  protected:
    void execute(){

      if(is_set("mig")){
        if(!store<mig_ntk>().empty()){

          if(oracle::checkExt(filename, "hpg")){

            auto ntk = *store<mig_ntk>().current();  

            oracle::hypergraph<mig_names> t(ntk);
            t.get_hypergraph(ntk);
            t.dump(filename);
          }
          else{
            std::cout << filename << " is not a valid hpg file\n";
          }
        }
        else{
          std::cout << "There is no MIG network stored\n";
        }
      }
      else{
        if(!store<aig_ntk>().empty()){

          if(oracle::checkExt(filename, "hpg")){
            
            auto ntk = *store<aig_ntk>().current();  

            oracle::hypergraph<aig_names> t(ntk);
            t.get_hypergraph(ntk);
            t.dump(filename);

          }
          else{
            std::cout << filename << " is not a valid hpg file\n";
          }
        }
        else{
          std::cout << "There is no AIG network stored\n";
        }
      }
    }

  private:
    std::string filename{};
  };

  ALICE_ADD_COMMAND(write_hypergraph, "Output");
}