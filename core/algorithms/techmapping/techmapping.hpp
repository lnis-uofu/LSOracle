/* LSOracle: A learning based Oracle for Logic Synthesis
 * Copyright 2021 Laboratory for Nano Integrated Systems (LNIS)
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

#include <assert.h>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

namespace oracle
{
namespace techmap
{

struct cut {
    explicit cut(size_t node) :
        inputs{std::vector<size_t>{node}}, output{node}
    {
    }

    explicit cut(std::vector<size_t> _inputs, size_t _output, kitty::dynamic_truth_table _truth_table) :
        inputs{std::move(_inputs)}, output{_output}, truth_table{std::move(_truth_table)}
    {
        std::sort(inputs.begin(), inputs.end());
    }

    cut merge(cut const& rhs, size_t new_output) const
    {
        std::vector<size_t> new_inputs;

        /*std::cout << "Merging [";
        for (size_t input : inputs) {
            std::cout << input << ", ";
        }
        std::cout << "] with [";
        for (size_t input : rhs.inputs) {
            std::cout << input << ", ";
        }
        std::cout << "] => [";*/

        std::set_union(inputs.begin(), inputs.end(), rhs.inputs.begin(), rhs.inputs.end(), std::back_inserter(new_inputs));

        /*for (size_t input : new_inputs) {
            std::cout << input << ", ";
        }
        std::cout << "]\n";*/

        return cut(std::move(new_inputs), new_output, kitty::dynamic_truth_table{});
    }

    int input_count() const
    {
        size_t input_count = inputs.size();
        if (std::find(inputs.begin(), inputs.end(), 0) != inputs.end()) {
            input_count -= 1;
        }
        if (std::find(inputs.begin(), inputs.end(), 1) != inputs.end()) {
            input_count -= 1;
        }
        return input_count;
    }

    bool is_trivial() const
    {
        return inputs.size() == 1 && inputs[0] == output;
    }

    bool operator==(cut const& rhs) const {
        return output == rhs.output && inputs == rhs.inputs;
    }

    std::vector<size_t> inputs;
    size_t output;
    kitty::dynamic_truth_table truth_table;
};


struct cell {
    explicit cell(size_t _index, std::vector<kitty::dynamic_truth_table> _truth_table) :
        index{_index}, truth_table{std::move(_truth_table)}
    {
    }

    explicit cell(std::vector<kitty::dynamic_truth_table> _truth_table) :
        index{}, truth_table{std::move(_truth_table)}
    {
    }

    size_t index;
    std::vector<kitty::dynamic_truth_table> truth_table;
};


struct lut {
    explicit lut(kitty::dynamic_truth_table _truth_table) :
        truth_table{std::move(_truth_table)}
    {
    }

    kitty::dynamic_truth_table truth_table;
};


struct constant_zero {};

struct constant_one {};

struct primary_input {
    explicit primary_input(size_t index) :
        index{index}
    {
    }

    size_t index;
};


struct primary_output {
    explicit primary_output(size_t index) :
        index{index}
    {
    }

    size_t index;
};


struct connection {
    explicit connection(size_t from, size_t to, size_t index) :
        from{from}, to{to}, index{index}
    {
    }

    size_t from;
    size_t to;
    size_t index;
};


template<class cell_type>
struct graph {
    size_t add_constant_zero()
    {
        assert(!frozen);
        size_t index = nodes.size();
        nodes.push_back(constant_zero{});
        return index;
    }

    size_t add_constant_one()
    {
        assert(!frozen);
        size_t index = nodes.size();
        nodes.push_back(constant_one{});
        return index;
    }

    bool is_constant(size_t index) const
    {
        return std::holds_alternative<constant_zero>(nodes[index]) || std::holds_alternative<constant_one>(nodes[index]);
    }

    size_t add_primary_input()
    {
        assert(!frozen);
        size_t index = nodes.size();
        nodes.push_back(primary_input{index});
        primary_inputs.push_back(index);
        return index;
    }

    bool is_primary_input(size_t index) const
    {
        return std::holds_alternative<primary_input>(nodes[index]);
    }

    size_t add_primary_output()
    {
        assert(!frozen);
        size_t index = nodes.size();
        nodes.push_back(primary_output{index});
        primary_outputs.push_back(index);
        return index;
    }

    bool is_primary_output(size_t index) const
    {
        return std::holds_alternative<primary_output>(nodes[index]);
    }

    size_t add_cell(cell_type const& c)
    {
        assert(!frozen);
        size_t index = nodes.size();
        nodes.push_back(c);
        return index;
    }

    size_t add_connection(size_t from, size_t to)
    {
        assert(!frozen);
        auto index = connections.size();
        connections.push_back(connection(from, to, index));
        return index;
    }

    void remove_connection(size_t index)
    {
        assert(!frozen && "Attempted to remove connection from frozen graph");
        connections[index].reset();
    }

    void freeze()
    {
        std::cout << "freezing graph...";
        fflush(stdout);
        node_fanin_nodes = std::vector<std::vector<size_t>>{nodes.size()};
        node_fanout_nodes = std::vector<std::vector<size_t>>{nodes.size()};

        // Populate the caches by computing topological orderings.
        compute_topological_ordering();
        compute_reverse_topological_ordering();

        frozen = true;
        std::cout << "done\n";
    }

    void unfreeze()
    {
        frozen = false;
    }

    void dump_to_stdout() const
    {
        std::cout << "digraph {\n";

        // TODO: assuming constant drivers are always first two nodes is probably not smart.
        assert(std::holds_alternative<constant_zero>(nodes[0]));
        assert(std::holds_alternative<constant_one>(nodes[1]));
        std::cout << "0 [shape=box label=\"Zero\"]\n";
        std::cout << "1 [shape=box label=\"One\"]\n";

        for (size_t pi : primary_inputs) {
            std::cout << pi << " [shape=box label=\"PI " << std::get<primary_input>(nodes[pi]).index << "\"]\n";
        }

        for (size_t po : primary_outputs) {
            std::cout << po << " [shape=box label=\"PO " << std::get<primary_output>(nodes[po]).index << "\"]\n";
        }

        for (size_t node = 0; node < nodes.size(); node++) {
            if constexpr (std::is_same_v<cell_type, cell>) {
                if (std::holds_alternative<cell>(nodes[node])) {
                    std::cout << node << " [label=\"Node " << node << " 0x";
                    kitty::print_hex(std::get<cell>(nodes[node]).truth_table[0]);
                    std::cout << "\"]\n";
                }
            } else if constexpr (std::is_same_v<cell_type, lut>) {
                if (std::holds_alternative<lut>(nodes[node])) {
                    std::cout << node << " [label=\"Node " << node << " 0x";
                    kitty::print_hex(std::get<lut>(nodes[node]).truth_table);
                    std::cout << "\"]\n";
                }
            }
        }

        for (std::optional<connection> conn : connections) {
            if (conn.has_value()) {
                std::cout << conn->from << " -> " << conn->to << '\n';
            }
        }

        std::cout << "}\n";
    }

    std::vector<connection> compute_node_fanin_connections(size_t node) const
    {
        std::vector<connection> fanin;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->to == node) {
                fanin.push_back(*conn);
            }
        }
        return fanin;
    }

    std::vector<size_t> compute_node_fanin_nodes(size_t node)
    {
        if (frozen && !node_fanin_nodes[node].empty()) {
            return node_fanin_nodes[node];
        }

        std::vector<size_t> fanin;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->to == node) {
                fanin.push_back(conn->from);
            }
        }

        if (frozen) {
            node_fanin_nodes[node] = fanin;
        }

        return fanin;
    }

    std::vector<size_t> compute_node_fanin_nodes(size_t node) const
    {
        if (frozen && !node_fanin_nodes[node].empty()) {
            return node_fanin_nodes[node];
        }

        std::vector<size_t> fanin;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->to == node) {
                fanin.push_back(conn->from);
            }
        }

        return fanin;
    }

    std::vector<connection> compute_node_fanout_connections(size_t node) const
    {
        std::vector<connection> fanout;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->from == node) {
                fanout.push_back(*conn);
            }
        }
        return fanout;
    }

    std::vector<size_t> compute_node_fanout_nodes(size_t node)
    {
        if (frozen && !node_fanout_nodes[node].empty()) {
            return node_fanout_nodes[node];
        }

        std::vector<size_t> fanout;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->from == node) {
                fanout.push_back(conn->to);
            }
        }

        if (frozen) {
            node_fanout_nodes[node] = fanout;
        }

        return fanout;
    }

    std::vector<size_t> compute_node_fanout_nodes(size_t node) const
    {
        if (frozen && !node_fanout_nodes[node].empty()) {
            return node_fanout_nodes[node];
        }

        std::vector<size_t> fanout;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->from == node) {
                fanout.push_back(conn->to);
            }
        }
        return fanout;
    }

    std::vector<size_t> compute_topological_ordering()
    {
        if (frozen && !forward_topological_ordering.empty()) {
            return forward_topological_ordering;
        }

        std::vector<size_t> ordering;
        std::vector<size_t> no_incoming{primary_inputs};
        graph g{*this};
        g.unfreeze();

        no_incoming.push_back(0);
        no_incoming.push_back(1);

        while (!no_incoming.empty()) {
            size_t node = no_incoming.back();
            ordering.push_back(node);
            no_incoming.pop_back();
            for (connection conn : g.compute_node_fanout_connections(node)) {
                g.remove_connection(conn.index);
                if (g.compute_node_fanin_connections(conn.to).empty()) {
                    no_incoming.push_back(conn.to);
                }
            }
        }

        // If `g` still has edges this is a cyclic graph, which cannot be topologically ordered.
        bool remaining_edges = false;

        for (std::optional<connection> const& conn : g.connections) {
            if (conn) {
                remaining_edges = true;
                std::cout << conn->from << " -> " << conn->to << '\n';
            }
        }

        if (remaining_edges) {
            throw std::logic_error{"input graph is cyclic or has nodes not reachable from primary inputs"};
        }

        if (frozen) {
            forward_topological_ordering = ordering;
        }

        return ordering;
    }

    std::vector<size_t> compute_topological_ordering() const
    {
        if (frozen && !forward_topological_ordering.empty()) {
            return forward_topological_ordering;
        }

        std::vector<size_t> ordering;
        std::vector<size_t> no_incoming{primary_inputs};
        graph g{*this};
        g.unfreeze();

        no_incoming.push_back(0);
        no_incoming.push_back(1);

        while (!no_incoming.empty()) {
            size_t node = no_incoming.back();
            ordering.push_back(node);
            no_incoming.pop_back();
            for (connection conn : g.compute_node_fanout_connections(node)) {
                g.remove_connection(conn.index);
                if (g.compute_node_fanin_connections(conn.to).empty()) {
                    no_incoming.push_back(conn.to);
                }
            }
        }

        // If `g` still has edges this is a cyclic graph, which cannot be topologically ordered.
        bool remaining_edges = false;

        for (std::optional<connection> const& conn : g.connections) {
            if (conn) {
                remaining_edges = true;
                std::cout << conn->from << " -> " << conn->to << '\n';
            }
        }

        if (remaining_edges) {
            throw std::logic_error("input graph is cyclic or has nodes not reachable from primary inputs");
        }

        return ordering;
    }

    std::vector<size_t> compute_reverse_topological_ordering()
    {
        if (frozen && !reverse_topological_ordering.empty()) {
            return reverse_topological_ordering;
        }

        std::vector<size_t> ordering;
        std::vector<size_t> no_outgoing{primary_outputs};
        graph g{*this};
        g.unfreeze();

        while (!no_outgoing.empty()) {
            size_t node = no_outgoing.back();
            ordering.push_back(node);
            no_outgoing.pop_back();
            for (connection conn : g.compute_node_fanin_connections(node)) {
                g.remove_connection(conn.index);
                if (g.compute_node_fanout_connections(conn.from).empty()) {
                    no_outgoing.push_back(conn.from);
                }
            }
        }

        // If `g` still has edges this is a cyclic graph, which cannot be topologically ordered.
        bool remaining_edges = false;

        for (std::optional<connection> const& conn : g.connections) {
            if (conn) {
                remaining_edges = true;
                std::cout << conn->from << " -> " << conn->to << '\n';
            }
        }

        if (remaining_edges) {
            throw std::logic_error("input graph is cyclic or has nodes not reachable from primary inputs");
        }

        if (frozen) {
            reverse_topological_ordering = ordering;
        }

        return ordering;
    }

    std::vector<size_t> nodes_in_cut(cut const& c) const
    {
        // Perform a reverse topological ordering to discover nodes in the cut
        // and then a forward topological ordering to produce useful output.

        std::vector<size_t> ordering;
        std::vector<size_t> no_outgoing{c.output};
        graph g{*this};
        g.unfreeze();

        while (!no_outgoing.empty()) {
            size_t node = no_outgoing.back();
            no_outgoing.pop_back();
            if (std::find(c.inputs.begin(), c.inputs.end(), node) == c.inputs.end()) {
                ordering.push_back(node);
                for (connection conn : g.compute_node_fanin_connections(node)) {
                    no_outgoing.push_back(conn.from);
                }
            }
        }

        // ordering now contains a reverse topological order of the nodes.

        // TODO: does this actually produce a forward topological order?
        std::reverse(ordering.begin(), ordering.end());

        return ordering;
    }

    kitty::dynamic_truth_table simulate(cut const& c) const
    {
        const std::vector<size_t> cut_nodes{nodes_in_cut(c)};
        kitty::dynamic_truth_table result{static_cast<uint32_t>(c.inputs.size())};

        // TODO: skip constant drivers when found.
        const int limit = 1 << c.inputs.size();

        for (unsigned int mask = 0; mask < limit; mask++) {
            std::unordered_map<size_t, bool> values{};

            // Constant drivers.
            values.insert({0, false});
            values.insert({1, true});

            for (int input = 0; input < c.inputs.size(); input++) {
                values.insert({c.inputs[input], ((1 << input) & mask) != 0});
            }

            for (size_t node : cut_nodes) {
                if (std::holds_alternative<cell>(nodes[node])) {
                    cell const& n = std::get<cell>(nodes[node]);
                    std::vector<size_t> fanin = compute_node_fanin_nodes(node);

                    uint64_t node_mask = 0;

                    for (unsigned int fanin_node = 0; fanin_node < fanin.size(); fanin_node++) {
                        if (values.find(fanin[fanin_node]) == values.end()) {
                            std::cout << "while simulating cut [";
                            for (size_t input : c.inputs) {
                                std::cout << input << ", ";
                            }
                            std::cout << "] -> " << c.output << ":\n";
                            std::cout << "at node " << fanin[fanin_node] << ":\n";
                            throw std::logic_error{"fanin node not in simulation values"};
                        }
                        node_mask |= int{values.at(fanin[fanin_node])} << fanin_node;
                    }

                    // TODO: assumes cell has a single output.
                    values.insert({node, kitty::get_bit(n.truth_table[0], node_mask)});
                } else if (is_constant(node)) {
                    continue;
                }
            }

            if (values.at(c.output)) {
                kitty::set_bit(result, mask);
            }
        }

        return result;
    }

    std::vector<size_t> primary_inputs;
    std::vector<size_t> primary_outputs;
    std::vector<std::variant<constant_zero, constant_one, primary_input, primary_output, cell_type>> nodes;
    std::vector<std::optional<connection>> connections;

    bool frozen;
    std::vector<size_t> forward_topological_ordering;
    std::vector<size_t> reverse_topological_ordering;
    std::vector<std::vector<size_t>> node_fanin_nodes;
    std::vector<std::vector<size_t>> node_fanout_nodes;
};


struct mapping_settings {
    unsigned int cut_input_limit;
    unsigned int node_cut_count;
};


struct mapping_info {
    std::optional<cut> selected_cut;
    std::optional<kitty::dynamic_truth_table> truth_table;
    unsigned int depth;
    int references; // Signed to detect underflow.
    float area_flow;
    unsigned int required;
};


struct frontier_info {
    explicit frontier_info(size_t node) :
        cuts{std::vector<cut>{cut{node}}}
    {
    }

    explicit frontier_info(std::vector<cut> _cuts) :
        cuts{std::move(_cuts)}
    {
    }

    std::vector<cut> cuts;
};


class mapper {
public:
    explicit mapper(graph<cell> _g, mapping_settings _settings) :
        g{std::move(_g)}, info{g.nodes.size()}, settings{_settings}
    {
        assert(settings.cut_input_limit >= 2 && "invalid argument: mapping for less than 2 inputs is impossible");
        assert(settings.node_cut_count >= 1 && "invalid argument: must store at least one cut per node");

        std::fill(info.begin(), info.end(), mapping_info{});
    }

    graph<lut> map()
    {
        g.freeze();

        std::cout << "Input graph has " << g.nodes.size() << " nodes and " << g.connections.size() << " edges.\n";

        std::cout << "Mapping phase 1: prioritise depth.\n";
        enumerate_cuts(false, false);

        // After depth mapping, recalculate node slacks.
        recalculate_slack();

        //derive_mapping();

        std::cout << "Mapping phase 2: prioritise global area.\n";
        enumerate_cuts(true, false);

        //derive_mapping();

        enumerate_cuts(true, false);

        //derive_mapping();

        std::cout << "Mapping phase 3: prioritise local area.\n";
        enumerate_cuts(true, true);

        //derive_mapping();

        enumerate_cuts(true, true);

        std::cout << "Deriving the final mapping of the network.\n";
        return derive_mapping();
    }

private:
    void enumerate_cuts(bool area_optimisation, bool local_area)
    {
        std::unordered_map<size_t, frontier_info> frontier;

        // TODO: ABC computes the graph crosscut to pre-allocate frontier memory.

        // Initialise frontier with the trivial cuts of primary inputs.
        frontier.insert({0, frontier_info{0}});
        frontier.insert({1, frontier_info{1}});

        for (size_t pi : g.primary_inputs) {
            frontier.insert({pi, frontier_info{pi}});
        }

        for (size_t node : g.compute_topological_ordering()) {
            // Skip primary inputs, primary outputs, and constants.
            if (g.is_primary_input(node) || g.is_primary_output(node) || g.is_constant(node)) {
                continue;
            }

            // Find the node cut set.
            std::vector<cut> cut_set = node_cut_set(node, frontier);

            // Prune cuts which exceed the node slack in area optimisation mode.
            if (area_optimisation) {
                if (std::all_of(cut_set.begin(), cut_set.end(), [&](cut const& c) {
                    return cut_depth(c, info) > info[node].required;
                })) {
                    std::cout << "Required time of node " << node << " is " << info[node].required << '\n';
                    std::cout << "Depth of cuts:\n";
                    for (cut const& c : cut_set) {
                        std::cout << "[";
                        for (size_t input : c.inputs) {
                            std::cout << input << " @ " << cut_depth(*info[input].selected_cut, info) << ", ";
                        }
                        std::cout << "] -> " << c.output << " = " << cut_depth(c, info) << '\n';
                    }
                }

                cut_set.erase(std::remove_if(cut_set.begin(), cut_set.end(), [&](cut const& c) {
                    return cut_depth(c, info) > info[node].required;
                }), cut_set.end());

                // Because the previous cut is included in the set and must meet required times
                // we must have at least one cut left from this.
                if (cut_set.empty()) {
                    throw std::logic_error{"bug: no cuts meet node required time"};
                }
            }

            // Sort the cuts by desired characteristics.
            if (!area_optimisation) {
                std::sort(cut_set.begin(), cut_set.end(), [&](cut const& a, cut const& b) {
                    return cut_depth(a, info) < cut_depth(b, info) ||
                        (cut_depth(a, info) == cut_depth(b, info) && cut_input_count(a, info) < cut_input_count(b, info)) ||
                        (cut_depth(a, info) == cut_depth(b, info) && cut_input_count(a, info) == cut_input_count(b, info) && cut_area_flow(a, info) < cut_area_flow(b, info));
                });
            } else if (!local_area) {
                std::sort(cut_set.begin(), cut_set.end(), [&](cut const& a, cut const& b) {
                    return cut_area_flow(a, info) < cut_area_flow(b, info) ||
                        (cut_area_flow(a, info) == cut_area_flow(b, info) && cut_fanin_refs(a, info) < cut_fanin_refs(b, info)) ||
                        (cut_area_flow(a, info) == cut_area_flow(b, info) && cut_fanin_refs(a, info) == cut_fanin_refs(b, info) && cut_depth(a, info) < cut_depth(b, info));
                });
            } else {
                std::sort(cut_set.begin(), cut_set.end(), [&](cut const& a, cut const& b) {
                    return cut_exact_area(a) < cut_exact_area(b) ||
                        (cut_exact_area(a) == cut_exact_area(b) && cut_fanin_refs(a, info) < cut_fanin_refs(b, info)) ||
                        (cut_exact_area(a) == cut_exact_area(b) && cut_fanin_refs(a, info) == cut_fanin_refs(b, info) && cut_depth(a, info) < cut_depth(b, info));
                });
            }

            // Deduplicate cuts to ensure diversity.
            cut_set.erase(std::unique(cut_set.begin(), cut_set.end()), cut_set.end());

            // Keep only the specified good cuts.
            if (cut_set.size() > settings.node_cut_count) {
                cut_set.erase(cut_set.begin()+settings.node_cut_count, cut_set.end());
            }

            // We should have at least one cut provided by the trivial cut.
            if (cut_set.empty()) {
                throw std::logic_error{"bug: node has no cuts"};// TODO: maybe this is redundant given the assert in area_optimisation?
            }

            // If there's a representative cut for this node already, decrement its references first.
            if (info[node].selected_cut.has_value()) {
                cut_deref(*info[node].selected_cut);
            }

            // Choose the best cut as the representative cut for this node.
            cut_ref(cut_set[0]);

            // Add the cut set of this node to the frontier.
            info[node].selected_cut = std::make_optional(cut_set[0]);
            info[node].depth = cut_depth(cut_set[0], info);
            info[node].area_flow = cut_area_flow(cut_set[0], info);

            frontier.insert({node, frontier_info{std::move(cut_set)}});

            // Erase fan-in nodes that have their fan-out completely mapped as they will never be used again.
            for (size_t fanin_node : g.compute_node_fanin_nodes(node)) {
                std::vector<size_t> node_fanout{g.compute_node_fanout_nodes(fanin_node)};
                if (std::all_of(node_fanout.begin(), node_fanout.end(), [&](size_t node) {
                    return info[node].selected_cut.has_value();
                })) {
                    //frontier.erase(fanin_node);
                }
            }
        }
    }

    void recalculate_slack()
    {
        // First find the maximum depth of the mapping.
        unsigned int max_depth = 0;

        for (size_t node = 0; node < g.nodes.size(); node++) {
            if (std::holds_alternative<cell>(g.nodes[node])) {
                if (info[node].depth > max_depth) {
                    max_depth = info[node].depth;
                }
            }
        }

        std::cout << "Maximum depth of network is " << max_depth << '\n';

        std::cout << "Propagating arrival times...";
        fflush(stdout);

        // Next, initialise the node required times.
        for (mapping_info& node : info) {
            node.required = max_depth;
        }

        // Then work from PO to PI, propagating required times.
        for (size_t node : g.compute_reverse_topological_ordering()) {
            //std::cout << "Visiting node " << node << '\n';

            unsigned int required = info[node].required - 1;
            for (size_t cut_input : info[node].selected_cut->inputs) {
                //std::cout << "Setting required time of node " << cut_input << " to " << std::min(info[cut_input].required, required) << '\n';
                info[cut_input].required = std::min(info[cut_input].required, required);

                // If we end up with a negative required time, we have a bug. For instance, this might fire if:
                // - the maximum depth isn't actually the maximum depth
                // - the graph has a loop
                assert(info[cut_input].required >= 0 && "bug: node has negative arrival time");
            }
        }
        std::cout << "done\n";
    }

    graph<lut> derive_mapping() const
    {
        // The mapping frontier is the list of all nodes which do not have selected cuts.
        // We start with the primary outputs and work downwards.
        std::vector<size_t> frontier;
        std::transform(g.primary_outputs.begin(), g.primary_outputs.end(), std::back_inserter(frontier), [](size_t po) {
            return po;
        });

        std::unordered_map<size_t, size_t> gate_graph_to_lut_graph;
        graph<lut> mapping;

        // Populate the LUT graph with the primary inputs and outputs of the gate graph.
        gate_graph_to_lut_graph.insert({0, mapping.add_constant_zero()});
        gate_graph_to_lut_graph.insert({1, mapping.add_constant_one()});

        for (size_t pi : g.primary_inputs) {
            size_t index = mapping.add_primary_input();
            gate_graph_to_lut_graph.insert({pi, index});
        }

        for (size_t po : g.primary_outputs) {
            size_t index = mapping.add_primary_output();
            gate_graph_to_lut_graph.insert({po, index});
        }

        std::vector<size_t> lut_stats;

        for (int i = 0; i < 7; i++) {
            lut_stats.push_back(0);
        }

        // While there are still nodes to be mapped:
        while (!frontier.empty()) {
            // Pop a node from the mapping frontier.
            size_t node = frontier.back();
            frontier.pop_back();

            // Add the node to the mapping graph.
            if (!g.is_primary_input(node) && !g.is_primary_output(node)) {
                kitty::dynamic_truth_table tt = g.simulate(*info[node].selected_cut);
                size_t index = mapping.add_cell(lut{tt});
                gate_graph_to_lut_graph.insert({node, index});
                lut_stats[info[node].selected_cut->input_count()]++;
            }

            // Add all the inputs in that cut which are not primary inputs or already-discovered nodes to the mapping frontier.
            if (g.is_primary_output(node)) {
                for (size_t input : g.compute_node_fanin_nodes(node)) {
                    frontier.push_back(input);
                    break;
                }
            }

            for (size_t input : info[node].selected_cut->inputs) {
                if (!g.is_primary_input(input) && !gate_graph_to_lut_graph.count(input)) {
                    frontier.push_back(input);
                }
            }
        }

        // Walk the frontier again, but this time populating the graph with connections.
        std::transform(g.primary_outputs.begin(), g.primary_outputs.end(), std::back_inserter(frontier), [](size_t po) {
            return po;
        });

        std::unordered_set<size_t> visited;

        while (!frontier.empty()) {
            // Pop a node from the mapping frontier.
            size_t node = frontier.back();
            frontier.pop_back();

            visited.insert(node);

            if (g.is_primary_output(node)) {
                for (size_t input : g.compute_node_fanin_nodes(node)) {
                    frontier.push_back(input);
                    mapping.add_connection(gate_graph_to_lut_graph.at(input), gate_graph_to_lut_graph.at(node));
                    break;
                }
            } else {
                for (size_t input : info[node].selected_cut->inputs) {
                    if (!g.is_primary_input(input) && !visited.count(input)) {
                        frontier.push_back(input);
                    }
                    mapping.add_connection(gate_graph_to_lut_graph.at(input), gate_graph_to_lut_graph.at(node));
                }
            }
        }

        size_t total_luts = 0;
        for (int lut_size = 1; lut_size < 7; lut_size++) {
            std::cout << "LUT" << lut_size << ": " << lut_stats[lut_size] << '\n';
            total_luts += lut_stats[lut_size];
        }
        std::cout << "LUTs: " << total_luts << '\n';

        return mapping;
    }

    std::vector<cut> node_cut_set(size_t node, std::unordered_map<size_t, frontier_info> const& frontier) const
    {
        assert(std::holds_alternative<cell>(g.nodes[node]));
        assert(std::get<cell>(g.nodes[node]).truth_table.size() == 1 && "not implemented: multiple output gates");

        std::vector<size_t> node_inputs{g.compute_node_fanin_nodes(node)};

        // To calculate the cut set of a node, we need to compute the cartesian product of its child cuts.
        // This is implemented as performing a 2-way cartesian product N times.

        // Start with the cut set of input zero.
        if (node_inputs.empty()) {
            std::cout << "node: " << node << '\n';
            throw std::logic_error{"node_cut_set called on node without fanin"};
        }

        std::vector<cut> cut_set{frontier.at(node_inputs[0]).cuts};

        // Append the trivial cut of input zero.
        cut_set.push_back(cut{node_inputs[0]});

        // For each other input:
        if (node_inputs.size() > 1) {
            std::for_each(node_inputs.begin()+1, node_inputs.end(), [&](size_t node_input) {
                if (frontier.find(node_input) == frontier.end()) {
                    throw std::logic_error("bug: mapping frontier does not contain node");
                }

                std::vector<cut> new_cuts;

                // Merge the present cut set with the cuts of this input.
                for (cut const& c : cut_set) {
                    for (int input_cut = 0; input_cut < frontier.at(node_input).cuts.size(); input_cut++) {
                        new_cuts.push_back(c.merge(frontier.at(node_input).cuts[input_cut], node));
                        new_cuts.push_back(c.merge(cut{node_input}, node));
                    }
                }

                // Filter out cuts which exceed the cut input limit.
                new_cuts.erase(std::remove_if(new_cuts.begin(), new_cuts.end(), [=](cut const& candidate) {
                    return candidate.input_count() > settings.cut_input_limit;
                }), new_cuts.end());

                // TODO: is it sound to keep a running total of the N best cuts and prune cuts that are worse than the limit?
                // Or does that negatively affect cut quality?

                // Replace the present cut set with the new one.
                cut_set = std::move(new_cuts);
            });
        } else {
            // When we have only a single input, we end up with the cut set of that input.
            // We need to patch the cut set to set the cut outputs as this node.
            for (cut& c : cut_set) {
                c.output = node;
            }
        }

        // Also include the previous-best cut in the cut set, if it exists, to avoid forgetting good cuts.
        if (info[node].selected_cut.has_value()) {
            cut_set.push_back(*info[node].selected_cut);
        }

        return cut_set;
    }

    // Ordering by cut depth is vital to find the best possible mapping for a network.
    unsigned int cut_depth(cut const& c, std::vector<mapping_info> const& info)
    {
        unsigned int depth = 0;
        for (size_t input : c.inputs) {
            if (info.at(input).depth > depth) {
                depth = info.at(input).depth;
            }
        }
        return depth + 1;
    }

    // It is better to prefer smaller cuts over bigger cuts because it allows more cuts to be mapped
    // for the same network depth.
    unsigned int cut_input_count(cut const& c, std::vector<mapping_info> const& info)
    {
        (void)info;
        return c.input_count();
    }

    // Preferring cuts with lower fanin references aims to reduce mapping duplication
    // where a node is covered by multiple mappings at the same time.
    float cut_fanin_refs(cut const& c, std::vector<mapping_info> const& info)
    {
        float references = 0.0;
        for (size_t input : c.inputs) {
            references += float(info.at(input).references);
        }
        return references / float(c.input_count());
    }

    // Area flow estimates how much this cone of logic is shared within the current mapping.
    float cut_area_flow(cut const& c, std::vector<mapping_info> const& info)
    {
        float sum_area_flow = 0.0f;
        for (size_t input : c.inputs) {
            sum_area_flow += info.at(input).area_flow;
        }
        return (sum_area_flow + 1.0f) / std::max(1.0f, float(info.at(c.output).references));
    }

    // Exact area calculates the number of LUTs that would be added to the mapping if this cut was selected.
    unsigned int cut_exact_area(cut const& c)
    {
        if (info.at(c.output).selected_cut.has_value()) {
            if (c == *info.at(c.output).selected_cut) {
                auto area2 = exact_area_ref(c);
                auto area1 = exact_area_deref(c);

                if (area1 != area2) {
                    throw std::logic_error("bug: mismatch between number of nodes referenced and dereferenced");
                }

                return area1;
            }
        }

        auto area2 = exact_area_ref(c);
        auto area1 = exact_area_deref(c);

        if (area1 != area2) {
            throw std::logic_error("bug: mismatch between number of nodes referenced and dereferenced");
        }

        return area1;
    }

    unsigned int exact_area_deref(cut const& c)
    {
        unsigned int area = 1;

        for (size_t cut_input : c.inputs) {
            if (std::holds_alternative<cell>(g.nodes[cut_input])) {
                if (info.at(cut_input).references <= 0) {
                    std::cout << "At node " << cut_input << ":\n";
                    throw std::logic_error{"bug: decremented node reference below zero"};
                }

                info.at(cut_input).references--;
                if (info.at(cut_input).references == 0) {
                    area += exact_area_deref(*info.at(cut_input).selected_cut);
                }
            }
        }

        return area;
    }

    unsigned int exact_area_ref(cut const& c)
    {
        unsigned int area = 1;

        for (size_t cut_input : c.inputs) {
            if (std::holds_alternative<cell>(g.nodes[cut_input])) {
                if (info.at(cut_input).references == 0) {
                    area += exact_area_ref(*info.at(cut_input).selected_cut);
                }
                info.at(cut_input).references++;
            }
        }

        return area;
    }

    void cut_deref(cut const& c)
    {
        for (size_t cut_input : c.inputs) {
            if (std::holds_alternative<cell>(g.nodes[cut_input])) {
                if (info.at(cut_input).references <= 0) {
                    throw std::logic_error{"bug: decremented node reference below zero"};
                }

                info.at(cut_input).references--;
                info.at(cut_input).area_flow = cut_area_flow(*info.at(cut_input).selected_cut, info);
                if (info.at(cut_input).references == 0) {
                    cut_deref(*info.at(cut_input).selected_cut);
                }
            }
        }
    }

    void cut_ref(cut const& c)
    {
        for (size_t cut_input : c.inputs) {
            if (std::holds_alternative<cell>(g.nodes[cut_input])) {
                if (info.at(cut_input).references == 0) {
                    cut_ref(*info.at(cut_input).selected_cut);
                }
                info.at(cut_input).references++;
                info.at(cut_input).area_flow = cut_area_flow(*info.at(cut_input).selected_cut, info);
            }
        }
    }

    graph<cell> g;
    std::vector<mapping_info> info;
    mapping_settings settings;
};


template<class Ntk>
graph<cell> mockturtle_to_lut_graph(Ntk const& input_ntk)
{
    static_assert(mockturtle::is_network_type_v<Ntk>);
    static_assert(mockturtle::has_foreach_pi_v<Ntk>);
    static_assert(mockturtle::has_foreach_po_v<Ntk>);
    static_assert(mockturtle::has_foreach_node_v<Ntk>);
    static_assert(mockturtle::has_cell_function_v<Ntk>);

    mockturtle::klut_network ntk = mockturtle::gates_to_nodes<mockturtle::klut_network, Ntk>(input_ntk);
    graph<cell> g{};
    std::unordered_map<mockturtle::klut_network::node, size_t> mockturtle_to_node{};

    mockturtle_to_node.insert({ntk.get_node(ntk.get_constant(false)), g.add_constant_zero()});
    mockturtle_to_node.insert({ntk.get_node(ntk.get_constant(true)), g.add_constant_one()});

    ntk.foreach_pi([&](mockturtle::klut_network::node const& node, uint32_t index) -> void {
        size_t pi = g.add_primary_input();
        mockturtle_to_node.insert({node, pi});
    });

    ntk.foreach_node([&](mockturtle::klut_network::node const& node) -> void {
        if (!ntk.is_constant(node)) {
            std::vector<kitty::dynamic_truth_table> truth_table{};
            truth_table.push_back(ntk.node_function(node));
            size_t c = g.add_cell(cell{truth_table});
            mockturtle_to_node.insert({node, c});
            ntk.foreach_fanin(node, [&](mockturtle::klut_network::signal const& fanin) -> void {
                g.add_connection(mockturtle_to_node.at(ntk.get_node(fanin)), mockturtle_to_node.at(node));
            });
        }
    });

    ntk.foreach_po([&](mockturtle::klut_network::signal const& signal, uint32_t index) -> void {
        size_t po = g.add_primary_output();
        g.add_connection(mockturtle_to_node.at(ntk.get_node(signal)), po);
    });

    mockturtle::write_blif(ntk, "c432.before.blif");

    //g.dump_to_stdout();

    return g;
}

mockturtle::klut_network lut_graph_to_mockturtle(graph<lut> const& g)
{
    mockturtle::klut_network ntk{};
    std::unordered_map<size_t, mockturtle::klut_network::signal> node_to_mockturtle{};

    //g.dump_to_stdout();

    node_to_mockturtle.insert({0, ntk.get_constant(0)});
    node_to_mockturtle.insert({1, ntk.get_constant(1)});

    for (size_t pi : g.primary_inputs) {
        node_to_mockturtle.insert({pi, ntk.create_pi()});
    }

    for (size_t node : g.compute_topological_ordering()) {
        std::vector<mockturtle::klut_network::signal> children{};
        for (size_t input : g.compute_node_fanin_nodes(node)) {
            children.push_back(node_to_mockturtle.at(input));
        }
        if (!g.is_primary_input(node) && !g.is_primary_output(node) && !g.is_constant(node)) {
            node_to_mockturtle.insert({node, ntk.create_node(children, std::get<lut>(g.nodes[node]).truth_table)});
        }
    }

    for (size_t po : g.primary_outputs) {
        for (size_t fanin : g.compute_node_fanin_nodes(po)) {
            // BUG: exporting to MIG breaks here; for later.
            if (node_to_mockturtle.find(fanin) == node_to_mockturtle.end()) {
                std::cout << "Node " << fanin << " not in node_to_mockturtle\n";
            }
            node_to_mockturtle.insert({po, ntk.create_po(node_to_mockturtle.at(fanin))});
            break;
        }
    }

    mockturtle::write_blif(ntk, "c432.after.blif");

    return ntk;
}

} // namespace techmap

} // namespace oracle
