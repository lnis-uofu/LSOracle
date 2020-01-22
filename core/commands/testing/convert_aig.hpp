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
  class convert_aig_command : public alice::command{

    public:
        explicit convert_aig_command( const environment::ptr& env )
                : command( env, "Convert AIG to MIG" ){

        }

    protected:
      void execute(){
          
        if(!store<aig_ntk>().empty()){

          auto ntk = *store<aig_ntk>().current();
          auto ntk_mig = oracle::aig_to_mig(ntk, 1);
          store<mig_ntk>().extend() = std::make_shared<mig_names>( ntk_mig );

        }
        else{
          std::cout << "No AIG stored\n";
        }
        
      }
    private:
    };

  ALICE_ADD_COMMAND(convert_aig, "Testing");
}