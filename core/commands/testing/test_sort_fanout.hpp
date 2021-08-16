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

class test_sort_fanout_command : public alice::command
{

public:
    explicit test_sort_fanout_command(const environment::ptr &env)
        : command(env, "Order the nodes by fanout count")
    {

        add_flag("--mig,-m", "Use stored MIG (AIG network is default)");
    }

protected:
    void execute()
    {
        if (is_set("mig")) {
            if (!store<mockturtle::mig_network>().empty()) {
                auto ntk = store<mockturtle::mig_network>().current();

                mockturtle::fanout_view<mockturtle::mig_network> fanout_ntk{ntk};
                // std::vector<mockturtle::mig_network::node> nodes = ntk._storage->nodes;

                ntk.foreach_node([&](auto node) {
                    env->out() << "Node = " << node << " fanout_size = " << fanout_ntk.fanout(
                                   node).size() << "\n";
                });

                // std::sort(ntk._storage->nodes.begin(), ntk._storage->nodes.end(), less_than_fanout());

                // for(int i = 0; i < ntk._storage->nodes.size(); i++){
                //   env->out() << "Node = " << ntk._storage->nodes[i] << " fanout_size = " << fanout_ntk.fanout(ntk._storage->nodes[i]).size() << "\n";
                // }

            } else {
                env->err() << "MIG network not stored\n";
            }
        } else {
            if (!store<mockturtle::aig_network>().empty()) {
                auto ntk = store<mockturtle::aig_network>().current();

                mockturtle::fanout_view<mockturtle::aig_network> fanout_ntk{ntk};
                // std::vector<mockturtle::mig_network::node> nodes = ntk._storage->nodes;

                ntk.foreach_node([&](auto node) {
                    env->out() << "Node = " << node << " fanout_size = " << fanout_ntk.fanout(
                                   node).size() << "\n";
                });

                // std::sort(ntk._storage->nodes.begin(), ntk._storage->nodes.end(), less_than_fanout());

                // for(int i = 0; i < ntk._storage->nodes.size(); i++){
                //   env->out() << "Node = " << ntk._storage->nodes[i] << " fanout_size = " << fanout_ntk.fanout(ntk._storage->nodes[i]).size() << "\n";
                // }
            } else {
                env->err() << "AIG network not stored\n";
            }
        }
    }

private:

    // struct less_than_fanout
    // {
    //     inline bool operator() (const mockturtle::mig_network::node& node1, const mockturtle::mig_network::node& node2)
    //     {
    //         return (fanout_ntk.fanout(node1).size() < fanout_ntk.fanout(node2).size());
    //     }
    // };

};

ALICE_ADD_COMMAND(test_sort_fanout, "Testing");
}
