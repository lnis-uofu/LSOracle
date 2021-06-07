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

namespace oracle
{
namespace techmap
{

struct cut {
    cut(size_t node) :
        inputs{std::vector<size_t>{node}}, output{node}
    {
    }

    cut(std::vector<size_t> _inputs, size_t _output) :
        inputs{std::move(_inputs)}, output{_output}
    {
        std::sort(inputs.begin(), inputs.end());
    }

    cut merge(cut const& rhs, size_t new_output) const
    {
        std::vector<size_t> new_inputs;
        std::set_union(inputs.begin(), inputs.end(), rhs.inputs.begin(), rhs.inputs.end(), std::back_inserter(new_inputs));
        return cut(std::move(new_inputs), new_output);
    }

    bool input_count() const
    {
        return inputs.size();
    }

    bool is_trivial() const
    {
        return inputs.size() == 1 && inputs[0] == output;
    }

    std::vector<size_t> inputs;
    size_t output;
};


struct cell {
    size_t index;
};


struct lut {
    unsigned long long lut_mask;
};


struct primary_input {
    primary_input(size_t index) :
        index{index}
    {
    }

    size_t index;
};


struct primary_output {
    primary_output(size_t index) :
        index{index}
    {
    }

    size_t index;
};


struct connection {
    connection(size_t from, size_t to, size_t index) :
        from{from}, to{to}, index{index}
    {
    }

    size_t from;
    size_t to;
    size_t index;
};


template<class cell_type>
struct graph {
    size_t add_primary_input()
    {
        size_t index = nodes.size();
        primary_input pi{index};
        nodes.push_back(pi);
        primary_inputs.push_back(index);
        return index;
    }

    bool is_primary_input(size_t index) const
    {
        return std::holds_alternative<primary_input>(nodes[index]);
    }

    size_t add_primary_output()
    {
        size_t index = nodes.size();
        primary_output po{index};
        nodes.push_back(po);
        primary_outputs.push_back(index);
        return index;
    }

    bool is_primary_output(size_t index) const
    {
        return std::holds_alternative<primary_output>(nodes[index]);   
    }

    size_t add_cell(cell_type const& c)
    {
        size_t index = nodes.size();
        nodes.push_back(c);
        return index;
    }

    size_t add_connection(size_t from, size_t to)
    {
        auto index = connections.size();
        connections.push_back(connection(from, to, index));
        return index;
    }

    void remove_connection(size_t index)
    {
        connections[index].reset();
    }

    std::vector<connection> node_fanin_connections(size_t node) const
    {
        std::vector<connection> fanin;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->to == node) {
                fanin.push_back(*conn);
            }
        }
        return fanin;
    }

    std::vector<size_t> node_fanin_nodes(size_t node) const
    {
        std::vector<size_t> fanin;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->to == node) {
                fanin.push_back(conn->from);
            }
        }
        return fanin;
    }

    std::vector<connection> node_fanout_connections(size_t node) const
    {
        std::vector<connection> fanout;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->from == node) {
                fanout.push_back(*conn);
            }
        }
        return fanout;
    }

    std::vector<size_t> node_fanout_nodes(size_t node) const
    {
        std::vector<size_t> fanout;
        for (std::optional<connection> const& conn : connections) {
            if (conn && conn->from == node) {
                fanout.push_back(conn->to);
            }
        }
        return fanout;
    }

    std::vector<size_t> topological_ordering() const
    {
        std::vector<size_t> ordering;
        std::vector<size_t> no_incoming{primary_inputs};
        graph g{*this};

        while (!no_incoming.empty()) {
            size_t node = no_incoming.back();
            ordering.push_back(node);
            no_incoming.pop_back();
            for (connection conn : g.node_fanout_connections(node)) {
                g.remove_connection(conn.index);
                if (g.node_fanin_connections(conn.to).empty()) {
                    no_incoming.push_back(conn.to);
                }
            }
        }

        // TODO: if `g` still has edges this is a cyclic graph, which cannot be topologically ordered.

        return ordering;
    }

    std::vector<size_t> reverse_topological_ordering() const
    {
        std::vector<size_t> ordering;
        std::vector<size_t> no_outgoing{primary_outputs};
        graph g{*this};

        while (!no_outgoing.empty()) {
            size_t node = no_outgoing.back();
            ordering.push_back(node);
            no_outgoing.pop_back();
            for (connection conn : g.node_fanin_connections(node)) {
                g.remove_connection(conn.index);
                if (g.node_fanout_connections(conn.to).empty()) {
                    no_outgoing.push_back(conn.to);
                }
            }
        }

        // TODO: if `g` still has edges this is a cyclic graph, which cannot be topologically ordered.

        return ordering;
    }

    std::vector<size_t> primary_inputs;
    std::vector<size_t> primary_outputs;
    std::vector<std::variant<primary_input, primary_output, cell_type>> nodes;
    std::vector<std::optional<connection>> connections;
};


struct mapping_settings {
    unsigned int cut_input_limit;
    unsigned int node_cut_count;
};


struct mapping_info {
    std::optional<cut> selected_cut;
    unsigned int depth;
    unsigned int references;
    float area_flow;
    unsigned int required;
};


struct frontier_info {
    frontier_info(size_t node) :
        cuts{std::vector<cut>{cut{node}}}
    {
    }

    frontier_info(std::vector<cut> _cuts) :
        cuts{std::move(_cuts)}
    {
    }

    std::vector<cut> cuts;
};


class mapper {
public:
    mapper(graph<cell> _g, mapping_settings _settings) :
        g{std::move(_g)}, info(g.nodes.size()), settings{_settings}
    {
        assert(settings.cut_input_limit >= 2 && "invalid argument: mapping for less than 2 inputs is impossible");
        assert(settings.node_cut_count >= 1 && "invalid argument: must store at least one cut per node");

        std::fill(info.begin(), info.end(), mapping_info{});
    }

    graph<lut> map()
    {
        // Mapping phase 1: prioritise depth.
        enumerate_cuts(cut_depth, cut_input_count, cut_area_flow, false);

        // Mapping phase 2: prioritise global area.
        enumerate_cuts(cut_area_flow, cut_fanin_refs, cut_depth, true);

        // Mapping phase 3: prioritise local area.
        enumerate_cuts(cut_exact_area, cut_fanin_refs, cut_depth, true);

        // Mapping phase 4: prioritise global area.
        enumerate_cuts(cut_area_flow, cut_fanin_refs, cut_depth, true);

        // Mapping phase 5: prioritise local area.
        enumerate_cuts(cut_exact_area, cut_fanin_refs, cut_depth, true);

        // Derive the final mapping of the network.
        return derive_mapping();
    }

private:
    template<class F1, class F2, class F3>
    void enumerate_cuts(F1 sort1, F2 sort2, F3 sort3, bool area_optimisation)
    {
        std::unordered_map<size_t, frontier_info> frontier;

        // TODO: ABC computes the graph crosscut to pre-allocate frontier memory.

        // Initialise frontier with the trivial cuts of primary inputs.
        for (size_t pi : g.primary_inputs) {
            frontier.insert({pi, frontier_info{pi}});
        }

        for (size_t node : g.topological_ordering()) {
            // Skip primary inputs.
            if (g.is_primary_input(node)) {
                continue;
            }

            // Find the node cut set.
            std::vector<cut> cut_set = node_cut_set(node, frontier);

            // Prune cuts which exceed the node slack in area optimisation mode.
            if (area_optimisation) {
                cut_set.erase(std::remove_if(cut_set.begin(), cut_set.end(), [&](cut const& c) {
                    return cut_depth(c, info) > info[node].required;
                }), cut_set.end());

                // Because the previous cut is included in the set and must meet required times
                // we must have at least one cut left from this.
                assert(!cut_set.empty() && "bug: erased all cuts from a node");
            }

            // Sort the cuts by desired characteristics.
            std::sort(cut_set.begin(), cut_set.end(), [&](cut const& a, cut const& b) {
                return sort1(a, info) < sort1(b, info) ||
                    (sort1(a, info) == sort1(b, info) && sort2(a, info) < sort2(b, info)) ||
                    (sort1(a, info) == sort1(b, info) && sort2(a, info) == sort2(b, info) && sort3(a, info) < sort3(b, info));
            });

            // Keep only the specified good cuts.
            if (cut_set.size() > settings.node_cut_count) {
                cut_set.erase(cut_set.begin()+settings.node_cut_count, cut_set.end());
            }

            // We should have at least one cut provided by the trivial cut.
            assert(!cut_set.empty() && "bug: node has no cuts"); // TODO: maybe this is redundant given the assert in area_optimisation?

            // Choose the best cut as the representative cut for this node, and add the cut set of this node to the frontier.
            for (size_t input : cut_set[0].inputs) {
                frontier.at(input).references++;
            }

            unsigned int depth = cut_depth(cut_set[0], frontier);
            info[node].selected_cut = std::make_optional(cut_set[0]);
            frontier.insert({node, frontier_info{std::move(cut_set), depth}});

            // Erase fan-in nodes that have their fan-out completely mapped as they will never be used again.
            for (size_t fanin_node : g.node_fanin_nodes(node)) {
                std::vector<size_t> node_fanout{g.node_fanout_nodes(fanin_node)};
                if (std::all_of(node_fanout.begin(), node_fanout.end(), [&](size_t node) {
                    return info[node].selected_cut.has_value();
                })) {
                    frontier.erase(node);
                }
            }
        }

        // After mapping, recalculate node slacks.
        recalculate_slack();
    }

    void recalculate_slack()
    {
        // First find the maximum depth of the mapping.
        unsigned int max_depth = std::max_element(info.begin(), info.end(), [&](mapping_info const& a, mapping_info const& b) {
            return a.depth < b.depth;
        })->depth;

        // Next, initialise the node required times.
        for (mapping_info& node : info) {
            node.required = UINT_MAX;
        }

        // Set the required times of the primary outputs to the maximum depth.
        for (size_t po : g.primary_outputs) {
            info[po].required = max_depth;
        }

        // Then work from PO to PI, propagating required times.
        for (size_t node : g.reverse_topological_ordering()) {
            unsigned int required = info[node].required - 1;
            for (size_t cut_input : info[node].selected_cut->inputs) {
                info[cut_input].required = std::min(info[cut_input].required, required);

                // If we end up with a negative required time, we have a bug. For instance:
                // - the maximum depth isn't actually the maximum depth
                // - the graph has a loop
                // - the 
                assert(info[cut_input].required >= 0 && "bug: node has negative arrival time");
            }
        }
    }

    graph<lut> derive_mapping()
    {
        // The mapping frontier is the list of all nodes which do not have selected cuts.
        // We start with the primary outputs and work downwards.
        std::vector<size_t> frontier;
        std::transform(g.primary_outputs.begin(), g.primary_outputs.end(), std::back_inserter(frontier), [](primary_output const& po) {
            return po.index;
        });

        std::unordered_map<size_t, size_t> gate_graph_to_lut_graph;
        graph<lut> mapping;

        // Populate the LUT graph with the primary inputs and outputs of the gate graph.
        for (size_t pi : g.primary_inputs) {
            size_t index = mapping.add_primary_input();
            gate_graph_to_lut_graph.insert({pi, index});
        }

        for (size_t po : g.primary_outputs) {
            size_t index = mapping.add_primary_output();
            gate_graph_to_lut_graph.insert({po, index});
        }

        // While there are still nodes to be mapped:
        while (!frontier.empty()) {
            // Pop a node from the mapping frontier.
            size_t node = frontier.back();
            frontier.pop_back();

            // Add the node to the mapping graph.
            if (!g.is_primary_input(node) && !g.is_primary_output(node)) {
                size_t index = mapping.add_cell(lut{});
                gate_graph_to_lut_graph.insert({node, index});
            }

            // Add all the inputs in that cut which are not primary inputs or already-discovered nodes to the mapping frontier.
            for (size_t input : info[node].selected_cut->inputs) {
                if (!g.is_primary_input(input) && !gate_graph_to_lut_graph.count(input)) {
                    frontier.push_back(input);
                }
            }
        }

        // Walk the frontier again, but this time populating the graph with connections.
        std::transform(g.primary_outputs.begin(), g.primary_outputs.end(), std::back_inserter(frontier), [](primary_output const& po) {
            return po.index;
        });

        std::unordered_set<size_t> visited;

        while (!frontier.empty()) {
            // Pop a node from the mapping frontier.
            size_t node = frontier.back();
            frontier.pop_back();

            visited.insert(node);

            for (size_t input : info[node].selected_cut->inputs) {
                if (!g.is_primary_input(input) && !visited.count(input)) {
                    frontier.push_back(input);
                }

                mapping.add_connection(gate_graph_to_lut_graph.at(input), gate_graph_to_lut_graph.at(node));
            }
        }

        return mapping;
    }

    std::vector<cut> node_cut_set(size_t node, std::unordered_map<size_t, frontier_info> const& frontier) const
    {
        std::vector<size_t> node_inputs{g.node_fanin_nodes(node)};

        // The trivial cut of a node is just the node itself.
        cut trivial_cut{node_inputs, node};

        // To calculate the cut set of a node, we need to compute the cartesian product of its child cuts.
        // This is implemented as performing a 2-way cartesian product N times.

        // Start with the cut set of input zero.
        std::vector<cut> cut_set{frontier.at(node_inputs[0]).cuts};

        // For each other input:
        std::for_each(node_inputs.begin()+1, node_inputs.end(), [&](size_t node_input) {
            std::vector<cut> new_cuts;
            
            // Merge the present cut set with the cuts of this input.
            for (cut c : cut_set) {
                std::transform(frontier.at(node_input).cuts.begin(), frontier.at(node_input).cuts.end(), std::back_inserter(new_cuts), [&](cut const& input_cut) {
                    return c.merge(input_cut, node);
                });
            }

            // Filter out cuts which exceed the cut input limit.
            new_cuts.erase(std::remove_if(new_cuts.begin(), new_cuts.end(), [=](cut const& candidate) {
                return candidate.input_count() > settings.cut_input_limit;
            }), new_cuts.end());

            // Replace the present cut set with the new one.
            cut_set = std::move(new_cuts);
        });

        // Include the trivial cut in the cut set.
        cut_set.push_back(std::move(trivial_cut));

        // Also include the previous-best cut in the cut set, if it exists, to avoid forgetting good cuts.
        if (info[node].selected_cut.has_value()) {
            cut_set.push_back(*info[node].selected_cut);
        }

        return cut_set;
    }

    // Ordering by cut depth is vital to find the best possible mapping for a network.
    static unsigned int cut_depth(cut const& c, std::vector<mapping_info> const& info)
    {
        return *std::max_element(c.inputs.begin(), c.inputs.end(), [&](size_t a, size_t b) {
            return info.at(a).depth < info.at(b).depth;
        }) + 1;
    }

    // It is better to prefer smaller cuts over bigger cuts because it allows more cuts to be mapped
    // for the same network depth.
    static unsigned int cut_input_count(cut const& c, std::vector<mapping_info> const& info)
    {
        (void)info;
        return c.input_count();
    }

    // Preferring cuts with lower fanin references aims to reduce mapping duplication
    // where a node is covered by multiple mappings at the same time.
    static float cut_fanin_refs(cut const& c, std::vector<mapping_info> const& info)
    {
        return std::transform_reduce(c.inputs.begin(), c.inputs.end(), 0.0, std::plus<>(), [&](size_t input) {
            return float(info.at(input).references);
        }) / float(c.input_count());
    }

    // Area flow estimates how much this cone of logic is shared within the current mapping.
    static float cut_area_flow(cut const& c, std::vector<mapping_info> const& info)
    {
        return std::transform_reduce(c.inputs.begin(), c.inputs.end(), 1.0f, std::plus<>(), [&](size_t input) {
            return float(info.at(input).area_flow) / std::max(1.0f, float(info.at(input).references));
        });
    }

    // Exact area calculates the number of LUTs that would be added to the mapping if this cut was selected.
    static unsigned int cut_exact_area(cut const& c, std::vector<mapping_info>& info)
    {
        unsigned int area = 1;

        for (size_t input : c.inputs) {
            info[input].references--;
        }

        for (size_t input : c.inputs) {
            if (info[input].references == 0 && !info[input].selected_cut->is_trivial()) {
                area += cut_exact_area(*info[input].selected_cut, info);
            }
        }

        for (size_t input : c.inputs) {
            info[input].references++;
        }

        return area;
    }

    graph<cell> g;
    std::vector<mapping_info> info;
    mapping_settings settings;
};

} // namespace techmap

} // namespace oracle
