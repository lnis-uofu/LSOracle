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
#include "algorithms/output/xmg_techmap.hpp"

namespace alice
{
class xmg_map_command : public alice::command
{
    using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
    using xmg_ntk = std::shared_ptr<xmg_names>;
public:
    explicit xmg_map_command(const environment::ptr &env)
        : command(env, "Performs technology mapping of the network")
    {
    }

protected:
    void execute()
    {
        if (store<xmg_ntk>().empty()) {
            env->err() << "There is not an XMG network stored.\n";
            return;
        }

        xmg_names &net = *store<xmg_ntk>().current();
        mockturtle::mapping_view mapping(net);
        oracle::xmg_techmap(mapping);
    }
private:
};

ALICE_ADD_COMMAND(xmg_map, "XMG_MAP");
}
