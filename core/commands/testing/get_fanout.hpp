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
class get_fanout_command : public alice::command
{

public:
    explicit get_fanout_command(const environment::ptr &env)
        : command(env, "Get the fanout list of a given node")
    {

        opts.add_option("--node,-n", nodeIdx, "Node to find fanout of")->required();
        add_flag("--mig,-m", "Look at stored MIG");
    }

protected:
    void execute()
    {
        int partition = 0;

        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {

                auto ntk = *store<mig_ntk>().current();
                mockturtle::fanout_view fanout{ntk};
                stringstream ss;
                ss << "Fanout of " << nodeIdx << " = {";
                fanout.foreach_fanout(ntk.index_to_node(nodeIdx), [&](auto const & p) {
                    ss << p << " ";
                });
                ss << "}";
                spdlog::info(ss.str());
            } else {
                spdlog::error("No MIG stored");
            }
        } else {
            if (!store<aig_ntk>().empty()) {

                auto ntk = *store<aig_ntk>().current();
                mockturtle::fanout_view fanout{ntk};
                spdlog::info("Fanout of " << nodeIdx << " = {";
                fanout.foreach_fanout(ntk.index_to_node(nodeIdx), [&](auto const & p) {
                    spdlog::info(p << " ";
                });
                spdlog::info("}");
            } else {
                spdlog::error("No AIG stored");
            }
        }

    }
private:
    int nodeIdx = 0;
};

ALICE_ADD_COMMAND(get_fanout, "Testing");
}
