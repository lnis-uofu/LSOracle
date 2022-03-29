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
#include <filesystem>


namespace alice
{
class get_all_partitions_command : public alice::command
{

public:
    explicit get_all_partitions_command(const environment::ptr &env)
        : command(env, "Exports every partition to Verilog files")
    {
        opts.add_option("--directory,directory", dir,
                        "Directory to write Verilog files to")->required();
        // add_flag("--cone,-c", "Writes out every cone of every partition");
        // add_flag("--verilog,-v", "Writes every partition or cone to a verilog file");
        add_flag("--mig,-m",
                 "Write out all of the partitions of the sored MIG network");
    }

protected:
    void execute()
    {
        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        if (is_set("mig")) {
            if (!store<mig_ntk>().empty()) {
                auto ntk = *store<mig_ntk>().current();
                env->out() << "\n";
                if (!store<part_man_mig_ntk>().empty()) {
                    auto partitions = *store<part_man_mig_ntk>().current();
                    for (int i = 0; i < partitions.get_part_num(); i++) {
                        std::vector<mockturtle::mig_network> parts;
                        std::vector<std::string> filenames;
                        int partition = i;
                        auto part_outputs = partitions.get_part_outputs(i);
                        env->out() << "Partition " << i << ":\n";
                        env->out() << "Number of Logic Cones = " << part_outputs.size() << "\n";
                        mkdir(dir.c_str(), 0777);

                        oracle::partition_view<mig_names> part = partitions.create_part(ntk, partition);
                        auto part_ntk = mockturtle::node_resynthesis<mockturtle::mig_network>(part,
                                        resyn_mig);

                        std::string filename = dir + "/" + ntk._storage->_network_name + "_" +
                                               std::to_string(partition) + ".v";;
                        filenames.push_back(filename);
                        parts.push_back(part_ntk);

                        assert(parts.size() == filenames.size());
                        for (int j = 0; j < parts.size(); j++) {
                            mockturtle::write_verilog(parts.at(j), filenames.at(j));
                        }
                        env->out() << "\n";
                    }
                } else {
                    env->err() << "Partitions have not been mapped\n";
                }
            } else {
                env->err() << "There is no MIG network stored\n";
            }
        } else {
            if (!store<aig_ntk>().empty()) {
                auto ntk = *store<aig_ntk>().current();
                env->out() << "\n";
                if (!store<part_man_aig_ntk>().empty()) {
                    mockturtle::write_verilog_params ps;
                    auto partitions = *store<part_man_aig_ntk>().current();
                    mockturtle::node_map<std::string, aig_names> node_names(ntk);
                    mockturtle::node_map<std::string, aig_names> input_names(ntk);
                    std::string toplevel_module = std::filesystem::path(
                                                      ntk._storage->_network_name).filename();
                    std::string toplevel_file = dir + "/" + toplevel_module + ".v";
                    for (int i = 0; i < partitions.get_part_num(); i++) {
                        std::vector<mockturtle::aig_network> parts;
                        std::vector<std::string> filenames;
                        int partition = i;
                        auto part_outputs = partitions.get_part_outputs(i);
                        env->out() << "Partition " << i << ":\n";
                        env->out() << "Number of Logic Cones = " << part_outputs.size() << "\n";
                        mkdir(dir.c_str(), 0777);

                        oracle::partition_view<aig_names> part = partitions.create_part(ntk, partition);
                        auto part_ntk = mockturtle::node_resynthesis<aig_names>(part, resyn_aig);

                        std::string modulename = std::filesystem::path(ntk._storage->_network_name + "_"
                                                 + std::to_string(partition)).filename();
                        std::string filename = dir + "/" + modulename + ".v";
                        filenames.push_back(filename);
                        parts.push_back(part_ntk);

                        if (part_ntk.num_pos() == 0)
                            continue;

                        assert(parts.size() == filenames.size());
                        for (int j = 0; j < parts.size(); j++) {
                            ps.module_name = modulename;
                            mockturtle::write_verilog(parts.at(j), filenames.at(j), ps);
                        }

                        if (i == 0)
                            //to do: write_toplevel_verilog has been removed along with the rest of the duplicate write_verilog method because it was not
                            //  compatible with the newest version of mockturtle.  We'll want it back, but for now I'm disabling this feature
                            oracle::write_toplevel_verilog(ntk, partitions, toplevel_file, node_names,
                                                           input_names, toplevel_module);

                        //to do: see above, but for call_submodule
                        oracle::call_submodule(ntk, part_ntk, toplevel_file, modulename, i, part,
                                               node_names, input_names);

                        env->out() << "\n";
                    }
                    std::ofstream os(toplevel_file.c_str(), std::ofstream::app);
                    os << "endmodule\n"
                       << std::flush;

                    os.close();
                } else {
                    env->err() << "Partitions have not been mapped\n";
                }
            } else {
                env->err() << "There is no AIG network stored\n";
            }
        }
    }
private:
    std::string dir{};
};

ALICE_ADD_COMMAND(get_all_partitions, "Output");
}
