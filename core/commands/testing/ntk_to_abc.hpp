#include <alice/alice.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

namespace alice
{
  class ntk_to_abc_command : public alice::command{

    public:
      explicit ntk_to_abc_command( const environment::ptr& env )
        : command( env, "Convert AIG into ABC GIA data structure" ){
      }

    protected:
      void execute(){
        if(!store<aig_ntk>().empty()){
          mockturtle::aig_network ntk = *store<aig_ntk>().current();
          //mockturtle::ntk_to_abc( ntk ); 
          mockturtle::abc_view abc{ntk}; 
          abc.stats();
          abc.foreach_node(); 
        }
        else{
          std::cout << "No AIG stored\n";
        }
      }
    private:
    };

  ALICE_ADD_COMMAND(ntk_to_abc, "Testing");
}