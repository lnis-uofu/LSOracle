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

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

#include "utility.hpp"

namespace alice
{
class critical_path_command : public alice::command
{

public:
    explicit critical_path_command(const environment::ptr &env)
        : command(env, "Determines the function of nodes along the critical path.")
    {
        add_flag("--mig,-m", "Display stats for stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG (AIG is default)");
        add_flag("--xmg,-g", "Display stats for stored XMG (AIG is default)");
        add_flag("--aig,-a", "Display stats for stored AIG (AIG is default)");
    }

protected:
    template <typename network>
    void dump_stats(std::string name)
    {
        if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
	    spdlog::error("{} network not stored", name);
	    return;
	}
	auto ntk = *store<std::shared_ptr<mockturtle::names_view<network>>>().current();

	oracle::slack_view<mockturtle::names_view<network>> slack{ntk};
	auto critical_path = slack.get_critical_path(ntk);

	oracle::function_counts counts = oracle::node_functions(ntk);
	for (auto curr_node : critical_path) {
	    update_counts(counts, ntk, curr_node);
	}
	spdlog::info("Critical path size = {}",critical_path.size() );
    spdlog::info("MAJ nodes on critical path = {}",counts.maj_num );
    spdlog::info("AND nodes on critical path = {}",counts.and_num );
    spdlog::info("OR nodes on critical path = {}",counts.or_num );
    spdlog::info("XOR3 nodes on critical path = {}",counts.xor3_num );
    spdlog::info("XOR nodes on critical path = {}",counts.xor_num );
    spdlog::info("XNOR nodes on critical path = {}",counts.xnor_num );
    spdlog::info("unknown nodes on critical path = {}",counts.unknown_num );
    spdlog::info("INPUTS on critical path = {}",counts.input_num);
    }

    void execute()
    {
        if (is_set("xag")) {
            dump_stats<mockturtle::xag_network>("XAG");
        } else if (is_set("xmg")) {
            dump_stats<mockturtle::xmg_network>("XMG");
        } else if (is_set("mig")) {
            dump_stats<mockturtle::mig_network>("MIG");
        } else {
	    dump_stats<mockturtle::aig_network>("AIG");
	}
    }
private:
};

ALICE_ADD_COMMAND(critical_path, "Stats");
}
