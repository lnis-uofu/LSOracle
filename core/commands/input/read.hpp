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
  class read_command : public alice::command{

    public:
      explicit read_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read an RTL file into a specified network type" ){

        opts.add_option( "--filename,filename", filename, "File to read in [.aig, .blif, .v]" )->required();
        add_flag("--mig,-m", "Store file as MIG network (AIG network is default)");
        add_flag("--klut,-k", "Store file as KLUT network (AIG network is default)");
        add_flag("--xag,-x", "Store file as XAG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(oracle::checkExt(filename, "aig")){
          if(is_set("mig")){
            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));

            store<mig_ntk>().extend() = std::make_shared<mig_names>( names_view );
            std::cout << "MIG network stored\n";

            filename.erase(filename.end() - 4, filename.end());
            names_view._storage->net_name = filename;
          }
          if(is_set("klut")){
            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));

            mockturtle::mapping_view<aig_names, true> mapped_aig{names_view};
            mockturtle::lut_mapping_params ps;
            ps.cut_enumeration_ps.cut_size = 4;
            mockturtle::lut_mapping<mockturtle::mapping_view<aig_names, true>, true>( mapped_aig, ps );

            const auto klut = *mockturtle::collapse_mapped_network<klut_names>( mapped_aig );

            store<klut_ntk>().extend() = std::make_shared<klut_names>( klut );
            std::cout << "KLUT network stored\n";

            filename.erase(filename.end() - 4, filename.end());
            names_view._storage->net_name = filename;
          }
          else if(is_set("xag")){
            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
                
            store<xag_ntk>().extend() = std::make_shared<xag_names>( names_view );
            std::cout << "XAG network stored\n";

            filename.erase(filename.end() - 4, filename.end());
            names_view._storage->net_name = filename;
          }
          else{
            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
            
            store<aig_ntk>().extend() = std::make_shared<aig_names>( names_view );
            std::cout << "AIG network stored\n";

            filename.erase(filename.end() - 4, filename.end());
            names_view._storage->net_name = filename;
          }

        }
        else if(oracle::checkExt(filename, "blif")){

          if(is_set("mig")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> names_view{klut_ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_blif(filename, mockturtle::blif_reader( names_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";
            
            mockturtle::mig_npn_resynthesis resyn;

            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, names_view, resyn );

            store<mig_ntk>().extend() = std::make_shared<mig_names>( named_dest );
            std::cout << "MIG network stored\n";

            filename.erase(filename.end() - 5, filename.end());
            named_dest._storage->net_name = filename;
          }
          else if(is_set("klut")){
            mockturtle::klut_network ntk;
            mockturtle::names_view<mockturtle::klut_network> names_view{ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_blif(filename, mockturtle::blif_reader( names_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";

            store<klut_ntk>().extend() = std::make_shared<klut_names>( names_view );
            std::cout << "KLUT network stored\n";

            filename.erase(filename.end() - 5, filename.end());
            names_view._storage->net_name = filename;

          }
          else if(is_set("xag")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> names_view{klut_ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_blif(filename, mockturtle::blif_reader( names_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";

            mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;

            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, names_view, resyn );

            store<xag_ntk>().extend() = std::make_shared<xag_names>( named_dest );
            std::cout << "XAG network stored\n";

            filename.erase(filename.end() - 5, filename.end());
            named_dest._storage->net_name = filename;

          }
          else{
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> names_view{klut_ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_blif(filename, mockturtle::blif_reader( names_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";

            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;

            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, names_view, resyn );

            store<aig_ntk>().extend() = std::make_shared<aig_names>( named_dest );
            std::cout << "AIG network stored\n";

            filename.erase(filename.end() - 5, filename.end());
            named_dest._storage->net_name = filename;
          }

        }
        else if(oracle::checkExt(filename, "v")){

          if(is_set("mig")){
            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network> names_view{ntk};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
            
            store<mig_ntk>().extend() = std::make_shared<mig_names>( names_view );
            std::cout << "MIG network stored" << std::endl;

            filename.erase(filename.end() - 2, filename.end());
            names_view._storage->net_name = filename;
          }
          else if(is_set("klut")){
            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));

            mockturtle::mapping_view<aig_names, true> mapped_aig{names_view};
            mockturtle::lut_mapping_params ps;
            ps.cut_enumeration_ps.cut_size = 4;
            mockturtle::lut_mapping<mockturtle::mapping_view<aig_names, true>, true>( mapped_aig, ps );

            const auto klut = *mockturtle::collapse_mapped_network<klut_names>( mapped_aig );

            store<klut_ntk>().extend() = std::make_shared<klut_names>( klut );
            std::cout << "KLUT network stored\n";

            filename.erase(filename.end() - 2, filename.end());
            names_view._storage->net_name = filename;

          }
          else if(is_set("xag")){
            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network> names_view{ntk};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
            
            store<xag_ntk>().extend() = std::make_shared<xag_names>( names_view );
            std::cout << "XAG network stored" << std::endl;

            filename.erase(filename.end() - 2, filename.end());
            names_view._storage->net_name = filename;
          }
          else{
            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
            
            store<aig_ntk>().extend() = std::make_shared<aig_names>( names_view );
            std::cout << "AIG network stored" << std::endl;

            filename.erase(filename.end() - 2, filename.end());
            names_view._storage->net_name = filename;
          }
        }
        else{
            std::cout << filename << " is not a valid input file. Accepted file extensions are .aig, .blif, and .v\n";
        }
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read, "Input");
}