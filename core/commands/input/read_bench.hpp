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
  class read_bench_command : public alice::command{

    public:
      explicit read_bench_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an bench file" ){

        opts.add_option( "--filename,filename", filename, "Bench file to read in" )->required();
        add_flag("--aig,-a", "Store BENCH file as AIG network (KLUT network is default)");
        add_flag("--mig,-m", "Store BENCH file as MIG network (KLUT network is default)");
        add_flag("--xag,-x", "Store BENCH file as XAG network (KLUT network is default)");
      }

    protected:
      void execute(){

        if(oracle::checkExt(filename, "bench")){
          
           if(is_set("mig")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_bench(filename, mockturtle::bench_reader( klut_name_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";
            
            mockturtle::mig_npn_resynthesis resyn;

            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );

            store<mig_ntk>().extend() = std::make_shared<mig_names>( named_dest );
            std::cout << "MIG network stored\n";

            filename.erase(filename.end() - 6, filename.end());
            named_dest._storage->net_name = filename;
          }
          else if(is_set("xag")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_bench(filename, mockturtle::bench_reader( klut_name_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";

            mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;

            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );

            store<xag_ntk>().extend() = std::make_shared<xag_names>( named_dest );
            std::cout << "XAG network stored\n";

            filename.erase(filename.end() - 6, filename.end());
            named_dest._storage->net_name = filename;
          }
          else if(is_set("aig")){

            mockturtle::klut_network ntk;
            mockturtle::names_view<mockturtle::klut_network> names_view{ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_bench(filename, mockturtle::bench_reader( names_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";

            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::aig_network aig;
            mockturtle::names_view<mockturtle::aig_network>named_dest ( aig );

            mockturtle::node_resynthesis( named_dest, names_view, resyn );

            store<aig_ntk>().extend() = std::make_shared<aig_names>( named_dest );
            std::cout << "AIG network stored\n";

            filename.erase(filename.end() - 6, filename.end());
            named_dest._storage->net_name = filename;
          }
          else{
            mockturtle::klut_network ntk;
            mockturtle::names_view<mockturtle::klut_network> names_view{ntk};
            lorina::diagnostic_engine diag;
            auto const result = lorina::read_bench(filename, mockturtle::bench_reader( names_view ), &diag);

            if(result != lorina::return_code::success)
              std::cout << "parsing failed\n";

            store<klut_ntk>().extend() = std::make_shared<klut_names>( names_view );
            std::cout << "KLUT network stored\n";

            filename.erase(filename.end() - 6, filename.end());
            names_view._storage->net_name = filename;
          }
        }
        else{
            std::cout << filename << " is not a valid bench file\n";
        }
        
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read_bench, "Input");
}