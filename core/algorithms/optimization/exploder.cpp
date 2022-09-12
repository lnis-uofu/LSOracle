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
#include "algorithms/optimization/optimizers.hpp"
#include "exploder.hpp"

namespace oracle
{
template<typename network> typename exploderizer<network>::xmg_names exploderizer<network>::exploderize(uint32_t target_depth)
{
    mockturtle::depth_view depth(ntk);
    xmg_names output;
    ntk.foreach_pi([&](typename exploderizer<network>::network_names::node pi) {
        auto n = output.create_pi();
        auto s = ntk.make_signal( pi );
        if ( ntk.has_name( s ) )
        {
            output.set_name (n, ntk.get_name( s ) );
        }
    });
    optimization_strategy_comparator<network> *strategy =  new depth_strategy<network>;
    ntk.foreach_po([&](auto po, auto i) {
        auto cone = extract_cone(po);

        assert(cone.num_pos() == 1);
        assert(cone.num_pis() == output.num_pis());

        if (depth.level(ntk.get_node(po)) < target_depth) {
            mockturtle::xmg_npn_resynthesis resyn;
            xmg_names copy;
            mockturtle::node_resynthesis<xmg_names, exploderizer<network>::window>(copy, cone, resyn);

            assert(copy.num_pos() == 1);
            assert(copy.num_pis() == output.num_pis());

            integrate(output, copy);
        } else {
            std::cout << "********************************"
                      << " pis " << cone.num_pis()
                      << " pos " << cone.num_pos()
                      << " gates " << cone.num_gates() << std::endl;
            optimizer<network> *optimized = optimize<network>(
                *strategy,
                optimization_strategy::depth,
                cone,
                i,
                abc_exec,
                "");
            xmg_names optim = optimized->export_superset();

            assert(optim.num_pos() == 1);
            assert(optim.num_pis() == output.num_pis());

            integrate(output, optim);
            delete optimized;
            std::cout << "******************************** updated result with"
                      << " pis " << output.num_pis()
                      << " pos " << output.num_pos()
                      << " gates " << output.num_gates() << std::endl;

        }
    });
    delete strategy;

    return mockturtle::cleanup_dangling(output);
}

template<typename network> void exploderizer<network>::integrate(xmg_names &output, xmg_names &optim)
{
    mockturtle::node_map<typename xmg_names::signal, xmg_names> map(optim);
    auto topo = mockturtle::topo_view(optim);
    topo.foreach_pi([&](auto p, auto i) {
        map[p] = output.make_signal(output.pi_at(i));
    });
    topo.foreach_gate([&](auto g) {
        std::vector<typename xmg_names::signal> fin;
        topo.foreach_fanin(g, [&](const typename xmg_names::signal f){
            if (optim.is_constant(optim.get_node(f))) {
                auto fanin = output.get_constant(optim.is_complemented(f));
                fin.push_back(fanin);
            } else {
                auto mapped = map[f];
                auto fanin = optim.is_complemented(f) ? output.create_not(mapped) : mapped;
                fin.push_back(fanin);
            }
        });
        map[g] = output.clone_node(optim, g, fin);
        // output.copy_signal_metadata(map[g], optim, optim.make_signal(g));
    });
    topo.foreach_po([&](const typename xmg_names::signal f) {
        auto n = map[topo.get_node(f)];
        uint32_t id;
        if (optim.is_complemented(f) != output.is_complemented(n)) {
            id = output.create_po(output.create_not(n));
        } else {
            id = output.create_po(n);
        }
        if ( ntk.has_output_name( id ) ) {
            output.set_output_name(id, ntk.get_output_name( id ) );
        }
    });
}

/*
 * generate a window of a logic cone, starting at the given signal.
 */
template<typename network>
const typename exploderizer<network>::window
exploderizer<network>::extract_cone(typename exploderizer<network>::network_names::signal s)
{
    std::vector<typename exploderizer<network>::network_names::node> gates;
    std::vector<typename exploderizer<network>::network_names::node> inputs;
    std::vector<typename exploderizer<network>::network_names::signal> outputs;
    outputs.push_back(s);

    ntk.foreach_pi([&](auto pi) {
        // relying on mockturtle guarantee of input order.
        inputs.push_back(pi);
    });
    std::queue<typename exploderizer<network>::network_names::node> traverse;
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

using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
template xmg_names exploderizer<mockturtle::aig_network>::exploderize(uint32_t);
}
