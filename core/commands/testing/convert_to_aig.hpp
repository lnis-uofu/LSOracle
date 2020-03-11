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
  class convert_to_aig_command : public alice::command{

    public:
        explicit convert_to_aig_command( const environment::ptr& env )
                : command( env, "Convert MIG to AIG" ){

        }

    protected:
      void execute(){
          
        if(!store<mig_ntk>().empty()){

          auto ntk = *store<mig_ntk>().current();
          mockturtle::direct_resynthesis<mockturtle::aig_network> resyn;
          mockturtle::aig_network ntk_aig;
          mockturtle::names_view<mockturtle::aig_network>named_dest ( ntk_aig );
          mockturtle::node_resynthesis(named_dest, ntk, resyn);
          store<aig_ntk>().extend() = std::make_shared<aig_names>( named_dest );

        }
        else{
          std::cout << "No MIG stored\n";
        }
        
      }
    private:
    };

  ALICE_ADD_COMMAND(convert_to_aig, "Testing");
}