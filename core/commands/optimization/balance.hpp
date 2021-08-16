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

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
  class balance_command : public alice::command{

    public:
      using aig_names = mockturtle::names_view<mockturtle::aig_network>;
      using aig_ntk = std::shared_ptr<aig_names>;
      using part_man_aig = oracle::partition_manager<aig_names>;
      using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

      using mig_names = mockturtle::names_view<mockturtle::mig_network>;
      using mig_ntk = std::shared_ptr<mig_names>;
      using part_man_mig = oracle::partition_manager<mig_names>;
      using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

      explicit balance_command( const environment::ptr& env )
        : command( env, "Balances the stored network in order to reduce the depth" ) {

          add_flag("--mig,-m", "Partitions stored MIG network (AIG network is default)");
        }

    protected:
      void execute(){
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            // auto& ntk = store<mockturtle::mig_network>().current();
            // oracle::balancing( ntk );
            env->err() << "Not implemented yet\n";
          }
          else{
            env->err() << "There is no MIG network stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto& ntk = *store<aig_ntk>().current();
            mockturtle::sop_rebalancing<mockturtle::aig_network> balfn;
            mockturtle::balancing_params bs;
            bs.cut_enumeration_ps.cut_size = 4u;

            ntk = mockturtle::balancing(ntk, {balfn}, bs);
            ntk = mockturtle::cleanup_dangling(ntk);

            mockturtle::depth_view depth{ntk};
            env->out() << "Final ntk size = " << ntk.num_gates() << " and depth = " << depth.depth() << "\n";
          }
          else{
            env->err() << "There is no AIG network stored\n";
          }
        }
      }

  };

  ALICE_ADD_COMMAND(balance, "Optimization");
}
