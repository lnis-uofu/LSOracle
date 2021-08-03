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
#pragma once

#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

namespace alice
{
  class connectivity_command : public alice::command{

    public:
      explicit connectivity_command( const environment::ptr& env )
          : command( env, "Displays connectivity stats of the stored network" ){
        add_flag("--aig,-a", "Display stats for stored AIG (AIG is default)");
        add_flag("--mig,-m", "Display stats for stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG (AIG is default)");
      }

    template <typename ntk> void dump_stats(string name) {
      if(!store<ntk>().empty()) {
        auto& dag = *store<ntk>().current();
        mockturtle::depth_view dag_depth{dag};
        mockturtle::fanout_view dag_view{dag_depth};
        vector<uint32_t> fanout_histogram(33, 0);
        vector<uint32_t> skip_histogram(dag_depth.depth(), 0);
        uint32_t mismatch_levels = 0;

        dag_view.foreach_node([this, &dag_view, &fanout_histogram, &skip_histogram, &mismatch_levels](auto n){
          uint32_t fanout = 0;
          uint32_t last_level = 0;
          bool mismatched = false;
          dag_view.foreach_fanout(n, [&dag_view, &skip_histogram, &fanout, &mismatched, &last_level, n](auto f) {
            fanout++;
            if (last_level != 0 && last_level != dag_view.level(f)) {
              mismatched = true;
            }
            last_level = dag_view.level(f);
            uint32_t skip = dag_view.level(f) - dag_view.level(n);
            skip_histogram[skip]++;
          });
          if (mismatched) {
            mismatch_levels++;
          }


          if (fanout >= fanout_histogram.size() - 1) {
            fanout_histogram[fanout_histogram.size() - 1]++;
          } else {
            fanout_histogram[fanout]++;
          }
        });
        env->out() << "Fanout histogram" << std::endl;
        for (size_t i = 0; i < fanout_histogram.size() - 1; i++) {
          env->out() << i << "\t" << fanout_histogram[i] << std::endl;
        }
        env->out() << ">=" << (fanout_histogram.size() - 1) << "\t" << fanout_histogram[fanout_histogram.size() - 1] << std::endl;

        env->out() << "Skip level connections\t" << mismatch_levels << std::endl;
        env->out() << "Skip histogram" << std::endl;
        for (size_t i = 0; i < skip_histogram.size(); i++) {
          env->out() << i << "\t" << skip_histogram[i] << std::endl;
        }
      } else {
        env->out() << "There is not an " << name << " network stored.\n";
      }
    }
    protected:
      void execute(){
        if(is_set("mig")){
          dump_stats<mig_ntk>("MIG");
        } else if(is_set("xag")) {
          dump_stats<xag_ntk>("XAG");
        } else {
          dump_stats<aig_ntk>("AIG");
        }
      }
    private:
  };

  ALICE_ADD_COMMAND(connectivity, "Connectivity");
}
