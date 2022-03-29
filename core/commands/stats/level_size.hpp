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

#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

namespace alice
{
class level_size_command : public alice::command
{

public:
    explicit level_size_command(const environment::ptr &env)
        : command(env, "Displays a histogram of fanout of the stored network")
    {
        add_flag("--aig,-a", "Display stats for stored AIG (AIG is default)");
        add_flag("--mig,-m", "Display stats for stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG (AIG is default)");
        add_flag("--xmg,-g", "Display stats for stored XMG (AIG is default)");
    }

    template <typename ntk> void dump_stats(string name)
    {
        if (store<std::shared_ptr<mockturtle::names_view<ntk>>>().empty()) {
            env->err() << name << " network not stored\n";
            return;
        }
        auto dag =
            *store<std::shared_ptr<mockturtle::names_view<ntk>>>().current();

	mockturtle::depth_view dag_view(dag);
	vector<uint32_t> levels(dag_view.depth(), 0);
	dag_view.foreach_node([&dag_view, &levels](auto n) {
	    levels[dag_view.level(n)]++;
	});
	env->out() << "Nodes per level" << std::endl;
	env->out() << "Level\tNodes" << std::endl;
	for (size_t i = 0; i < levels.size(); i++) {
	    env->out() << i << "\t" << levels[i] << std::endl;
	}
    }
protected:
    void execute()
    {
        if (is_set("mig")) {
            dump_stats<mockturtle::mig_network>("MIG");
        } else if (is_set("xag")) {
            dump_stats<mockturtle::xag_network>("XAG");
        } else if (is_set("xmg")) {
            dump_stats<mockturtle::xmg_network>("XMG");
        } else {
            dump_stats<mockturtle::aig_network>("AIG");
        }
    }
private:
};

ALICE_ADD_COMMAND(level_size, "Stats");
}
