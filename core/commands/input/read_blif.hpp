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
  /*Reads a blif file and stores the network in a store*/
  class read_blif_command : public alice::command{

    public:
      explicit read_blif_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an blif file" ){

        opts.add_option( "--filename,filename", filename, "BLIF file to read in" )->required();
        add_flag("--mig,-m", "Store BLIF file as MIG network (AIG network is default)");
        add_flag("--xag,-x", "Store BLIF file as XAG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(checkExt(filename, "blif")){
          if(is_set("mig")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::read_blif(filename, mockturtle::blif_reader( klut_name_view ));

            mockturtle::direct_resynthesis<mockturtle::mig_network> resyn;

            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );

            // std::cout << "Blif Inputs:\n";
            // klut_name_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << klut_name_view.get_name(klut_name_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "Blif Outputs:\n";
            // klut_name_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po << " name: " << klut_name_view.get_output_name(i) << "\n";
            // });

            // std::cout << "MIG Inputs:\n";
            // named_dest.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << named_dest.get_name(named_dest.make_signal(pi)) << "\n";
            // });

            // std::cout << "MIG Outputs:\n";
            // named_dest.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << named_dest.get_output_name(i) << "\n";
            // });

            store<mig_ntk>().extend() = std::make_shared<mig_names>( named_dest );
            

          }
          else if(is_set("xag")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::read_blif(filename, mockturtle::blif_reader( klut_name_view ));

            mockturtle::direct_resynthesis<mockturtle::xag_network> resyn;

            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );

            // std::cout << "Blif Inputs:\n";
            // klut_name_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << klut_name_view.get_name(klut_name_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "Blif Outputs:\n";
            // klut_name_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po << " name: " << klut_name_view.get_output_name(i) << "\n";
            // });

            // std::cout << "XAG Inputs:\n";
            // named_dest.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << named_dest.get_name(named_dest.make_signal(pi)) << "\n";
            // });

            // std::cout << "XAG Outputs:\n";
            // named_dest.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << named_dest.get_output_name(i) << "\n";
            // });

            store<xag_ntk>().extend() = std::make_shared<xag_names>( named_dest );
            

          }
          else{
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_blif(filename, mockturtle::blif_reader( klut_name_view ), &diag);
            
            if(result == lorina::return_code::success){
              std::cout << "parsing successful\n";
            }
            else{
              std::cout << "parsing failed\n";
            }

            std::cout << "Blif Inputs:\n";
            klut_name_view.foreach_pi([&](auto pi){
              std::cout << "PI: " << pi << " name: " << klut_name_view.get_name(klut_name_view.make_signal(pi)) << "\n";
            });

            std::cout << "Blif Outputs:\n";
            klut_name_view.foreach_po([&](auto po, auto i){
              std::cout << "PO: " << po << " name: " << klut_name_view.get_output_name(i) << "\n";
            });

            mockturtle::direct_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );
            std::cout << "Finished resynthesis\n";

            

            std::cout << "AIG Inputs:\n";
            named_dest.foreach_pi([&](auto pi){
              std::cout << "PI: " << pi << " name: " << named_dest.get_name(named_dest.make_signal(pi)) << "\n";
            });

            std::cout << "AIG Outputs:\n";
            named_dest.foreach_po([&](auto po, auto i){
              std::cout << "PO: " << po.index << " name: " << named_dest.get_output_name(i) << "\n";
            });

            store<aig_ntk>().extend() = std::make_shared<aig_names>( named_dest );

          }

        }
        else{
            std::cout << filename << " is not a valid blif file\n";
        }
        
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read_blif, "Input");
}