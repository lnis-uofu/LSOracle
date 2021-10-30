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
/*
#include "algorithms/output/xmg_techmap.hpp"

namespace oracle
{
    // find exact matches
    //
    // foreach node in topo order (maybe should be reverse...), if not visited
    //   if:
    //     all fanins at same level, one below the node.
    //     the fanins fanout to at most two nodes including this one
    //     fanout of node goes to the next level. optional second fanout goes to the next level.
    //     and the total distinct fanins <= 6, all at the level below the node.
    //   then: mark as mapped root, mark children as visited.
    //     if only two fanin, mark fractured.
    void map_exact(xmg_mapping &ntk,
			mockturtle::fanout_view &fanout,
			mockturtle::topo_view &topo,
			uint32_t &mapped,
			uint32_t &covered,
			uint32_t &total,
			uint32_t &oversize
			)
    {
	topo.foreach_node([&](auto node) {
	    total++;
	    if (ntk.visited(node)) return;
	    std::set<typename xmg_mapping::node> fanin;
	    std::set<typename xmg_mapping::signal> fanin_fanouts;
	    topo.foreach_fanin(node, [&](auto s) {
		auto fin = ntk.get_node(s);
		if (ntk.is_constant(fin)) return;
		fanin.insert(fin);
		fanout.foreach_fanout(fin, [&](auto fout) {
		    fanin_fanouts.insert(fout);
		});
	    });
	    if (fanin_fanouts.size() <= 2 && fanin.size() <= 6) {
		ntk.add_to_mapping(node, fanin.begin(), fanin.end());
		mapped++;
		for (auto fin : fanin) {
		    ntk.set_visited(fin, 1);
		    covered++;
		}
	    } else if (exclusive && fanin.size() > 6) {
		oversize++;
	    }
	});

    }

    // pack fractured.
    //
    // foreach unvisited node in reverse topo:
    //   if:
    //     has exactly one fanout to a mapped group
    //     fanout skips a level.
    //     find a fanin of the mapped group that is mapped as fractured, and that has combined distinct fanins <= 6.
    //   then: add to mapping and mark visited.
    void pack_fractured(xmg_mapping &ntk,
			mockturtle::fanout_view &fanout,
			mockturtle::topo_view &topo,
			uint32_t &mapped,
			uint32_t &covered,
			uint32_t &total,
			uint32_t &oversize
			)
    {
	topo.foreach_node([&](auto node) {
	    total++;
	    if (ntk.visited(node)) return;
	    bool exclusive = true;
	    std::set<typename xmg_mapping::node> fanin;
	    topo.foreach_fanin(node, [&](auto s) {
		auto fin = ntk.get_node(s);
		if (ntk.is_constant(fin)) return true;
		fanin.insert(fin);
		fanout.foreach_fanout(fin, [&](auto fout) {
		    exclusive = exclusive && fout == node;
		});
		return exclusive;
	    });
	    if (exclusive && fanin.size() <= 6) {
		ntk.add_to_mapping(node, fanin.begin(), fanin.end());
		mapped++;
		for (auto fin : fanin) {
		    ntk.set_visited(fin, 1);
		    covered++;
		}
	    } else if (exclusive && fanin.size() > 6) {
		oversize++;
	    }
	});

    }
void xmg_techmap(xmg_mapping &ntk)
{
    mockturtle::fanout_view fanout(ntk);
    mockturtle::topo_view topo(ntk);
    ntk.clear_visited();
    uint32_t mapped = 0;
    uint32_t covered = 0;
    uint32_t total = 0;
    uint32_t oversize = 0;
    map_exact(ntk, fanout, topo, mapped, covered, total, oversize);
    pack_fractured(ntk, fanout, topo, mapped, covered, total, oversize);
    std::cout << "mapped " << mapped << std::endl;
    std::cout << "covered " << covered << std::endl;
    std::cout << (mapped + covered) << "/" << total << " " << ((
                  mapped + covered) / total) << std::endl;
    std::cout << "oversized " << oversize << std::endl;
}
}
*/
