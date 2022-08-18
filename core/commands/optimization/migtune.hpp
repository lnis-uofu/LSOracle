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
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

extern string lsoracle_path;
namespace alice
{
class migtune_command : public alice::command
{

public:
    explicit migtune_command(const environment::ptr &env)
        : command(env, "Perform automatic optimization script")
    {
        opts.add_flag("--aig,-a", "Partition stored AIG");
        opts.add_flag("--mig,-m", "Partition stored MIG (Default)");
        opts.add_flag("--xag,-x", "Partition stored XAG");
        opts.add_flag("--xmg,-g", "Partition stored XMG");
        opts.add_option("--strategy",strategy, "Strategy for optimization [depth,size]");
    }

protected:
    template <typename network>
    void tune(std::string name, int size_strategy, int depth_strategy, string src_flag)
    {
        if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
            env->err() << name << " network not stored\n";
            return;
        }
        mockturtle::names_view<network> opt =
            *store<std::shared_ptr<mockturtle::names_view<network>>>().current();

        mockturtle::depth_view mig_depth{opt};

        env->out() << "Initial " << name << " depth " << mig_depth.depth() << " and nodes " <<
                   opt.num_gates() << std::endl;

        std::string ckt_name = "testing_mig.v";

        mockturtle::write_verilog_params ps;
        mockturtle::write_verilog(opt, ckt_name, ps);

        int target;
        if (strategy == "depth") {
            target = depth_strategy;
        } else if (strategy == "size") {
            target = size_strategy;
        } else {
            env->err() << "Unknown strategy " << strategy << std::endl;
            return;
        }
        int repeats = 4;
        int prefix_pos = 1;
        int n_samples = 5;
        int mab_iter = 10;
        int f_forget = 0;
        int f_softmax = 0;
        string recipe = oracle::bayes_flow_tune(lsoracle_path, ckt_name, repeats,
                                                prefix_pos, target,
                                                n_samples, mab_iter, f_forget, f_softmax);

        string command = lsoracle_path + " -c \"read " + src_flag + " " + ckt_name + " " +
            recipe + " write_verilog " + src_flag + " " + ckt_name + ".out.v ; \"";
        env->out() << command << std::endl;
        system(command.c_str());

        mockturtle::names_view<network> output;
        lorina::return_code result = lorina::read_verilog(ckt_name + ".out.v",
                                                          mockturtle::verilog_reader(output));
        if (result != lorina::return_code::success) {
            env->err() << "Unable to read verilog file" << std::endl;
            return;
        }
        output.set_network_name(opt.get_network_name());

        store<std::shared_ptr<mockturtle::names_view<network>>>().extend() =
                    std::make_shared<mockturtle::names_view<network>>(output);
    }

    void execute()
    {
        if (is_set("aig")) {
            tune<mockturtle::aig_network>("AIG", 0, 1, "");
        } else if (is_set("xag")) {
            tune<mockturtle::xag_network>("XAG", 4, 5, "-x");
        } else if (is_set("xmg")) {
            tune<mockturtle::xmg_network>("XMG", 6, 7, "-g");
        } else {
            tune<mockturtle::mig_network>("MIG", 2, 3, "-m");
        }
    }
private:
    std:: string strategy={"depth"};
};

ALICE_ADD_COMMAND(migtune, "Optimization");
}
