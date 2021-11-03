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
#include <nlohmann/json.hpp>

#include "utility.hpp"

namespace alice
{
class write_stats_command : public alice::command
{
    using json = nlohmann::json;

public:
    explicit write_stats_command(const environment::ptr &env)
        : command(env, "Output statistics in json format.")
    {
	add_option("--output,output", file, "Filename");
	add_option("--name", design, "Design name");
        add_flag("--mig,-m", "Display stats for stored MIG (AIG is default)");
        add_flag("--xag,-x", "Display stats for stored XAG");
        add_flag("--xmg,-g", "Display stats for stored XMG");
        add_flag("--aig,-a", "Display stats for stored AIG");
    }

protected:
    template <typename network, typename original>
    void dump_stats(std::string name)
    {
        if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
	    env->err() << name << " network not stored\n";
	    return;
	}

	json stats;

	auto ntk = *store<std::shared_ptr<mockturtle::names_view<network>>>().current();

	// Basic stats.
	stats["module"] = ntk.get_network_name();
	stats["design"] = design;
	stats["graph"] = name;
	stats["size"] = ntk.size();
	stats["registers"] = ntk.num_latches();
	stats["inputs"] = ntk.num_pis();
	stats["outputs"] = ntk.num_pos();

	mockturtle::depth_view depth_view(ntk);
	stats["depth"] = depth_view.depth();

	oracle::function_counts counts = oracle::node_functions(ntk);
	write_nodes(stats["nodes"], counts);

	// preoptimized stats
	// TODO make optional
	if (store<std::shared_ptr<mockturtle::names_view<original>>>().empty()) {
	    env->err() << "Original AIG network not stored\n";
	} else {
	    auto orig = *store<std::shared_ptr<mockturtle::names_view<original>>>().current();
	    mockturtle::depth_view depth_orig(orig);
	    stats["preoptimization"]["graph"] = "AIG";
	    stats["preoptimization"]["depth"] = depth_orig.depth();
	    stats["preoptimization"]["size"] = orig.size();
	    oracle::function_counts orig_counts = oracle::node_functions(orig);
	    write_nodes(stats["preoptimization"]["nodes"], orig_counts);
	}

	// Optimization stats
	// TODO store optimization runtime and type.


	if (!(store<std::shared_ptr<oracle::partition_manager<mockturtle::names_view<original>>>>().empty())) {
	    auto partman = *store<std::shared_ptr<oracle::partition_manager<mockturtle::names_view<original>>>>().current();
	    stats["optimization"]["partitions"]["number"] = partman.get_part_num();
	}

	oracle::slack_view<mockturtle::names_view<network>> slack(ntk);
	auto critical_path = slack.get_critical_path(ntk);

	oracle::function_counts critical_counts = oracle::node_functions(ntk);
	for (auto curr_node : critical_path) {
	    update_counts(critical_counts, ntk, curr_node);
	}
	write_nodes(stats["critical_path"]["nodes"], critical_counts);

	// TODO write techmap library
	// TODO write timing.
	// TODO write power
	// TODO write techmapped area, gate counts
	
	std::ofstream output(file);
	output << stats;
	output.close();

	env->out() << "Written to " << file << std::endl;
    }

    void execute()
    {
        if (is_set("xag")) {
            dump_stats<mockturtle::xag_network, mockturtle::aig_network>("XAG");
        } else if (is_set("xmg")) {
            dump_stats<mockturtle::xmg_network, mockturtle::aig_network>("XMG");
        } else if (is_set("mig")) {
            dump_stats<mockturtle::mig_network, mockturtle::aig_network>("MIG");
        } else {
	    dump_stats<mockturtle::aig_network, mockturtle::aig_network>("AIG");
	}
    }
private:
    void write_nodes(json &j, oracle::function_counts &c) {
	j["MAJ3"] = c.maj_num;
	j["XOR"] = c.xor_num;
	j["XNOR"] = c.xnor_num;
	j["XOR3"] = c.xor3_num;
	j["AND"] = c.and_num;
	j["OR"] = c.or_num;
	j["UNKNOWN"] = c.unknown_num;
    }
    string file;
    string design;
};

ALICE_ADD_COMMAND(write_stats, "Stats");
}
