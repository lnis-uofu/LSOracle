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
#include "algorithms/partitioning/partition_manager.hpp"

#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <stdlib.h>

#include "utility.hpp"

namespace alice
{
    class external_partition_command : public alice::command
    {

    public:
        explicit external_partition_command(const environment::ptr &env)
            : command(env, "Partitions current network using an external file in HMetis format")
        {
            add_flag("--aig,-a", "Partition stored AIG (Default)");
            add_flag("--mig,-m", "Partition stored MIG");
            add_flag("--xag,-x", "Partition stored XAG");
            add_flag("--xmg,-g", "Partition stored XMG");

            opts.add_option("--file,-f,file", part_file,
                            "External file containing partition information");
        }

    protected:

        template <typename network>
        void process_network(string name)
        {
            if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
                env->err() << name << " network not stored\n";
                return;
            }
            mockturtle::names_view<network> ntk =
                *store<std::shared_ptr<mockturtle::names_view<network>>>().current();


            env->out() << "Partitioning stored " << name << " network using external file" << std::endl;
            std::vector<int> parts = read_integer_file(part_file);
            if (parts.size() != ntk.size()) {
                env->out() << "Partition file contains the incorrect number of nodes" << std::endl;
                exit(1);
            }

            int num_partitions = *std::max_element(parts.begin(), parts.end()) + 1;
            env->out() << "Found " << num_partitions << " partitions" << std::endl;

            mockturtle::node_map<int, mockturtle::names_view<network>> part_data(ntk);
            for (int i = 0; i < parts.size(); i++) {
                part_data[ntk.index_to_node(i)] = parts[i];
            }

            oracle::partition_manager_junior<network> partitions(ntk, part_data, num_partitions);
            store<std::shared_ptr<oracle::partition_manager_junior<network>>>().extend() =
                std::make_shared<oracle::partition_manager_junior<network>>(partitions);
        }
        void execute()
        {
            if (is_set("mig")) {
                process_network<mockturtle::mig_network>("MIG");
            } else if (is_set("xag")) {
                process_network<mockturtle::xag_network>("XAG");
            } else if (is_set("xmg")) {
                process_network<mockturtle::xmg_network>("XMG");
            } else {
                process_network<mockturtle::aig_network>("AIG");
            }
        }
    private:
        std::string part_file = "";
    };

    ALICE_ADD_COMMAND(external_partition, "Partitioning");
}
