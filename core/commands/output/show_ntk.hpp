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
  class show_ntk_command : public alice::command{

    public:
      explicit show_ntk_command( const environment::ptr& env )
          : command( env, "Display details about the stored network" ){

        add_flag("--mig,-m", "Store AIG file as MIG network (AIG network is default)");
      }

    protected:
      void execute(){
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto mig = *store<mig_ntk>().current();

            for (int j =1; j < mig._storage->nodes.size(); j++) {
              for (int i = 0; i < mig._storage->nodes.data()->children.size(); i++) {
                std::cout << "node index " << j << " node fan in " << mig._storage->nodes[j].children[i].index << " and data " << mig._storage->nodes[j].children[i].data << std::endl;
              }
            }
            for (unsigned k = mig.num_pis()+1; k<= mig._storage->inputs.size(); k++ ){
              auto node = mig.index_to_node(k);
              std::cout << " reg " << k << " fan out size " << mig.fanout_size(node) << std::endl;
            }
            for (unsigned l=0; l< mig._storage->outputs.size(); l++){
              std::cout << " outputs " << std::endl;
              std::cout << " node fan in data " << mig._storage->outputs[l].data << std::endl;
            }

            std::cout << "Inputs:\n";
            mig.foreach_pi([&](auto pi, auto i){
              if( i < mig.num_pis() - mig.num_latches() )
                std::cout << "PI: " << pi << " name: " << mig.get_name(mig.make_signal(pi)) << "\n";
            });

            std::cout << "Outputs:\n";
            mig.foreach_po([&](auto po, auto i){
              if( i < mig.num_pos() - mig.num_latches() )
                std::cout << "PO: " << po.index << " name: " << mig.get_output_name(i) << "\n";
            });

          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){

            auto aig = *store<aig_ntk>().current();

            for (int j =1; j < aig._storage->nodes.size(); j++) {
              for (int i = 0; i < aig._storage->nodes.data()->children.size(); i++) {
                std::cout << "node index " << j << " node fan in " << aig._storage->nodes[j].children[i].index << " and data " << aig._storage->nodes[j].children[i].data << std::endl;
              }
            }

            for (unsigned k = aig.num_pis()+1; k<= (aig._storage->inputs.size() - aig.num_latches()); k++ ){
              auto node = aig.index_to_node(k);
              std::cout << " reg " << k << " fan out size " << aig.fanout_size(node) << std::endl;
            }

            for( int outIndex=0; outIndex<aig.num_pos()- aig.num_latches(); outIndex++){
              std::cout << "Output " << outIndex << " data " << aig._storage->outputs[outIndex].data << std::endl;
            }

            aig.foreach_ri([&](auto fi) {
              std::cout << "Register " << aig.ri_index(fi) << " data " << fi.data << std::endl;
            });

            std::cout << "Inputs:\n";
            aig.foreach_pi([&](auto pi, auto i){
              if( i < aig.num_pis() - aig.num_latches() )
                std::cout << "PI: " << pi << " name: " << aig.get_name(aig.make_signal(pi)) << "\n";
            });

            std::cout << "Outputs:\n";
            aig.foreach_po([&](auto po, auto i){
              if( i < aig.num_pos() - aig.num_latches() )
                std::cout << "PO: " << po.index << " name: " << aig.get_output_name(i) << "\n";
            });
            
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
        
    private:
      
    };

  ALICE_ADD_COMMAND(show_ntk, "Output");
}