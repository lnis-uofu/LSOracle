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
class xmgmigtune_command : public alice::command
{

public:
    explicit xmgmigtune_command(const environment::ptr &env)
        : command(env, "Perform automatic xmg based optimization script")
    {

        opts.add_option("--strategy", strategy, "Optimization strategy [0-1]");
        opts.add_option("--name", name, "Name");
    }

protected:
    void execute()
    {

        if (!store<xmg_ntk>().empty()) {
            auto &opt = *store<xmg_ntk>().current();

            auto start = std::chrono::high_resolution_clock::now();
            mockturtle::depth_view xmg_depth{opt};

            env->out() << "Initial xmg depth " << xmg_depth.depth() << " and nodes " <<
                       opt.num_gates() << std::endl;


            //mockturtle::write_blif_params ps;
            //ps.skip_feedthrough = 1u;
            //mockturtle::write_blif(opt, ckt_name, ps);

            switch (strategy) {
              default:
              case 0: {
                printf("Reducing xmg size...\n");
                oracle::bayes_flow_tune(name.c_str(), 4, 1, 6, 5, 10, 0, 0);
                break;
              }
              
              case 1: {
                printf("Reducing xmg depth...\n");
                oracle::bayes_flow_tune(name.c_str(), 4, 1, 7, 5, 10, 0, 0);
                break; 
                  //
              }
            }

            // mockturtle::depth_view new_xmg_depth{opt};
            // env->out() << "xmg logic depth " << new_xmg_depth.depth() << " nodes " <<
            //            opt.num_gates() << std::endl;

            // env->out() << "Final ntk size = " << opt.num_gates() << " and depth = " <<
            //            new_xmg_depth.depth() << "\n";
            // env->out() << "Area Delay Product = " << opt.num_gates() * new_xmg_depth.depth()
            //            << "\n";
            // auto stop = std::chrono::high_resolution_clock::now();
            // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
            //                 (stop - start);
            // env->out() << "Full Optimization: " << duration.count() << "ms\n";
            // env->out() << "Finished optimization\n";

        } else {
            env->err() << "There is not an xmg network stored.\n";
        }

    }
private:
    unsigned strategy{0u};
    std::string name{};

};

ALICE_ADD_COMMAND(xmgmigtune, "Optimization");
}
