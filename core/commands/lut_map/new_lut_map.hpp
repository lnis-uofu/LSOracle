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
  class new_lut_map_command : public alice::command{

    public:
      explicit new_lut_map_command( const environment::ptr& env )
          : command( env, "Performs technology mapping of the network using LSOracle's native technology mapper" ){
        opts.add_option( "--lut_size,-K", cut_size, "Size of cut to calculate [Default = 2]" );
        opts.add_option( "--cut_size,-C", cut_per_node_limit, "Maximum number of cuts to calculate per node [Default = 50]" );

        add_flag("--mig,-m", "Read from the stored MIG network");
        opts.add_option( "--out,-o", out_file, "Write LUT mapping to bench file" );
      }

    protected:
        void execute(){
            
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
                auto& mig = *store<mig_ntk>().current();
                
                //required to provide the mapping view that mockturtle_to_lut_graph wants; alternatively, could use TTs AND2 = 8, MAJ3 = E8, XOR2 = 6.  
                //Larger MAJ and XOR are supported in the newest mockturtle, but not a priority
                mockturtle::mapping_view<mockturtle::mig_network, true> prelim{mig};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4; //default in stock mapper
                ps.cut_enumeration_ps.cut_limit = 8; //v.s.
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( prelim, ps );

                //new mapper
                oracle::techmap::graph <oracle::techmap::cell> mapped_network;
                oracle::techmap::mapping_settings map_param;
                map_param.cut_input_limit = cut_size;
                map_param.node_cut_count = cut_per_node_limit;
                mapped_network = oracle::techmap::mockturtle_to_lut_graph(prelim);
                oracle::techmap::mapper* m = new oracle::techmap::mapper(mapped_network, map_param);
                oracle::techmap::graph <oracle::techmap::lut> lut_network = m->map();

                //return to mockturtle to allow output
                mockturtle::klut_network result = oracle::techmap::lut_graph_to_mockturtle(lut_network);

                if(out_file != ""){
                  std::cout << "filename = " << out_file << "\n";
                  if(oracle::checkExt(out_file, "bench")){
                    mockturtle::write_bench(result, out_file);
                  }
                  else if(oracle::checkExt(out_file, "blif")){
                    mockturtle::write_blif(result, out_file);
                  }
                  else{
                    std::cout << "Not valid output file\n";
                  }
                }

                delete m;

            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          } //mig
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();

              mockturtle::mapping_view<mockturtle::aig_network, true> prelim{aig};
              mockturtle::lut_mapping_params ps;
              ps.cut_enumeration_ps.cut_size = 4;
              ps.cut_enumeration_ps.cut_limit = 8;
              mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>( prelim, ps );

              oracle::techmap::graph <oracle::techmap::cell> mapped_network;
              oracle::techmap::mapping_settings map_param;
              map_param.cut_input_limit = cut_size;
              map_param.node_cut_count = cut_per_node_limit;
              mapped_network = oracle::techmap::mockturtle_to_lut_graph(prelim);
              oracle::techmap::mapper* m = new oracle::techmap::mapper(mapped_network, map_param);
              oracle::techmap::graph <oracle::techmap::lut> lut_network = m->map();
              
              //return to mockturtle to allow output
              mockturtle::klut_network result = oracle::techmap::lut_graph_to_mockturtle(lut_network);

              if(out_file != ""){
                std::cout << "filename = " << out_file << "\n";
                if(oracle::checkExt(out_file, "bench")){
                    mockturtle::write_bench(result, out_file);
                } else if(oracle::checkExt(out_file, "blif")) {
                    mockturtle::write_blif(result, out_file);
                }else{
                    std::cout << "Not valid output file\n";
                }
              } 
              delete m;
            }
            else {
                std::cout << "There is not an AIG network stored.\n";
            }
          } //aig
        }  //execute()

    private:
      int cut_size = 4;
      int cut_per_node_limit = 6; //mockturtle uses 30 by default.  ABC uses 2k or something. I met in the middle.
      std::string out_file = "";
    };

  ALICE_ADD_COMMAND(new_lut_map, "LUT");
}