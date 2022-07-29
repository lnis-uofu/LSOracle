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

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
class refactor_command : public alice::command
{

public:
    explicit refactor_command(const environment::ptr &env)
        : command(env, "Perform refactoring on stored network")
    {
        opts.add_option("--cut_size,-k", cut_size, "Number of inputs in the MFFC. Default = 4.");
        add_flag("--zero,-z", "Allow zero-gain substitution");
        add_flag("--mig,-m", "Refactoring an MIG");
        add_flag("--xag,-x", "Refactoring an XAG");
        add_flag("--dc,-d", "Uses dont care.");
    }

protected:
    void execute()
    {

        mockturtle::refactoring_params ps;
        if (is_set("zero"))
            ps.allow_zero_gain = true;
        
        if (is_set("dc"))
            ps.use_dont_cares = true;

        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {
                mockturtle::mig_npn_resynthesis resyn;
                ps.max_pis = cut_size;
                auto &ntk = *store<mig_ntk>().current();
                mockturtle::refactoring(ntk, resyn, ps);
                ntk = mockturtle::cleanup_dangling(ntk);
            } else {
                env->err() << "There is no MIG network stored\n";
            }
        } 
        else if (is_set("xag")) {
            if (!store<xag_ntk>().empty()) {
                mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;
                ps.max_pis = cut_size;
                auto &ntk = *store<xag_ntk>().current();
                mockturtle::refactoring(ntk, resyn, ps);
                ntk = mockturtle::cleanup_dangling(ntk);
            } else {
                env->err() << "There is no XAG network stored\n";
            }
        } 
        else {
            if (!store<aig_ntk>().empty()) {
                auto &ntk = *store<aig_ntk>().current();
                // mockturtle::shannon_resynthesis<mockturtle::aig_network> fallback;
                // mockturtle::dsd_resynthesis<mockturtle::aig_network, decltype( fallback )> resyn( fallback );
                mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
                ps.max_pis = cut_size;
                mockturtle::refactoring(ntk, resyn, ps);
                ntk = mockturtle::cleanup_dangling(ntk);

                // mockturtle::depth_view depth{ntk};
                // env->out() << "Final ntk size = " << ntk.num_gates() << " and depth = " <<
                //            depth.depth() << "\n";
            } else {
                env->err() << "There is no AIG network stored\n";
            }
        }
    }
private:
    int cut_size = 4;

};

ALICE_ADD_COMMAND(refactor, "Refactoring");
}
