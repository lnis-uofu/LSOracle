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
#include <sys/stat.h>
#include "algorithms/optimization/resynthesis.hpp"

namespace alice
{
class optimize_command : public alice::command
{

public:
    explicit optimize_command(const environment::ptr &env)
        : command(env, "Perform timing driven mixed synthesis.")
    {
        opts.add_option("--abc_exec", abc_exec,
                        "ABC executable, defaults to using path.");
        opts.add_flag("--ndp", "Node Depth Product target");
        opts.add_flag("--nodes", "Node Count target");
        opts.add_flag("--depth", "Depth target");
	opts.add_flag("--resynth", "Resynthesis for depth");
    opts.add_option("--convergence",strategyconvergence, "Reoptimization until convergence with the strategy wanted (balanced, nodes, depth)");
    }
protected:
    void execute()
    {
	synth<mockturtle::aig_network>("AIG");
    }

    template <typename network>
    void synth(string name)
    {   
        auto ntk = *store<std::shared_ptr<mockturtle::names_view<network>>>().current();
        if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
            env->err() << "No " << name << " stored\n";
            return;
        }
        if (store<std::shared_ptr<oracle::partition_manager_junior<network>>>().empty()) {
            env->err() << name << " not partitioned yet\n";
            return;
        }

        oracle::partition_manager_junior<network> partitions_jr =
            *store<std::shared_ptr<oracle::partition_manager_junior<network>>>().current();

        mockturtle::depth_view orig_depth(partitions_jr.get_network());
        auto start = std::chrono::high_resolution_clock::now();
        mockturtle::names_view<mockturtle::xmg_network> ntk_result;
        mockturtle::names_view<mockturtle::xmg_network> ntk2_result;

	if (is_set("resynth")) {
	    ntk_result = oracle::optimize_resynthesis<mockturtle::aig_network>(partitions_jr, abc_exec);
	} 
       
    if (is_set("convergence")){
        //oracle::optimization_strategy strategy;
        //strategy = oracle::optimization_strategy::balanced;
        oracle::optimization_strategy strategy;
        if (strategyconvergence=="depth") {
		strategy = oracle::optimization_strategy::depth;
	    } else if (strategyconvergence=="nodes") {
		strategy = oracle::optimization_strategy::size;
	    } else {
		strategy = oracle::optimization_strategy::balanced;
	    }
        bool reoptimize_bool = true;
        ntk_result = oracle::optimize_basic<network>(partitions_jr, abc_exec, strategy, true);
    }

    else {
	    oracle::optimization_strategy strategy;
	    if (is_set("depth")) {
		strategy = oracle::optimization_strategy::depth;
	    } else if (is_set("nodes")) {
		strategy = oracle::optimization_strategy::size;
	    } else {
		strategy = oracle::optimization_strategy::balanced;
	    }
    
        ntk_result = oracle::optimize_basic<network>(partitions_jr, abc_exec, strategy,false);
    }
        auto stop = std::chrono::high_resolution_clock::now();
        mockturtle::depth_view new_depth(ntk_result);


        if (ntk_result.size() == partitions_jr.get_network().size()
                && orig_depth.depth() == new_depth.depth()) {
            env->err() << "No change made to network" << std::endl;
        }

        env->out() << "Final ntk size = " << ntk_result.num_gates() << " and depth = "
                    << new_depth.depth() << "\n";
        env->out() << "Final number of latches = " << ntk_result.num_latches() << "\n";
        env->out() << "Node Depth Product = "
                    << ntk_result.num_gates() * new_depth.depth()
                    << "\n";
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                        (stop - start);
        env->out() << "Full Optimization: " << duration.count() << "ms\n";
        env->out() << "Finished optimization\n";
        store<std::shared_ptr<mockturtle::names_view<mockturtle::xmg_network>>>().extend() =
                    std::make_shared<mockturtle::names_view<mockturtle::xmg_network>>(ntk_result);
    }

    string abc_exec{"abc"};
private:
    std:: string strategyconvergence={""};

};
ALICE_ADD_COMMAND(optimize, "Optimization");
}
