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
class show_ntk_command : public alice::command
{

public:
    explicit show_ntk_command(const environment::ptr &env)
        : command(env, "Display details about the stored network")
    {

        add_flag("--mig,-m", "Store AIG file as MIG network (AIG network is default)");
    }

protected:
    void execute()
    {
        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {
                auto mig = *store<mig_ntk>().current();

                for (int j = 1; j < mig._storage->nodes.size(); j++) {
                    for (int i = 0; i < mig._storage->nodes.data()->children.size(); i++) {
                        stringstream ss;
                        ss << "node index " << j
                           << " node fan in " << mig._storage->nodes[j].children[i].index
                           << " and data " << mig._storage->nodes[j].children[i].data;
                        spdlog::info(ss.str());
                    }
                }
                for (unsigned k = mig.num_pis() + 1; k <= mig._storage->inputs.size(); k++) {
                    auto node = mig.index_to_node(k);
                    spdlog::info(" reg {} fan out size {}", k, mig.fanout_size(node));
                }
                for (unsigned l = 0; l < mig._storage->outputs.size(); l++) {
                    spdlog::info("outputs node fan in data {}", mig._storage->outputs[l].data);
                }

                spdlog::info("Inputs:");
                mig.foreach_pi([&](auto pi, auto i) {
                    if (i < mig.num_pis() - mig.num_latches())
                        spdlog::info("PI: {} name: {}", pi, mig.get_name(mig.make_signal(pi)));
                });

                spdlog::info("Outputs:");
                mig.foreach_po([&](auto po, auto i) {
                    if (i < mig.num_pos() - mig.num_latches())
                        spdlog::info("PO: {} name: {}", (size_t)po.index, mig.get_output_name(i));
                });

            } else {
                spdlog::error("MIG network not stored");
            }
        } else {
            if (!store<aig_ntk>().empty()) {

                auto aig = *store<aig_ntk>().current();

                for (int j = 1; j < aig._storage->nodes.size(); j++) {
                    for (int i = 0; i < aig._storage->nodes.data()->children.size(); i++) {
                        stringstream ss;
                        ss << "node index " << j
                           << " node fan in " << aig._storage->nodes[j].children[i].index
                           << " and data " << aig._storage->nodes[j].children[i].data;
                        spdlog::info(ss.str());
                    }
                }

                for (unsigned k = aig.num_pis() + 1;
                        k <= (aig._storage->inputs.size() - aig.num_latches()); k++) {
                    auto node = aig.index_to_node(k);
                    spdlog::info(" reg {} fan out size {}", k, aig.fanout_size(
                                   node));
                }

                for (int outIndex = 0; outIndex < aig.num_pos() - aig.num_latches();
                        outIndex++) {
                    spdlog::info("Output {} data {}", outIndex,
                               aig._storage->outputs[outIndex].data);
                }

                aig.foreach_ri([&](auto fi) {
                    spdlog::info("Register {} data {}", aig.ri_index(fi), fi.data);
                });

                spdlog::info("Inputs:");
                aig.foreach_pi([&](auto pi, auto i) {
                    if (i < aig.num_pis() - aig.num_latches())
                        spdlog::info("PI: {} name: {}", pi, aig.get_name(aig.make_signal(pi)));
                });

                spdlog::info("Outputs:");
                aig.foreach_po([&](auto po, auto i) {
                    if (i < aig.num_pos() - aig.num_latches())
                        spdlog::info("PO: {} name: {}", (size_t)po.index, aig.get_output_name(i));
                });

            } else {
                spdlog::error("AIG network not stored");
            }
        }
    }

private:

};

ALICE_ADD_COMMAND(show_ntk, "Output");
}
