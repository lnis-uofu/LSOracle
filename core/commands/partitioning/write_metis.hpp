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
#include "algorithms/partitioning/partition_manager_junior.hpp"
namespace alice
{
    class write_metis_command : public alice::command
    {

    public:
        explicit write_metis_command(const environment::ptr &env)
            : command(env,
                      "Store partition map of in hMetis format.")
            {
                add_flag("--aig,-a", "Stored partition AIG (Default)");
                add_flag("--mig,-m", "Stored partition MIG");
                add_flag("--xag,-x", "Stored partition XAG");
                add_flag("--xmg,-g", "Stored partition XMG");
                opts.add_option("--file,-f,file", output_file,
                                "File to write to");
            }

    protected:

        template <typename network>
        void write(string name)
        {
            if (store<std::shared_ptr<oracle::partition_manager_junior<network>>>().empty()) {
                spdlog::error("{} partitioned network not stored", name);
                return;
            }

            oracle::partition_manager_junior<network> partitions =
                *store<std::shared_ptr<oracle::partition_manager_junior<network>>>().current();

            std::ofstream out;
            out.open(output_file);
            bool first = true;
            partitions.get_network().foreach_node([&](auto n, auto i){
                if (first) {
                    out << partitions.node_partition(n);
                    first = false;
                } else {
                    out << std::endl << partitions.node_partition(n);
                }
            });
            out.close();
        }
        void execute()
        {
            if (is_set("mig")) {
                write<mockturtle::mig_network>("MIG");
            } else if (is_set("xag")) {
                write<mockturtle::xag_network>("XAG");
            } else if (is_set("xmg")) {
                write<mockturtle::xmg_network>("XMG");
            } else {
                write<mockturtle::aig_network>("AIG");
            }
        }
    private:
        std::string output_file = "";
    };

    ALICE_ADD_COMMAND(write_metis, "Partitioning");
}
