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
#include "utility.hpp"

namespace alice
{
/*Reads a blif file and stores the network in a store*/
class read_blif_command : public alice::command
{

public:
    explicit read_blif_command(const environment::ptr &env)
        : command(env, "Uses the lorina library to read in an blif file")
    {

        opts.add_option("--filename,filename", filename,
                        "BLIF file to read in")->required();
        add_flag("--aig,-a",
                 "Store BLIF file as AIG network (KLUT network is default)");
        add_flag("--mig,-m",
                 "Store BLIF file as MIG network (KLUT network is default)");
        add_flag("--xag,-x",
                 "Store BLIF file as XAG network (KLUT network is default)");
        add_flag("--xmg,-n",
                 "Store BLIF file as XMG network (KLUT network is default)");
    }

protected:
    void execute()
    {

        if (oracle::checkExt(filename, "blif")) {
            if (is_set("mig")) {
                mockturtle::klut_network klut_ntk;
                mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(klut_name_view));

                if (result != lorina::return_code::success)
                    env->err() << "parsing failed\n";

                mockturtle::mig_npn_resynthesis resyn;

                mockturtle::mig_network ntk;
                mockturtle::names_view<mockturtle::mig_network>named_dest(ntk);

                mockturtle::node_resynthesis(named_dest, klut_name_view, resyn);

                store<mig_ntk>().extend() = std::make_shared<mig_names>(named_dest);
                env->out() << "MIG network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                named_dest.set_network_name(filename);
            } else if (is_set("xag")) {
                mockturtle::klut_network klut_ntk;
                mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(klut_name_view));

                if (result != lorina::return_code::success)
                    env->err() << "parsing failed\n";

                mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;

                mockturtle::xag_network ntk;
                mockturtle::names_view<mockturtle::xag_network>named_dest(ntk);

                mockturtle::node_resynthesis(named_dest, klut_name_view, resyn);

                store<xag_ntk>().extend() = std::make_shared<xag_names>(named_dest);
                env->out() << "XAG network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                named_dest.set_network_name(filename);
            } else if (is_set("xmg")) {
                mockturtle::klut_network klut_ntk;
                mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(klut_name_view));

                if (result != lorina::return_code::success)
                    env->err() << "parsing failed\n";

                mockturtle::xmg_npn_resynthesis resyn;

                mockturtle::xmg_network ntk;
                mockturtle::names_view<mockturtle::xmg_network>named_dest(ntk);

                mockturtle::node_resynthesis(named_dest, klut_name_view, resyn);

                store<xmg_ntk>().extend() = std::make_shared<xmg_names>(named_dest);
                env->out() << "XMG network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                named_dest.set_network_name(filename);
            } else if (is_set("aig")) {

                mockturtle::klut_network ntk;
                mockturtle::names_view<mockturtle::klut_network> names_view{ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(names_view));

                if (result != lorina::return_code::success)
                    env->err() << "parsing failed\n";

                mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
                mockturtle::aig_network aig;
                mockturtle::names_view<mockturtle::aig_network>named_dest(aig);

                mockturtle::node_resynthesis(named_dest, names_view, resyn);

                store<aig_ntk>().extend() = std::make_shared<aig_names>(named_dest);
                env->out() << "AIG network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                named_dest.set_network_name(filename);
            } else {
                mockturtle::klut_network ntk;
                mockturtle::names_view<mockturtle::klut_network> names_view{ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(names_view));

                if (result != lorina::return_code::success)
                    env->err() << "parsing failed\n";

                store<klut_ntk>().extend() = std::make_shared<klut_names>(names_view);
                env->out() << "KLUT network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                names_view.set_network_name(filename);
            }
        } else {
            env->err() << filename << " is not a valid blif file\n";
        }

    }
private:
    std::string filename{};
};

ALICE_ADD_COMMAND(read_blif, "Input");
}
