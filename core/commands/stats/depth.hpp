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
  class depth_command : public alice::command{

    public:
      explicit depth_command( const environment::ptr& env )
          : command( env, "Displays the depth of the stored network" ){

        add_flag("--mig,-m", "Display depth of stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display depth of stored XAG (AIG is default)");
      }

    protected:
      void execute(){
      
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto& mig = *store<mig_ntk>().current();
            mockturtle::depth_view mig_depth{mig};

            std::cout << "MIG level " << mig_depth.depth()  << std::endl;
          }
          else{
            std::cout << "There is not an MIG network stored.\n";
          }
        }
        else if(is_set("xag")){
          if(!store<xag_ntk>().empty()){
            auto& xag = *store<xag_ntk>().current();
            mockturtle::depth_view xag_depth{xag};

            std::cout << "XAG level " << xag_depth.depth()  << std::endl;
          }
          else{
            std::cout << "There is not an XAG network stored.\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto& aig = *store<aig_ntk>().current();
            mockturtle::depth_view aig_depth{aig};

            std::cout << "AIG level " << aig_depth.depth()  << std::endl;
          }
          else{
            std::cout << "There is not an AIG network stored.\n";
          }
        }
      }
    private:
  };

  ALICE_ADD_COMMAND(depth, "Stats");
}