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
#include <vector>
#include "algorithms/optimization/optimizers.hpp"

namespace oracle {

template <typename network>
class exploderizer
{
public:
    using network_names = mockturtle::names_view<network>;
    using window = mockturtle::window_view<mockturtle::names_view<network>>;
    using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;

    exploderizer(mockturtle::names_view<network> ntk, std::string abc_exec): ntk(ntk), abc_exec(abc_exec) {
    }

    xmg_names exploderize()
    {
        xmg_names output;
        ntk.foreach_pi([&](typename network_names::node pi) {
            output.create_pi();
        });
        optimization_strategy_comparator<network> *strategy =  new d_strategy<network>;
        ntk.foreach_po([&](auto po, auto i) {
            auto cone = extract_cone(po);
            std::cout << "********************************"
                      << " pis " << cone.num_pis()
                      << " pos " << cone.num_pos()
                      << " gates " << cone.num_gates() << std::endl;
            optimizer<network> *optimized = optimize(
                *strategy,
                optimization_strategy::depth,
                cone,
                i,
                abc_exec);
            xmg_names optim = optimized->export_superset();
            integrate(output, optim);
            delete optimized;
            std::cout << "******************************** updated result with"
                      << " pis " << output.num_pis()
                      << " pos " << output.num_pos()
                      << " gates " << output.num_gates() << std::endl;

        });
        return output;
    }

    void integrate(xmg_names &output, xmg_names &optim)
    {
        mockturtle::node_map<typename xmg_names::signal, xmg_names> map(optim);
        auto topo = mockturtle::topo_view(optim);
        topo.foreach_pi([&](auto p, auto i) {
            map[p] = output.make_signal(output.pi_at(i));
        });
        topo.foreach_gate([&](auto g) {
            std::vector<typename xmg_names::signal> fin;
            topo.foreach_fanin(g, [&](const typename xmg_names::signal f){
                auto n = map[topo.get_node(f)];
                if (optim.is_complemented(f)) {
                    fin.push_back(output.create_not(n));
                } else {
                    fin.push_back(n);
                }
            });
            map[g] = output.clone_node(output, g, fin);
            auto signal = optim.make_signal(g);
            if (optim.has_name(signal)) {
                output.set_name(map[g], optim.get_name(signal));
            }
        });
        topo.foreach_po([&](const typename xmg_names::signal f) {
            auto n = map[topo.get_node(f)];
            if (optim.is_complemented(f)) {
                output.create_po(output.create_not(n));
            } else {
                output.create_po(n);
            }
        });
    }
    /*
     * generate a window of a logic cone, starting at the given signal.
     */
    const window extract_cone(typename network_names::signal s)
    {
        std::vector<typename network_names::node> gates;
        std::vector<typename network_names::node> inputs;
        std::vector<typename network_names::signal> outputs;
        outputs.push_back(s);

        ntk.foreach_pi([&](auto pi) {
            // relying on mockturtle guarantee of input order.
            inputs.push_back(pi);
        });
        std::queue<typename network_names::node> traverse;
        traverse.push(ntk.get_node(s));
        ntk.incr_trav_id();
        uint32_t id = ntk.trav_id();
        while (!traverse.empty()) {
            auto t = traverse.front();
            if (ntk.visited(t) != id && !ntk.is_pi(t)) {
                gates.push_back(t);
                ntk.foreach_fanin(t, [&](auto f) {
                    traverse.push(ntk.get_node(f));
                });
            }
            ntk.set_visited(t, id);
            traverse.pop();
        }
        return window(ntk, inputs, outputs, gates);
    }
private:
    network_names ntk;
    const std::string abc_exec;
};
};
