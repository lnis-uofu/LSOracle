/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

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
                // std::string filename = mig._storage->net_name + "_lut.bench";
                mockturtle::mapping_view<mockturtle::mig_network, true> mapped{mig};

                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = lut_size;
                ps.cut_enumeration_ps.cut_limit = cut_size;

                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped, ps );

                std::cout << "number of cells = " << mapped.num_cells() << "\n";

                const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped );
                // auto const& klut = *klut_opt;
                mockturtle::names_view<mockturtle::klut_network> names_view{*klut_opt};

                names_view.foreach_pi([&](auto pi){
                  if(mig.has_name(mig.make_signal(pi - 1))){
                    names_view.set_name(names_view.make_signal(pi), mig.get_name(mig.make_signal(pi - 1)));
                  }
                });
                names_view.foreach_po([&](auto po, auto index){
                  names_view.set_output_name(index, mig.get_output_name(index));
                });

                mockturtle::depth_view klut_depth{names_view};
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
