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
#include "algorithms/output/xmg_techmap.hpp"

namespace oracle
{
void xmg_techmap(xmg_mapping &ntk)
{
    mockturtle::fanout_view fanout(ntk);
    mockturtle::topo_view topo(ntk);
    ntk.clear_visited();
    uint32_t mapped = 0;
    uint32_t covered = 0;
    uint32_t total = 0;
    uint32_t oversize = 0;
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
    std::cout << "mapped " << mapped << std::endl;
    std::cout << "covered " << covered << std::endl;
    std::cout << (mapped + covered) << "/" << total << " " << ((
                  mapped + covered) / total) << std::endl;
    std::cout << "oversized " << oversize << std::endl;
}
}
