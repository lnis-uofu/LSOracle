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

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <unordered_set>
#include <cassert>

#include <mockturtle/mockturtle.hpp>

namespace oracle
{

template<typename Ntk>
class pattern_rec : public Ntk
{

public:
    using storage = typename Ntk::storage;
    using node = typename Ntk::node;
    using signal = typename Ntk::signal;

public:
    pattern_rec() {}

    explicit pattern_rec(Ntk const &ntk)
        : Ntk(ntk)
    {
        static_assert(mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type");
        static_assert(mockturtle::has_set_visited_v<Ntk>,
                      "Ntk does not implement the set_visited method");
        static_assert(mockturtle::has_visited_v<Ntk>,
                      "Ntk does not implement the visited method");
        static_assert(mockturtle::has_get_node_v<Ntk>,
                      "Ntk does not implement the get_node method");
        static_assert(mockturtle::has_get_constant_v<Ntk>,
                      "Ntk does not implement the get_constant method");
        static_assert(mockturtle::has_is_constant_v<Ntk>,
                      "Ntk does not implement the is_constant method");
        static_assert(mockturtle::has_make_signal_v<Ntk>,
                      "Ntk does not implement the make_signal method");

        create_patterns();
        ntk.foreach_node([&](auto node) {
            considered[node] = false;
        });

    }

    inline void find_xor_groups(Ntk const &ntk)
    {
        for (int i = 0; i < xor_patterns.size(); i++) {
            spdlog::info("pattern " << i + 1);
            Ntk curr_pattern = xor_patterns.at(i);
            mockturtle::default_simulator<kitty::dynamic_truth_table> sim(
                curr_pattern.num_pis());
            const auto tts = mockturtle::simulate<kitty::dynamic_truth_table>(curr_pattern,
                             sim);

            curr_pattern.foreach_po([&](auto const &, auto i) {
                spdlog::info("truth table of node {} is {}", i, kitty::to_hex(
                               tts[i]));
            });
        }

        // mockturtle::default_simulator<kitty::dynamic_truth_table> sim( ntk.num_pis() );
        // const auto tts = mockturtle::simulate_nodes<kitty::dynamic_truth_table>( ntk, sim );

        // ntk.foreach_node( [&]( auto const& node, auto i ) {
        //   spdlog::info("truth table of node " << node << " is ";
        //   kitty::print_hex(tts[node], env->out());
        //   spdlog::info("");
        // } );

        mockturtle::topo_view top{ntk};
        std::vector<node> reverse_top = top.get_node_vec();
        std::reverse(reverse_top.begin(), reverse_top.end());


        for (int i = 0; i < xor_patterns.size(); i++) {
            spdlog::info("\n\nPATTERN NUMBER " << i);
            Ntk curr_pattern = xor_patterns.at(i);

            std::vector<node> xor_nodes;

            for (int j = 0; j < reverse_top.size() - ntk.num_pis(); j++) {
                node ntk_node = reverse_top.at(j);
                spdlog::info("ntk_node = " << ntk_node);
                if (!considered[ntk_node])
                    xor_nodes = xor_test(ntk, curr_pattern, ntk_node);
                // if(xor_nodes.size() != 0){
                //   xor_groups.push_back(xor_nodes);
                //   spdlog::info("XOR added");
                // }
            }
        }

        spdlog::info("xors found = " << xor_groups.size());
        for (int i = 0; i < xor_groups.size(); i++) {
            spdlog::info("XOR group " << i + 1 << " = {";

            for (int j = 0; j != xor_groups.at(i).size(); j++) {
                spdlog::info(xor_groups.at(i).at(j) << " ";
            }
            spdlog::info("}");
            std::vector<node> inputs = xor_inputs(ntk, xor_groups.at(i));
            spdlog::info("inputs = {";
            for (int j = 0; j < inputs.size(); j++) {
                spdlog::info(inputs.at(j) << " ";
            }
            spdlog::info("}");

            spdlog::info("output = " << xor_groups.at(i).back());
        }
    }

private:

    void create_patterns()
    {
        Ntk xor_patt_1;
        auto x = xor_patt_1.create_pi();
        auto y = xor_patt_1.create_pi();
        auto n1 = xor_patt_1.create_and(x, xor_patt_1.create_not(y));
        auto n2 = xor_patt_1.create_and(xor_patt_1.create_not(x), y);
        auto n3 = xor_patt_1.create_and(xor_patt_1.create_not(n1),
                                        xor_patt_1.create_not(n2));
        xor_patt_1.create_po(xor_patt_1.create_not(n3));

        xor_patterns.push_back(xor_patt_1);

        // Ntk xor_patt_2;
        // x = xor_patt_2.create_pi();
        // y = xor_patt_2.create_pi();
        // n1 = xor_patt_2.create_and(x, y);
        // n2 = xor_patt_2.create_and(xor_patt_2.create_not(x), xor_patt_2.create_not(y));
        // n3 = xor_patt_2.create_and(xor_patt_2.create_not(n1), xor_patt_2.create_not(n2));
        // xor_patt_2.create_po(n3);

        // xor_patterns.push_back(xor_patt_2);

        // Ntk xor_patt_6;
        // x = xor_patt_6.create_pi();
        // y = xor_patt_6.create_pi();
        // n1 = xor_patt_6.create_and(xor_patt_6.create_not(x), xor_patt_6.create_not(y));
        // n2 = xor_patt_6.create_and(x, y);
        // n3 = xor_patt_6.create_and(xor_patt_6.create_not(n1), xor_patt_6.create_not(n2));
        // xor_patt_6.create_po(n3);

        // xor_patterns.push_back(xor_patt_6);

        Ntk xor_patt_3;
        x = xor_patt_3.create_pi();
        y = xor_patt_3.create_pi();
        auto n4 = xor_patt_3.create_and(x, xor_patt_3.create_not(y));
        n1 = xor_patt_3.create_and(x, xor_patt_3.create_not(n4));
        n2 = xor_patt_3.create_and(xor_patt_3.create_not(n4), xor_patt_3.create_not(y));
        n3 = xor_patt_3.create_and(xor_patt_3.create_not(n1),
                                   xor_patt_3.create_not(n2));
        xor_patt_3.create_po(n3);

        xor_patterns.push_back(xor_patt_3);

        // Ntk xor_patt_4;
        // x = xor_patt_4.create_pi();
        // y = xor_patt_4.create_pi();
        // n4 = xor_patt_4.create_and(x, y);
        // n1 = xor_patt_4.create_and(x, xor_patt_4.create_not(n4));
        // n2 = xor_patt_4.create_and(xor_patt_4.create_not(n4), y);
        // n3 = xor_patt_4.create_and(xor_patt_4.create_not(n1), xor_patt_4.create_not(n2));
        // xor_patt_4.create_po(xor_patt_4.create_not(n3));

        // xor_patterns.push_back(xor_patt_4);

        // Ntk xor_patt_5;
        // x = xor_patt_5.create_pi();
        // y = xor_patt_5.create_pi();
        // n4 = xor_patt_5.create_and(xor_patt_5.create_not(x), xor_patt_5.create_not(y));
        // n1 = xor_patt_5.create_and(xor_patt_5.create_not(x), xor_patt_5.create_not(n4));
        // n2 = xor_patt_5.create_and(xor_patt_5.create_not(n4), xor_patt_5.create_not(y));
        // n3 = xor_patt_5.create_and(xor_patt_5.create_not(n1), xor_patt_5.create_not(n2));
        // xor_patt_5.create_po(xor_patt_5.create_not(n3));

        // xor_patterns.push_back(xor_patt_5);

    }

    bool node_equivalence(Ntk ntk, Ntk curr_pattern, node ntk_node, node patt_node)
    {
        int ntk_comp_count = 0;
        int patt_comp_count = 0;
        bool equiv = true;
        mockturtle::depth_view depth{ntk};
        int prev_depth = depth.level(ntk.get_node(
                                         ntk._storage->nodes[ntk_node].children[1]));
        ntk.foreach_fanin(ntk_node, [&](auto conn, auto i) {
            auto curr_patt_child = curr_pattern._storage->nodes[patt_node].children[i];
            spdlog::info("ntk child = {} comp = {}", conn.index, ntk.is_complemented(
                           conn));
            spdlog::info("current pattern child = " << curr_patt_child.index << " comp = "
                       << curr_pattern.is_complemented(curr_patt_child));
            if (ntk.is_complemented(conn)) {
                ntk_comp_count++;
            }
            if (curr_pattern.is_complemented(curr_patt_child)) {
                patt_comp_count++;
            }

            if (depth.level(ntk.get_node(conn)) != prev_depth
                    && curr_pattern.is_po(patt_node))
                equiv = false;
        });
        if (ntk_comp_count != patt_comp_count || ntk.is_pi(ntk_node)
                || curr_pattern.is_pi(patt_node)) {
            equiv = false;
        }

        return equiv;
    }

    std::vector<node> xor_inputs(Ntk const &ntk, std::vector<node> xor_group)
    {

        std::sort(xor_group.begin(), xor_group.end());
        // spdlog::info("xor_group = {";
        // for(int i = 0; i < xor_group.size(); i++){
        //   spdlog::info(xor_group.at(i) << " ";
        // }
        // spdlog::info("}");
        mockturtle::depth_view depth{ntk};
        std::vector<node> inputs;
        if (xor_group.size() >= 2) {
            node node1 = xor_group.at(0);
            node node2 = xor_group.at(1);

            // spdlog::info("node1 = " << node1);
            // spdlog::info("node2 = " << node2);

            if (ntk.is_pi(node1)) {
                if (std::find(inputs.begin(), inputs.end(), node1) == inputs.end())
                    inputs.push_back(node1);
            } else {
                ntk.foreach_fanin(node1, [&](auto conn, auto i) {
                    if (ntk.get_node(conn) != node2) {
                        if (std::find(inputs.begin(), inputs.end(), ntk.get_node(conn)) == inputs.end())
                            inputs.push_back(ntk.get_node(conn));
                    }
                });
            }

            if (ntk.is_pi(node2)) {
                if (std::find(inputs.begin(), inputs.end(), node2) == inputs.end())
                    inputs.push_back(node2);
            } else {
                ntk.foreach_fanin(node2, [&](auto conn, auto i) {
                    if (ntk.get_node(conn) != node1) {
                        if (std::find(inputs.begin(), inputs.end(), ntk.get_node(conn)) == inputs.end())
                            inputs.push_back(ntk.get_node(conn));
                    }
                });
            }

            for (int i = 0; i < xor_group.size(); i++) {
                node curr_node = xor_group.at(i);
                if (ntk.is_pi(curr_node))
                    if (std::find(inputs.begin(), inputs.end(), curr_node) == inputs.end())
                        inputs.push_back(curr_node);

                ntk.foreach_fanin(curr_node, [&](auto conn, auto i) {
                    if (std::find(xor_group.begin(), xor_group.end(),
                                  ntk.get_node(conn)) == xor_group.end()) {
                        if (std::find(inputs.begin(), inputs.end(), ntk.get_node(conn)) == inputs.end())
                            inputs.push_back(ntk.get_node(conn));
                    }
                });
            }

        }

        return inputs;

    }

    std::vector<node> xor_test(Ntk const &ntk, Ntk const &curr_pattern,
                               node root_node)
    {
        // int patt_idx = 0;
        std::vector<node> xor_nodes;
        // mockturtle::topo_view patt_top{curr_pattern};
        // std::vector<node> patt_nodes = patt_top.get_node_vec();
        // std::reverse(patt_nodes.begin(), patt_nodes.end());
        mockturtle::fanout_view fanout{ntk};

        std::map<node, bool> visited;
        ntk.foreach_node([&](auto node) {
            visited[ntk.node_to_index(node)] = false;
        });
        // Create a queue for BFS
        std::queue<node> net_queue;

        // Mark the current node as visited and enqueue it
        visited[root_node] = true;
        net_queue.push(root_node);

        spdlog::info("BFS");
        spdlog::info("curr_pattern size = " << (curr_pattern.size() -
                   curr_pattern.num_pos()));
        while (!net_queue.empty()) {
            node curr_node = net_queue.front();
            net_queue.pop();
            spdlog::info(curr_node);
            xor_nodes.push_back(curr_node);

            if (xor_nodes.size() > 1
                    && xor_nodes.size() < (curr_pattern.size() - curr_pattern.num_pos() -
                                           curr_pattern.num_pis() + 1)
                    && (fanout.fanout(curr_node).size() != 1 && !ntk.is_pi(curr_node))) {
                xor_nodes.clear();
                break;
            }

            if (xor_nodes.size() == curr_pattern.size() - curr_pattern.num_pos())
                break;
            // spdlog::info("patt idx = {} pattern gate num = {}", patt_idx, curr_pattern.num_gates() - 1);
            // node patt_node = patt_nodes.at(patt_idx);
            // spdlog::info("pattern node = " << patt_node);
            // spdlog::info("ntk node = " << curr_node);
            // spdlog::info("ntk fanout size = " << fanout.fanout(curr_node).size());
            // bool equiv = node_equivalence(ntk, curr_pattern, curr_node, patt_node);
            // spdlog::info("node equiv = " << equiv);
            // if(!equiv){
            //   patt_idx = 0;
            //   xor_nodes.clear();
            //   spdlog::info("XOR not found for node " << curr_node);
            //   return xor_nodes;
            // }
            // else if(curr_pattern.is_po(patt_node)){
            //   if(equiv){
            //     xor_nodes.push_back(curr_node);
            //     spdlog::info("XOR PO NODE");
            //     patt_idx++;
            //   }
            // }
            // else if(equiv && (fanout.fanout(curr_node).size() == 1)){
            //   xor_nodes.push_back(curr_node);
            //   spdlog::info("XOR GATE NODE");
            //   patt_idx++;
            //   if(patt_idx > (curr_pattern.num_gates() - 1)){
            //     return xor_nodes;
            //   }
            // }
            if (!ntk.is_pi(curr_node)) {
                ntk.foreach_fanin(curr_node, [&](auto conn, auto i) {
                    node curr_child = ntk.get_node(conn);
                    if (!visited[curr_child]) {
                        net_queue.push(curr_child);
                        visited[curr_child] = true;
                    }
                });
            }
        }

        // std::vector<node> inputs;
        // Ensure xor_nodes is in topological order
        std::sort(xor_nodes.begin(), xor_nodes.end());
        // Separating inputs from xor_nodes
        if (xor_nodes.size() > 2) {
            // inputs.push_back(xor_nodes.at(0));
            // inputs.push_back(xor_nodes.at(1));

            xor_nodes.erase(xor_nodes.begin());
            xor_nodes.erase(xor_nodes.begin());
        }

        spdlog::info("xor_nodes = {";
        for (int i = 0; i < xor_nodes.size(); i++) {
            spdlog::info(xor_nodes.at(i) << " ";
        }
        spdlog::info("}");
        std::vector<node> inputs = xor_inputs(ntk, xor_nodes);
        std::vector<node> output = {xor_nodes.back()};
        std::sort(xor_nodes.begin(), xor_nodes.end());
        spdlog::info("xor_nodes = {";
        for (int i = 0; i < xor_nodes.size(); i++) {
            spdlog::info(xor_nodes.at(i) << " ";
        }
        spdlog::info("}");

        spdlog::info("xor inputs = {";
        for (int i = 0; i < inputs.size(); i++) {
            spdlog::info(inputs.at(i) << " ";
        }
        spdlog::info("}");

        spdlog::info("xor output = {";
        for (int i = 0; i < output.size(); i++) {
            spdlog::info(output.at(i) << " ";
        }
        spdlog::info("}");

        if (inputs.size() > 0) {
            mockturtle::fanout_view<Ntk> fanout(ntk);
            fanout.clear_visited();
            mockturtle::window_view<mockturtle::fanout_view<Ntk>> window(fanout, inputs,
                    output, false);
            window.foreach_node([&](auto node) {
                spdlog::info("Node = " << node);
                window.foreach_fanin(node, [&](auto conn, auto i) {
                    spdlog::info("child[" << i << "] = " << conn.index);
                });
            });

            mockturtle::default_simulator<kitty::dynamic_truth_table> sim(window.num_pis());
            const auto tts = mockturtle::simulate<kitty::dynamic_truth_table>(window, sim);

            window.foreach_po([&](auto const &, auto i) {
                spdlog::info(fmt::format("truth table of output {} is {}\n", i,
                                          kitty::to_hex(tts[i]));

                if (kitty::to_hex(tts[i]) == "6" || kitty::to_hex(tts[i]) == "9") {
                    if (std::find(xor_groups.begin(), xor_groups.end(),
                                  xor_nodes) == xor_groups.end()) {
                        bool duplicate = false;
                        for (int i = 0; i < xor_groups.size(); i++) {
                            std::vector<node> other_inputs = xor_inputs(ntk, xor_groups.at(i));
                            std::vector<node> other_output = {xor_groups.at(i).back()};
                            if (other_inputs == inputs && other_output == output) {
                                duplicate = true;
                            }
                        }
                        if (!duplicate)
                            xor_groups.push_back(xor_nodes);
                    }
                }
            });

        } else {
            spdlog::error("circuit too small");
        }

        // Ntk xor_test;
        // mockturtle::node_map<signal, Ntk> old_to_new( curr_pattern );
        // old_to_new[ntk.get_constant( false )] = xor_test.get_constant( false );

        // for(int i = 0; i < inputs.size(); i++){
        //   node curr_node = inputs.at(i);
        //   spdlog::info("pi = " << curr_node);
        //   old_to_new[curr_node] = xor_test.create_pi();
        // }
        // for(int i = inputs.size(); i < xor_nodes.size(); i++){
        //   std::vector<signal> children;
        //   node curr_node = xor_nodes.at(i);
        //   spdlog::info("current node = " << curr_node);

        //   ntk.foreach_fanin( curr_node, [&]( auto child, auto ) {
        //     const auto f = old_to_new[child];
        //     spdlog::info("fanin = {} and data = {}", child.index, child.data);
        //     if ( ntk.is_complemented( child ) )
        //     {
        //       children.push_back( xor_test.create_not( f ) );
        //     }
        //     else
        //     {
        //       children.push_back( f );
        //     }
        //   } );
        //   old_to_new[curr_node] = xor_test.clone_node( ntk, curr_node, children );
        // }
        // spdlog::info("last xor_node = " << xor_nodes.back());
        // xor_test.create_po(old_to_new[xor_nodes.back()]);

        // xor_test.foreach_node([&](auto node){
        //   spdlog::info("node = " << node);
        //   xor_test.foreach_fanin(node, [&](auto conn, auto i){
        //     spdlog::info("child[" << i << "] = " << conn.index);
        //   });
        // });

        // mockturtle::default_simulator<kitty::dynamic_truth_table> sim( xor_test.num_pis() );
        // const auto tts = mockturtle::simulate<kitty::dynamic_truth_table>( xor_test, sim );
        // xor_test.foreach_po( [&]( auto const&, auto i ) {
        //   spdlog::info("truth table of node {} is {}", i, kitty::to_hex(tts[i]));
        // } );

        // if(kitty::to_hex(tts[0]) != "6" && kitty::to_hex(tts[0]) != "9"){
        //   xor_nodes.clear();
        // }
        // else{
        //   for(int i = curr_pattern.num_pis(); i < xor_nodes.size(); i++){
        //     considered[xor_nodes.at(i)] = true;
        //   }
        // }

        return xor_nodes;
    }

    std::vector<Ntk> xor_patterns;
    std::vector<std::vector<node>> xor_groups;
    std::map<node, bool> considered;
};

} /* namespace oracle */
