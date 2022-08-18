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
#include <vector>
#include <mockturtle/mockturtle.hpp>

namespace oracle
{
// Beware the son of partition manager!
template<typename network_base>
class partition_manager_junior
{
public:
    using network = typename mockturtle::names_view<network_base>;
    using partition_map = typename mockturtle::node_map<int, network>;
    using storage = typename network::storage;
    using node = typename network::node;
    using signal = typename network::signal;
    using window_view = typename mockturtle::window_view<network>;
    using fanout_view = typename mockturtle::fanout_view<network>;
    partition_manager_junior(network ntk, partition_map partitions, int part_num):
        ntk(ntk),
        partitions(partitions),
        partition_count(part_num) {}

    network &get_network()
    {
        return ntk;
    }

    window_view partition(int id)
    {
        std::vector<node> inputs;
        std::vector<signal> outputs;
        std::vector<node> gates;
        fanout_view fanout(ntk);

        ntk.foreach_node([&](node const &n) {
           if (ntk.is_constant(n) || partitions[n] != id) {
               return;
           } else if (ntk.is_ci(n)) {
               inputs.push_back(n);
           } else {
              gates.push_back(n);
              // Add CI for each non-partition fanin.
              ntk.foreach_fanin(n, [&](signal const &f){
                  node fin = ntk.get_node(f);
                  if (partitions[fin] != id && !ntk.is_constant(fin)) {
                      inputs.push_back(fin);
                  }
              });
          }

           // Add output if fans out to non-partition.
           fanout.foreach_fanout(n, [&](node const &s) {
               if (partitions[s] != id) {
                  outputs.push_back(ntk.make_signal(n));
                  return false;
               }
               return true;
           });
           // Add output if is a CO source.
           ntk.foreach_co([&](signal const &s) {
               if (ntk.get_node(s) == n) {
                   // always use non-complemented
                   outputs.push_back(ntk.make_signal(n));
                   return false;
               }
               return true;
           });
        });
        std::sort(inputs.begin(), inputs.end());
        auto iend = std::unique(inputs.begin(), inputs.end());
        inputs.resize(std::distance(inputs.begin(),iend));
        std::sort(outputs.begin(), outputs.end());
        auto oend = std::unique(outputs.begin(), outputs.end());
        outputs.resize(std::distance(outputs.begin(),oend));
        return mockturtle::window_view(ntk, inputs, outputs, gates);
    }

    template<class optimized_network>
    void integrate(int id, mockturtle::names_view<optimized_network> &opt)
    {
        window_view part = partition(id);
        integrate<optimized_network>(id, part, opt);
    }

    template<class network>
    void print_network(network &ntk) {
        std::cout << "Network raw:" << std::endl;
        for (int i = 0; i < ntk._storage->nodes.size(); i++) {
            auto n = ntk.index_to_node(i);
            std::cout << " (" << n;
            ntk.foreach_fanin(n, [&](auto c) {
                                          std::cout << "|" << c.index;
                                      });
            std::cout << ")";
        }
        std::cout << std::endl << "Nodes:";
        ntk.foreach_node([&](node n) {
                             std::cout << " " << n;
                         });
        std::cout << std::endl;
        std::cout << "Gates:";
        ntk.foreach_gate([&](node n) {
                             std::cout << " " << n;
                         });
        std::cout << std::endl;
        std::cout << "CIs:";
        ntk.foreach_ci([&](node s, int i) {
                           std::cout << " " << s << "|" << i;
                       });
                std::cout << std::endl;
        std::cout << "COs" << std::endl;
        ntk.foreach_co([&](signal s, int i) {
                           std::cout << s.index << " " << i << " " << s.complement << std::endl;
                       });

    }
    template<class optimized_network>
    void integrate(int partition_id, const window_view &part, mockturtle::names_view<optimized_network> &opt)
    {
        std::cout << "Running integration" << std::endl;
        assert(opt.num_cis() == part.num_cis());
        assert(opt.num_cos() == part.num_cos());
        mockturtle::node_map<signal, mockturtle::names_view<optimized_network>> old_to_new(opt, ntk.get_constant(false));
        // WARNING!!!! This works by assuming that PIs and POs in the
        // optimized network were created in the same order as in the partition.
        // This does not deal with other types of inputs/outputs, window_view treats all
        // inputs/outputs as generic PI/PO.
        part.foreach_ci([&](auto n, auto i) {
            auto o = opt.ci_at(i);
            old_to_new[o] = ntk.make_signal(n);
        });
        std::cout << "Setup PIs" << std::endl;
        mockturtle::topo_view opt_topo{opt};

        opt_topo.foreach_gate([&](auto node) {
            // Insert node into original network.
            std::vector<signal> children;
            opt_topo.foreach_fanin(node, [&](auto child) {
                if (opt_topo.is_constant(opt_topo.get_node(child))) {
                    signal fanin = ntk.get_constant(opt_topo.is_complemented(child));
                    children.push_back(fanin);
                } else {
                    signal mapped = old_to_new[child];
                    signal fanin = opt_topo.is_complemented(child) ? ntk.create_not(mapped) : mapped;
                    children.push_back(fanin);
                }
            });

            old_to_new[node] = ntk.clone_node(opt_topo, node, children);
            // Clone names if present.
            auto signal = opt_topo.make_signal(node);
            if (opt_topo.has_name(signal)) {
                ntk.set_name(old_to_new[node], opt_topo.get_name(signal));
            }
        });
        std::cout << "Inserted new nodes" << std::endl;
        partitions.resize();
        opt_topo.foreach_gate([&](auto node) {
            partitions[old_to_new[node]] = partition_id;
        });

        // Calculate substitutions from partition outputs.
        opt_topo.foreach_co([&](auto opt_signal, auto index) {
            auto opt_node = opt_topo.get_node(opt_signal);
            signal new_out = old_to_new[opt_node];
            if (opt_topo.is_complemented(opt_signal)) {
                new_out = ntk.create_not(new_out);
            }
            signal part_signal = part.co_at(index);
            node orig_node = ntk.get_node(part_signal);
            if (!opt_topo.is_constant(opt_node)
                && !opt_topo.is_ci(opt_node)
                && opt_node != ntk.get_node(new_out)) {
                substitutions[orig_node] = new_out;
            }
        });
        std::cout << "Calculated substitutions" << std::endl;
    }

    void substitute_nodes()
    {
        std::cout << "Begin node substitution for " << substitutions.size() << " nodes" << std::endl;
        // std::list<std::pair<node, signal>> substitution_list(substitutions.begin(), substitutions.end());
        // ntk.substitute_nodes(substitution_list);
        for (auto substitution = substitutions.begin(); substitution != substitutions.end(); substitution++) {
            ntk.substitute_node(substitution->first, substitution->second);

        }
        std::cout << "Substituted nodes." << std::endl;
        ntk = mockturtle::cleanup_dangling(ntk);
        std::cout << "Cleaned up dangling." << std::endl;
        substitutions.clear();
    }
    int node_partition(const node &n)
    {
        return partitions[n];
    }

    int count()
    {
        return partition_count;
    }

private:
    network ntk;
    partition_map partitions;
    int partition_count;
    std::unordered_map<node, signal> substitutions;
};
}
