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
  /*Reads an aig file and stores the AIG network in a store*/
  class test_case_command : public alice::command{

    public:
      explicit test_case_command( const environment::ptr& env )
          : command( env, "Test cut_rewriting bug" ){
      }

    protected:
      void execute(){

            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
            lorina::read_aiger("broken.aig", mockturtle::aiger_reader( names_view ));
                
            std::cout << "AIG network stored\n";

            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::cut_rewriting_params ps;
            ps.cut_enumeration_ps.cut_size = 4;

            mockturtle::cut_rewriting(names_view, resyn, ps);
            names_view = mockturtle::cleanup_dangling(names_view);
          
      }
    private:
    };

  ALICE_ADD_COMMAND(test_case, "Test");
}