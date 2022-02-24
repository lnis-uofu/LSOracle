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
/*Reads an aig file and stores the AIG network in a store*/
class read_aig_command : public alice::command
{

public:
    explicit read_aig_command(const environment::ptr &env)
        : command(env, "Uses the lorina library to read in an aig file")
    {

        opts.add_option("--filename,filename", filename,
                        "AIG file to read in")->required();
        add_flag("--mig,-m", "Store AIG file as MIG network (AIG network is default)");
        add_flag("--xag,-x", "Store AIG file as XAG network (AIG network is default)");
        add_flag("--xmg,-g", "Store AIG file as XMG network (AIG network is default)");

    }

protected:
    void execute()
    {

        if (oracle::checkExt(filename, "aig")) {
            if (is_set("mig")) {
                mockturtle::mig_network ntk;
                mockturtle::names_view<mockturtle::mig_network> names_view{ntk};
                lorina::return_code result = lorina::read_aiger(filename,
                                             mockturtle::aiger_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read aiger file." << std::endl;
                    return;
                }
                store<mig_ntk>().extend() = std::make_shared<mig_names>(names_view);
                env->out() << "MIG network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            } else if (is_set("xag")) {
                mockturtle::xag_network ntk;
                mockturtle::names_view<mockturtle::xag_network> names_view{ntk};
                lorina::return_code result = lorina::read_aiger(filename,
                                             mockturtle::aiger_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read aiger file." << std::endl;
                    return;
                }
                store<xag_ntk>().extend() = std::make_shared<xag_names>(names_view);
                env->out() << "XAG network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            } else if (is_set("xmg")) {
                mockturtle::xmg_network ntk;
                mockturtle::names_view<mockturtle::xmg_network> names_view{ntk};
                lorina::return_code result = lorina::read_aiger(filename,
                                             mockturtle::aiger_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read aiger file";
                    return;
                }
                store<xmg_ntk>().extend() = std::make_shared<xmg_names>(names_view);
                env->out() << "XMG network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            } else {
                mockturtle::aig_network ntk;
                mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
                lorina::return_code result = lorina::read_aiger(filename,
                                             mockturtle::aiger_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read aiger file." << std::endl;
                    return;
                }
                store<aig_ntk>().extend() = std::make_shared<aig_names>(names_view);
                env->out() << "AIG network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            }

        } else {
            env->err() << filename << " is not a valid aig file\n";
        }
    }
private:
    std::string filename{};
};

ALICE_ADD_COMMAND(read_aig, "Input");
}
