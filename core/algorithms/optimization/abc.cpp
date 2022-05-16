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

#include "algorithms/optimization/abc.hpp"
#include "algorithms/partitioning/partition_manager_junior.hpp"

namespace oracle
{

template <typename network> std::string get_po_name_or_default(const network &ntk, const typename network::signal &signal)
{
    int index = ntk.po_index(signal);

    if (ntk.has_output_name(index)) {
        return ntk.get_output_name(index);
    } else {
        int digits_out = std::to_string(ntk.num_pos()).length();
        // std::cout << "missing output name for index " << index << std::endl;
        return fmt::format("po__{0:0{1}}", index, digits_out);
    }
}

template <typename network>
std::string get_pi_name_or_default(const network &ntk, const typename network::node &node)
{
    typename network::signal signal = ntk.make_signal(node);

    if (ntk.has_name(signal)) {
        return ntk.get_name(signal);
    } else {
        // std::cout << "missing name for PI node " << node << std::endl;
        int digits_in = std::to_string(ntk.num_pis()).length();
        return fmt::format("pi__{0:0{1}}", node, digits_in);
    }
}

template <typename network>
std::string get_node_name_or_default(const network &ntk, const typename network::node &node)
{
    if (ntk.is_pi(node)) {
        return get_pi_name_or_default(ntk, node);
    } else {
        typename network::signal signal = ntk.make_signal(node);
        if (ntk.has_name(signal)) {
            return  ntk.get_name(signal);
        } else {
            // std::cout << "missing name for non-PI node " << node << std::endl;
            int digits_gate = std::to_string(ntk.num_gates()).length();
            return fmt::format("node__{0:0{1}}", node, digits_gate);
        }
    }
}

template <typename network>
std::string get_ri_name_or_default(const network &ntk, const typename network::signal &signal)
{
    if (ntk.has_name(signal)) {
        return ntk.get_name(signal);
    } else {
        typename network::node node = ntk.get_node(signal);
        // std::cout << "missing name for RI node " << node << std::endl;
        int digits_in = std::to_string(ntk.num_registers()).length();
        return fmt::format("ri__{0:0{1}}", node, digits_in);
    }
}

    template <typename network>
mockturtle::window_view<mockturtle::names_view<network>> fix_names2(partition_manager_junior<network> &partman, int index)
{
    mockturtle::window_view<mockturtle::names_view<network>> part = partman.partition(index);
    mockturtle::names_view<network> ntk = partman.get_network();
    part.foreach_pi([&part, &ntk](typename network::node n) {
        std::string name = get_node_name_or_default(ntk, n);
        part.set_name(part.make_signal(n), name);
    });
    int feedthrough = 0;
    part.foreach_po([&part, &ntk, &feedthrough](typename network::signal s, int i) {
        typename network::node n = part.get_node(s);
        if (ntk.is_pi(n)) {
            feedthrough++;
            // skip feedthroughs
            return;
        }
        std::string name = get_node_name_or_default(ntk, n);
        part.set_output_name(i, name);
    });
    if (feedthrough > 0 ) {
        std::cout << "Skipped renaming for " << feedthrough << " feedthrough." << std::endl;
    }
    return part;
}

    void optimize_abc(oracle::partition_manager_junior<mockturtle::aig_network> &partitions,
                      const std::string &abc_exec,
                              const std::string &prefix)
{
    for (int i = 0; i < partitions.count(); i++) {
        auto orig = fix_names2(partitions, i);
        abc_optimizer<mockturtle::aig_network> optimizer(i, orig, optimization_strategy::depth, abc_exec, prefix);
        optimizer.convert();
        optimizer.optimize();
        mockturtle::names_view<mockturtle::aig_network> optimal = optimizer.optimized();
        partitions.integrate<mockturtle::aig_network>(i, optimal);
    }
    partitions.substitute_nodes();
}

}
