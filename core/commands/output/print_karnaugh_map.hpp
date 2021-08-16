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


namespace alice
{
class print_karnaugh_command : public alice::command
{

public:
    using aig_names = mockturtle::names_view<mockturtle::aig_network>;
    using aig_ntk = std::shared_ptr<aig_names>;
    using part_man_aig = oracle::partition_manager<aig_names>;
    using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

    using mig_names = mockturtle::names_view<mockturtle::mig_network>;
    using mig_ntk = std::shared_ptr<mig_names>;
    using part_man_mig = oracle::partition_manager<mig_names>;
    using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

    explicit print_karnaugh_command(const environment::ptr &env)
        : command(env, "Prints all the partitioned truth tables as Karnaugh maps")
    {

        opts.add_option("--directory,-d", directory,
                        "Directory to write files to")->required();
        opts.add_option("--filename,-f", filename, "Classification File to read from");
        add_flag("--tensor,-t",
                 "Write the k-maps to tensor dataset depending on <filename>");
        add_flag("--mig,-m",
                 "Read from the MIG network and MIG partition manager for k-maps");

    }

protected:
    void execute()
    {

        if (is_set("mig")) {
            env->err() << "MIG networks not supported yet\n";
        } else {
            if (!store<aig_ntk>().empty()) {
                auto aig = *store<aig_ntk>().current();
                if (!store<part_man_aig_ntk>().empty()) {
                    env->out() << "Writing k-map images for stored AIG network\n";
                    auto partitions = *store<part_man_aig_ntk>().current();
                    partitions.write_karnaugh_maps(aig, directory);
                } else {
                    env->err() << "AIG not partitioned yet\n";
                }
            } else {
                env->err() << "AIG network not stored\n";
            }
        }
    }

private:
    std::string filename{};
    std::string directory{};
};

ALICE_ADD_COMMAND(print_karnaugh, "Output");
}
