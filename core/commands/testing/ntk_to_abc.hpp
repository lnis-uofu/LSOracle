#include <alice/alice.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

namespace alice
{

  mockturtle::abc_view abc; 

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
          abc.to_abc(ntk);
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

  class from_abc_command : public alice::command{

    public:
      explicit from_abc_command( const environment::ptr& env )
        : command( env, "Convert AIG into ABC GIA data structure" ){
      }

    protected:
      void execute(){
        if(!store<aig_ntk>().empty()){
          
          if( abc.abc_has_aig() )
          {
            std::cout << "There is an AIG in ABC\n";
          }
          mockturtle::aig_network ntk = abc.from_abc(); 
          printf("ntk size: %d, and nodes: %d, num_pis: %d, and num pos: %d\n", ntk.size(), ntk.num_gates(), ntk.num_pis(), ntk.num_pos());
          abc.dc2();
          auto ntk2 = abc.from_abc(); 
          printf("new ntk size: %d, and nodes: %d, num_pis: %d, and num pos: %d\n", ntk2.size(), ntk2.num_gates(), ntk2.num_pis(), ntk2.num_pos());

          abc.free_manager();
          //mockturtle::ntk_to_abc( ntk ); 
          //abc.stats();
          //abc.foreach_node(); 
        }
        else{
          std::cout << "No AIG stored\n";
        }
      }
    private:
  };

  ALICE_ADD_COMMAND(from_abc, "Testing");
}