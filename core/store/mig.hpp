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

#include <memory>

#include <alice/alice.hpp>
#include <lorina/aiger.hpp>
#include <lorina/blif.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/mockturtle.hpp>

#include <fmt/format.h>

namespace alice
{
using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using mig_ntk = std::shared_ptr<mig_names>;
using part_man_mig = oracle::partition_manager<mig_names>;
using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

ALICE_ADD_STORE(mig_ntk, "mig", "m", "mig", "MIGs")

ALICE_DESCRIBE_STORE(mig_ntk, mig)
{

    return fmt::format("i/o = {}/{} gates = {}", mig->num_pis(), mig->num_pos(),
                       mig->num_gates());
}

ALICE_LOG_STORE_STATISTICS(mig_ntk, mig)
{
    mockturtle::depth_view depth{*mig};
    return {
        {"nodes", mig->size()},
        {"inputs", mig->num_pis() - mig->num_latches()},
        {"latches", mig->num_latches()},
        {"outputs", mig->num_pos() - mig->num_latches()},
        {"MAJ nodes", mig->num_gates()},
        {"MIG level", depth.depth()}};
}

ALICE_PRINT_STORE_STATISTICS(mig_ntk, os, mig)
{
    mockturtle::depth_view depth{*mig};
    os << "nodes: " << mig->size() << std::endl;
    os << "inputs: " << mig->num_pis() - mig->num_latches() << std::endl;
    os << "latches: " << mig->num_latches() << std::endl;
    os << "outputs: " << mig->num_pos() - mig->num_latches() << std::endl;
    os << "MAJ nodes: " << mig->num_gates() << std::endl;
    os << "MIG level: " << depth.depth() << std::endl;

}

ALICE_ADD_STORE(part_man_mig_ntk, "part_man_mig", "pm_m", "part_man_mig",
                "PART_MAN_MIGs")

/* Implements the short string to describe a store element in store -a */
ALICE_DESCRIBE_STORE(part_man_mig_ntk, part_man)
{

    const auto name = "partition manager for Named MIG networks";
    const auto part_num = part_man->get_part_num();

    return fmt::format("{} # partitions = {}", name, part_num);
}//end partition manager<mig_network> describe store

ALICE_LOG_STORE_STATISTICS(part_man_mig_ntk, part_man)
{

    return {
        {"partition number", part_man->get_part_num()}};
}//end partition manager<mig_network> log store statistics

/* Implements the functionality of ps -b */
ALICE_PRINT_STORE_STATISTICS(part_man_mig_ntk, os, part_man)
{
    os << "partition number: " << part_man->get_part_num() << std::endl;
}//end partition manager<mig_network> print store statistics
}
