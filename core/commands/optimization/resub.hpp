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
class resub_command : public alice::command
{

public:
    explicit resub_command(const environment::ptr &env)
        : command(env, "Perform resubstitution on stored network")
    {
        opts.add_option("--cut_size,-k", cut_size, "Number of inputs in the MFFC. Default = 4.");
        opts.add_option("--insert,-i", insert, "Maximum number of insertions");
        add_flag("--mig,-m", "Resubs a MIG");
        add_flag("--sim,-s", "Simulation based resubstitution");
    }

protected:
    void execute()
    {
        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {
                mockturtle::resubstitution_params ps;
                ps.max_pis = cut_size;
                ps.max_inserts = insert; 

                auto &mig = *store<mig_ntk>().current();
                mockturtle::depth_view depth_mig{mig};
                mockturtle::fanout_view fanout_mig{depth_mig};
                 
                mockturtle::mig_resubstitution( fanout_mig, ps );
                mig = mockturtle::cleanup_dangling( mig );
            } else {
                env->err() << "There is no MIG network stored\n";
            }
        } else {
            if (!store<aig_ntk>().empty()) {
                auto &aig = *store<aig_ntk>().current();

                mockturtle::resubstitution_params ps;
                mockturtle::resubstitution_stats st;

                ps.max_pis = cut_size;
                ps.max_inserts = insert;
                ps.progress = false;

                if (is_set("sim")) {
                    mockturtle::sim_resubstitution( aig, ps, &st );
                }
                else 
                    mockturtle::aig_resubstitution( aig, ps, &st );

                aig = mockturtle::cleanup_dangling( aig );

            } else {
                env->err() << "There is no AIG network stored\n";
            }
        }
    }
private:
    int cut_size = 4;
    int insert = 1;
};

ALICE_ADD_COMMAND(resub, "Resubstitution");
}
