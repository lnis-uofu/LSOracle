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
class depth_command : public alice::command
{

public:
    explicit depth_command(const environment::ptr &env)
        : command(env, "Displays the depth of the stored network")
    {
        add_flag("--aig,-a", "Display depth of stored AIG (AIG is default)");
        add_flag("--mig,-m", "Display depth of stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display depth of stored XAG (AIG is default)");
        add_flag("--xmg,-g", "Display depth of stored XMG (AIG is default)");
    }

protected:
    void execute()
    {

        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {
                auto &mig = *store<mig_ntk>().current();
                mockturtle::depth_view mig_depth{mig};

                spdlog::info("MIG level {}",mig_depth.depth() );
            } else {
                spdlog::error("There is not an MIG network stored.");
            }
        } else if (is_set("xag")) {
            if (!store<xag_ntk>().empty()) {
                auto &xag = *store<xag_ntk>().current();
                mockturtle::depth_view xag_depth{xag};

                spdlog::info("XAG level {}",xag_depth.depth() );
            } else {
                spdlog::error("There is not an XAG network stored.");
            }
        } else if (is_set("xmg")) {
            if (!store<xmg_ntk>().empty()) {
                auto &xmg = *store<xmg_ntk>().current();
                mockturtle::depth_view xmg_depth{xmg};

                spdlog::info("XMG level {}",xmg_depth.depth() );
            } else {
                spdlog::error("There is not an XMG network stored.");
            }
        } else {
            if (!store<aig_ntk>().empty()) {
                auto &aig = *store<aig_ntk>().current();
                mockturtle::depth_view aig_depth{aig};

                spdlog::info("AIG level {}",aig_depth.depth() );
            } else {
                spdlog::error("There is not an AIG network stored.");
            }
        }
    }
private:
};

ALICE_ADD_COMMAND(depth, "Stats");
}
