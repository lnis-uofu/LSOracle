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
  class read_aig_command : public alice::command{

    public:
      explicit read_aig_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an aig file" ){

        opts.add_option( "--filename,filename", filename, "AIG file to read in" )->required();
        add_flag("--mig,-m", "Store AIG file as MIG network (AIG network is default)");
        add_flag("--xag,-x", "Store AIG file as XAG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(checkExt(filename, "aig")){
          if(is_set("mig")){
            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
            
            mockturtle::depth_view depth{names_view};

            store<mig_ntk>().extend() = std::make_shared<mig_names>( ntk );

            // filename.erase(filename.end() - 4, filename.end());
            // ntk._storage->net_name = filename;

            // std::cout << "MIG Inputs:\n";
            // names_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << names_view.get_name(names_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "MIG Outputs:\n";
            // names_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << names_view.get_output_name(i) << "\n";
            // });

          }
          else if(is_set("xag")){
            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
                
            store<xag_ntk>().extend() = std::make_shared<xag_names>( ntk );

            // filename.erase(filename.end() - 4, filename.end());
            // ntk._storage->net_name = filename;

            // std::cout << "XAG Inputs:\n";
            // names_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << names_view.get_name(names_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "XAG Outputs:\n";
            // names_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << names_view.get_output_name(i) << "\n";
            // });

          }
          else{
            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));

            mockturtle::depth_view depth{names_view};
                
            store<aig_ntk>().extend() = std::make_shared<aig_names>( ntk );

            // filename.erase(filename.end() - 4, filename.end());
            // ntk._storage->net_name = filename;

            // std::cout << "AIG Inputs:\n";
            // names_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << names_view.get_name(names_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "AIG Outputs:\n";
            // names_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << names_view.get_output_name(i) << "\n";
            // });

          }
          

        }
        else{
            std::cout << filename << " is not a valid aig file\n";
        }
        
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read_aig, "Input");
}