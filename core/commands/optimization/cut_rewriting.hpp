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
  class cut_rewriting_command : public alice::command{

    public:
        explicit cut_rewriting_command( const environment::ptr& env )
                : command( env, "Perform cut rewriting on stored network" ){

                opts.add_option( "--cut_size,-c", cut_size, "Cut size (4 is the default)" );
                add_flag("--mig,-m", "Performs cut rewriting on stored MIG network (AIG is default)");
        }

    protected:
      void execute(){

        mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
        mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto &ntk_mig = *store<mig_ntk>().current();
            mockturtle::mig_npn_resynthesis resyn;
            mockturtle::cut_rewriting_params ps;
            ps.cut_enumeration_ps.cut_size = cut_size;

            mockturtle::cut_rewriting(ntk_mig, resyn, ps);
            ntk_mig = mockturtle::cleanup_dangling(ntk_mig);
          }
          else{
            std::cout << "No MIG stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto &ntk_aig = *store<aig_ntk>().current();
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::cut_rewriting_params ps;
            ps.cut_enumeration_ps.cut_size = cut_size;

            mockturtle::cut_rewriting(ntk_aig, resyn, ps);
            // ntk_aig.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << "\n";
            // });
            // ntk_aig.foreach_gate([&](auto gate){
            //   std::cout << "Gate: " << gate << "\n";
            //   if(ntk_aig.is_po(gate))
            //     std::cout << "IS PO\n";
            //   ntk_aig.foreach_fanin(gate, [&](auto conn, auto i){
            //     std::cout << "child[" << i << "] = " << conn.index << "\n";
            //   });
            // });
            ntk_aig = mockturtle::cleanup_dangling(ntk_aig);

            mockturtle::depth_view depth{ntk_aig};
            std::cout << "Final ntk size = " << ntk_aig.num_gates() << " and depth = " << depth.depth() << "\n";
          }
          else{
            std::cout << "No AIG stored\n";
          }
        }

      }
    private:
        int cut_size = 4;
    };

  ALICE_ADD_COMMAND(cut_rewriting, "Optimization");
}
