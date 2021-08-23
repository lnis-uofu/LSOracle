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
class write_partition_command : public alice::command
{
    using aig_names = mockturtle::names_view<mockturtle::aig_network>;
    using aig_ntk = std::shared_ptr<aig_names>;
    using part_man_aig = oracle::partition_manager<aig_names>;
    using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

    using mig_names = mockturtle::names_view<mockturtle::mig_network>;
    using mig_ntk = std::shared_ptr<mig_names>;
    using part_man_mig = oracle::partition_manager<mig_names>;
    using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

public:
    explicit write_partition_command(const environment::ptr &env)
        : command(env, "Writes the partition to file")
    {

        opts.add_option("--filename,filename", filename,
                        "BLIF file to write out to")->required();
        opts.add_option("--num,-n", partnum, "Part to write.");
    }

protected:
    void execute()
    {
        auto ntk_aig = *store<aig_ntk>().current();
        auto partitions_aig = *store<part_man_aig_ntk>().current();

        auto ntk_mig = *oracle::aig_to_mig(ntk_aig, 1);

        part_man_mig partitions(ntk_mig,
                                partitions_aig.get_all_part_connections(),
                                partitions_aig.get_all_partition_inputs(),
                                partitions_aig.get_all_partition_outputs(),
                                partitions_aig.get_all_partition_regs(),
                                partitions_aig.get_all_partition_regin(), partitions_aig.get_part_num());


        mockturtle::write_blif_params ps;
        auto part = partitions.create_part(ntk_mig, partnum);
        auto mig = oracle::part_to_mig(part, 0);

        mockturtle::write_blif(*mig, filename, ps);

    }
private:
    std::string filename{};
    int partnum;
};

ALICE_ADD_COMMAND(write_partition, "Output");
}
