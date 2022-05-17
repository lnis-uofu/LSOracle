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
#include <cassert>
#include <mockturtle/mockturtle.hpp>

#include <libkahypar.h>

namespace oracle
{

template<typename Ntk>
class level_partition_manager : public Ntk
{
public:
    using storage = typename Ntk::storage;
    using node = typename Ntk::node;
    using signal = typename Ntk::signal;

public:

    level_partition_manager() {}

    level_partition_manager(Ntk const &ntk) : Ntk(ntk)
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

        oracle::partition_manager<Ntk> partitions(ntk, 2);
        std::set<node> shared_io = partitions.get_shared_io(0, 1);

        levels.clear();
        level_idx = 1;
        std::unordered_map<node, bool> visited;
        ntk.foreach_node([&](auto node) {
            visited[node] = false;
        });
        spdlog::info("Cutset Variables = {";
        typename std::set<node>::iterator it;
        for (it = shared_io.begin(); it != shared_io.end(); ++it) {
            spdlog::info(*it << " ";
            visited[*it] = true;
        }
        spdlog::info("}");
        levels[0] = shared_io;
        compute_level_nodes(ntk, shared_io, visited);
        spdlog::info("Number of levels = " << levels.size());
        for (auto level_it = levels.rbegin(); level_it != levels.rend(); ++level_it) {
            std::set<node> curr_level = level_it->second;
            spdlog::info("\nLevel " << level_it->first << " = {";
            for (it = curr_level.begin(); it != curr_level.end(); ++it) {
                spdlog::info(*it << " ";
            }
            spdlog::info("}");
        }
    }

    oracle::partition_manager<Ntk> generate_partitions(Ntk const &ntk, int coarse)
    {
        int idx = 0;
        int part_idx = 0;
        std::map<node, int> new_partitions;
        for (auto level_it = levels.rbegin(); level_it != levels.rend(); ++level_it) {
            std::set<node> curr_level = level_it->second;
            if (idx != coarse - 1) {
                for (auto it = curr_level.begin(); it != curr_level.end(); ++it) {
                    new_partitions[*it] = part_idx;
                }
                idx++;
            } else {
                for (auto it = curr_level.begin(); it != curr_level.end(); ++it) {
                    new_partitions[*it] = part_idx;
                }
                idx = 0;
                part_idx++;
            }
        }

        int num_partitions = levels.size() / coarse;
        if (levels.size() % coarse != 0) {
            num_partitions++;
        }

        oracle::partition_manager<Ntk> partitions(ntk, new_partitions, num_partitions);
        return partitions;
    }

    int get_levels()
    {
        return levels.size();
    }

    std::set<node> get_level(int lev_idx)
    {
        return levels[lev_idx];
    }

private:
    std::map<int, std::set<node>> levels;
    int level_idx = 1;

    void compute_level_nodes(Ntk const &ntk, std::set<node> prev_level,
                             std::unordered_map<node, bool> visited)
    {
        mockturtle::fanout_view<Ntk> fanout_ntk{ntk};
        std::set<node> curr_level;

        typename std::set<node>::iterator it;
        for (it = prev_level.begin(); it != prev_level.end(); ++it) {
            // spdlog::info("Current node = " << *it);
            fanout_ntk.foreach_fanout(*it, [&](const auto & p) {
                if (curr_level.find(p) == curr_level.end()
                        && prev_level.find(p) == prev_level.end() && !visited[p]) {
                    // spdlog::info("Adding fanout {} to current level set", p);
                    curr_level.insert(p);
                    visited[p] = true;
                }
            });

            ntk.foreach_fanin(*it, [&](const auto & fanin) {
                auto node = ntk.get_node(fanin);
                if (curr_level.find(node) == curr_level.end()
                        && prev_level.find(node) == prev_level.end() && !visited[node]) {
                    // spdlog::info("Adding fanin {} to current level set", node);
                    curr_level.insert(node);
                    visited[node] = true;
                }
            });
        }

        if (!curr_level.empty()) {
            levels[level_idx] = curr_level;
            level_idx++;
            compute_level_nodes(ntk, curr_level, visited);
        }

    }

};
} /* namespace oracle */
