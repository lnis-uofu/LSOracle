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
class write_hypergraph_command : public alice::command
{

public:
    explicit write_hypergraph_command(const environment::ptr &env)
        : command(env,
                  "Output current stored network in a hypergraph representation to be used by hMetis")
    {

        opts.add_option("--filename,filename", filename,
                        "hypergraph file to write to")->required();
        add_flag("--mig,-m", "Use stored MIG network (Default is AIG)");
    }

protected:
    void execute()
    {

        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {

                if (oracle::checkExt(filename, "hpg")) {

                    auto ntk = *store<mig_ntk>().current();

                    oracle::hypergraph<mig_names> t(ntk);
                    t.get_hypergraph(ntk);
                    t.dump(filename);
                } else {
                    env->err() << filename << " is not a valid hpg file\n";
                }
            } else {
                env->err() << "There is no MIG network stored\n";
            }
        } else {
            if (!store<aig_ntk>().empty()) {

                if (oracle::checkExt(filename, "hpg")) {

                    auto ntk = *store<aig_ntk>().current();

                    oracle::hypergraph<aig_names> t(ntk);
                    t.get_hypergraph(ntk);
                    t.dump(filename);

                } else {
                    env->err() << filename << " is not a valid hpg file\n";
                }
            } else {
                env->err() << "There is no AIG network stored\n";
            }
        }
    }

private:
    std::string filename{};
};

ALICE_ADD_COMMAND(write_hypergraph, "Output");
}
