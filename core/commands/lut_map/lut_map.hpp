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
  class lut_map_command : public alice::command{

    public:
      explicit lut_map_command( const environment::ptr& env )
          : command( env, "Performs technology mapping of the network" ){

        opts.add_option( "--lut_size,-K", lut_size, "LUT size for mapping [DEFAULT = 6]" );
        opts.add_option( "--cut_size,-C", cut_size, "Max number of priority cuts [DEFAULT = 8]" );
        add_flag("--mig,-m", "Read from the stored MIG network");
        opts.add_option( "--out,-o", out_file, "Write LUT mapping to bench file" );
      }

    protected:
        void execute(){
            
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
                auto& mig = *store<mig_ntk>().current();

                std::cout << "Number of gates = " << mig.num_gates() << "\n";
                mockturtle::depth_view mig_depth{mig};
                std::cout << "Depth = " << mig_depth.depth() << "\n";

                mockturtle::mapping_view<mockturtle::mig_network, true> mapped_mig{mig};

                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4;
                ps.cut_enumeration_ps.cut_limit = 8;
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped_mig, ps );
                std::cout << "number of cells = " << mapped_mig.num_cells() << "\n";
                const auto klut = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_mig );
                mockturtle::depth_view klut_depth{*klut};
                std::cout << "Before optimization\n";
                std::cout << "LUT = " << mapped_mig.num_cells() << " lev = " << klut_depth.depth() << "\n";
                std::cout << "#LUT Level Product = " << mapped_mig.num_cells() * klut_depth.depth() << "\n";
                // // std::string filename = mig._storage->net_name + "_lut.bench";
                // mockturtle::mapping_view<mockturtle::mig_network, true> mapped{mig};

                // mockturtle::lut_mapping_params ps;
                // ps.cut_enumeration_ps.cut_size = lut_size;
                // ps.cut_enumeration_ps.cut_limit = cut_size;

                // mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped, ps );

                // std::cout << "number of cells = " << mapped.num_cells() << "\n";

                // const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped );
                // // auto const& klut = *klut_opt;
                // mockturtle::names_view<mockturtle::klut_network> names_view{*klut_opt};

                // names_view.foreach_pi([&](auto pi){
                //   if(mig.has_name(mig.make_signal(pi - 1))){
                //     names_view.set_name(names_view.make_signal(pi), mig.get_name(mig.make_signal(pi - 1)));
                //   }
                // });
                // names_view.foreach_po([&](auto po, auto index){
                //   names_view.set_output_name(index, mig.get_output_name(index));
                // });
                // names_view._storage->net_name = filename;

                // mockturtle::depth_view klut_depth{names_view};
                // std::cout << "gates = " << names_view.num_gates() << " depth = " << klut_depth.depth() << "\n";
                // std::cout << "LUT = " << mapped.num_cells() << " lev = " << klut_depth.depth() << "\n";
                // std::cout << "#LUT Level Product = " << mapped.num_cells() * klut_depth.depth() << "\n";
                // std::cout << "Finshed LUT mapping\n";

                // mockturtle::cut_rewriting_params cut_ps;
                // cut_ps.cut_enumeration_ps.cut_size = 4;
                // mockturtle::direct_resynthesis<mockturtle::klut_network> resyn_klut;

                // mockturtle::cut_rewriting(names_view, resyn_klut, cut_ps);
                // names_view = mockturtle::cleanup_dangling(names_view);

                // // mockturtle::mapping_view<mockturtle::klut_network, true> new_mapped{names_view};
                // std::cout << "AFTER cut_rewriting\n";
                // mockturtle::depth_view new_klut_depth{names_view};
                // std::cout << "gates = " << names_view.num_gates() << " depth = " << new_klut_depth.depth() << "\n";
                // std::cout << "Product = " << names_view.num_gates() * new_klut_depth.depth() << "\n";

                // if(out_file != ""){
                //   std::cout << "filename = " << out_file << "\n";
                //   if(oracle::checkExt(out_file, "bench")){
                //     mockturtle::write_bench(names_view, out_file);
                //   }
                //   else if(oracle::checkExt(out_file, "blif")){
                //     mockturtle::write_blif(names_view, out_file);
                //   }
                //   else{
                //     std::cout << "Not valid output file\n";
                //   }
                // }
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              // std::string filename = aig._storage->net_name + "_lut.bench";
              mockturtle::mapping_view<mockturtle::aig_network, true> mapped{aig};

              mockturtle::lut_mapping_params ps;
              ps.cut_enumeration_ps.cut_size = lut_size;
              ps.cut_enumeration_ps.cut_limit = cut_size;

              mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>( mapped, ps );

              std::cout << "number of cells = " << mapped.num_cells() << "\n";

              const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped );
              // auto const& klut = *klut_opt;
              mockturtle::names_view<mockturtle::klut_network> names_view{*klut_opt};

              names_view.foreach_pi([&](auto pi){
                if(aig.has_name(aig.make_signal(pi - 1))){
                  names_view.set_name(names_view.make_signal(pi), aig.get_name(aig.make_signal(pi - 1)));
                }
              });
              names_view.foreach_po([&](auto po, auto index){
                names_view.set_output_name(index, aig.get_output_name(index));
              });

              mockturtle::depth_view klut_depth{names_view};
              // std::cout << "klut size = " << klut.size() << "\n";
              // klut.foreach_node([&](auto node){
              //   std::cout << "Node = " << node << "\n";
              //   klut.foreach_fanin(node, [&](auto const &conn, auto i){
              //     std::cout << "child[" << i << "] = " << conn << "\n";
              //   });
              // });
              std::cout << "LUT = " << mapped.num_cells() << " lev = " << klut_depth.depth() << "\n";
              std::cout << "#LUT Level Product = " << mapped.num_cells() * klut_depth.depth() << "\n";
              std::cout << "Finshed LUT mapping\n";
              if(out_file != ""){
                std::cout << "filename = " << out_file << "\n";
                if(oracle::checkExt(out_file, "bench")){
                  mockturtle::write_bench(names_view, out_file);
                }
                else if(oracle::checkExt(out_file, "blif")){
                  mockturtle::write_blif(names_view, out_file);
                }
                else{
                  std::cout << "Not valid output file\n";
                }
              }
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }    
    private:
      int lut_size = 6;
      int cut_size = 8;
      std::string out_file = "";
    };

  ALICE_ADD_COMMAND(lut_map, "LUT");
}