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
class write_dot_command : public alice::command
{

public:
    explicit write_dot_command(const environment::ptr &env)
        : command(env, "Writes the Boolean network into structural verilog")
    {

        opts.add_option("--filename,filename", filename,
                        "Verilog file to write out to")->required();
        add_flag("--mig,-m",
                 "Read from the MIG network and MIG partition manager for truth table generation");
    }

protected:
    void execute()
    {
        if (oracle::checkExt(filename, "dot")) {
            if (is_set("mig")) {
                if (!store<mig_ntk>().empty()) {
                    auto &mig = *store<mig_ntk>().current();
                    mockturtle::write_dot(mig, filename);
                } else {
                    env->err() << "There is not an MIG network stored.\n";
                }
            } else {
                if (!store<aig_ntk>().empty()) {
                    auto &aig = *store<aig_ntk>().current();
                    mockturtle::write_dot(aig, filename);
                } else {
                    env->err() << "There is not an AIG network stored.\n";
                }
            }

        } else {
            env->err() << filename << " is not a valid dot file\n";
        }
    }

private:
    std::string filename{};
};

ALICE_ADD_COMMAND(write_dot, "Output");
}
