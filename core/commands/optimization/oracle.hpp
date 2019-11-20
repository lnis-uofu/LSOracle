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
#include <math.h>


namespace alice
{
  class oracle_command : public alice::command{

    public:
        explicit oracle_command( const environment::ptr& env )
                : command( env, "Reads a File into AIG, Partitions It, and Performs Mixed Synthesis on Network" ){

                opts.add_option( "--filename,filename", filename, "file to read in [.aig, .blif, .v]" )->required();
                opts.add_option( "--partition,partition", num_partitions, "Number of partitions (Network Size / 300 set as partitition number if not specified)" );
                opts.add_option( "--nn_model,-n", nn_model, "Trained neural network model for classification" );
                opts.add_option( "--out,-o", out_file, "output file to write resulting network to [.v, .blif]" );
                opts.add_option( "--strategy,-s", strategy, "classification strategy [area delay product{DEFAULT}=0, area=1, delay=2]" );
                add_flag("--high,-b", "Uses a high effort approach instead of classification");
                add_flag("--aig,-a", "Perform only AIG optimization on all partitions");
                add_flag("--mig,-m", "Perform only MIG optimization on all partitions");
                add_flag("--combine,-c", "Combine adjacent partitions that have been classified for the same optimization");
                add_flag("--skip-feedthrough", "Do not include feedthrough nets when writing out the file");
        }

    protected:
      void execute(){

        mockturtle::aig_network ntk;
        mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
        if(oracle::checkExt(filename, "aig")){
          lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
        }
        else if(oracle::checkExt(filename, "blif")){
          mockturtle::klut_network ntk;
          mockturtle::names_view<mockturtle::klut_network> names_klut{ntk};
          lorina::diagnostic_engine diag;
          auto const result = lorina::read_blif(filename, mockturtle::blif_reader( names_klut ), &diag);

          if(result != lorina::return_code::success)
            std::cout << "parsing failed\n";

          mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;

          mockturtle::node_resynthesis( names_view, names_klut, resyn );
        }
        else if(oracle::checkExt(filename, "v")){
          lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
        }
        else{
          std::cout << filename << " is not an accepted network file {.aig, .blif, .v}\n";
          return;
        }

        //If number of partitions is not specified
        if(num_partitions == 0){
          double size = ( (double) ntk.size() ) / 300.0;
          num_partitions = ceil(size);
        }

        mockturtle::depth_view orig_depth{names_view};
        oracle::partition_manager<aig_names> partitions(names_view, num_partitions);
        store<part_man_aig_ntk>().extend() = std::make_shared<part_man_aig>( partitions );
        std::cout << filename << " partitioned " << num_partitions << " times\n";
        
        if(is_set("high"))
          high = true;
        if(is_set("aig"))
          aig = true;
        if(is_set("mig"))
          mig = true;
        if(is_set("combine"))
          combine = true;

        auto start = std::chrono::high_resolution_clock::now();

        auto ntk_mig = oracle::optimization(names_view, partitions, strategy, nn_model, 
          high, aig, mig, combine);

        auto stop = std::chrono::high_resolution_clock::now();

        mockturtle::depth_view new_depth{ntk_mig};
        if (ntk_mig.size() != names_view.size() || orig_depth.depth() != new_depth.depth()){
          std::cout << "Final ntk size = " << ntk_mig.num_gates() << " and depth = " << new_depth.depth() << "\n";
          std::cout << "Final number of latches = " << ntk_mig.num_latches() << "\n";
          std::cout << "Area Delay Product = " << ntk_mig.num_gates() * new_depth.depth() << "\n";
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
          std::cout << "Full Optimization: " << duration.count() << "ms\n";
          std::cout << "Finished optimization\n";
          store<mig_ntk>().extend() = std::make_shared<mig_names>( ntk_mig );
          std::cout << "MIG network stored\n";

          if(out_file != ""){
            if(oracle::checkExt(out_file, "v")){
              mockturtle::write_verilog_params ps;
              if(is_set("skip-feedthrough"))
                ps.skip_feedthrough = 1u;  

              mockturtle::write_verilog(ntk_mig, out_file, ps);
              std::cout << "Resulting network written to " << out_file << "\n";
            }
            else if(oracle::checkExt(out_file, "blif")){
              mockturtle::write_blif_params ps;
              if(is_set("skip-feedthrough"))
                ps.skip_feedthrough = 1u;
              
              mockturtle::write_blif(ntk_mig, out_file, ps);
              std::cout << "Resulting network written to " << out_file << "\n";
            }
            else{
              std::cout << out_file << " is not an accepted output file {.v, .blif}\n";
            }
          }
        }
        else{
          std::cout << "No change made to network\n";
        }
      }
    private:
      std::string filename{};
      int num_partitions{0u};
      std::string nn_model{};
      std::string out_file{};
      unsigned strategy{0u};
      bool high = false;
      bool aig = false;
      bool mig = false;
      bool combine = false;
    };

  ALICE_ADD_COMMAND(oracle, "Optimization");
}