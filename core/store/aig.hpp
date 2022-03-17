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
#include "algorithms/partitioning/partition_manager_junior.hpp"
namespace alice
{
using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using aig_ntk = std::shared_ptr<aig_names>;
using part_man_aig = oracle::partition_manager<aig_names>;
using part_man_aig_ntk = std::shared_ptr<part_man_aig>;
using part_man_jr_aig = oracle::partition_manager_junior<mockturtle::aig_network>;
using part_man_jr_aig_ntk = std::shared_ptr<part_man_jr_aig>;

ALICE_ADD_STORE(aig_ntk, "aig", "a", "aig", "AIGs")

ALICE_DESCRIBE_STORE(aig_ntk, aig)
{

    return fmt::format("i/o = {}/{} gates = {}", aig->num_pis(), aig->num_pos(),
                       aig->num_gates());
}

ALICE_LOG_STORE_STATISTICS(aig_ntk, aig)
{
    mockturtle::depth_view depth{*aig};
    return {
        {"nodes", aig->size()},
        {"inputs", aig->num_pis()},
        {"latches", aig->num_latches()},
        {"outputs", aig->num_pos()},
        {"AIG nodes", aig->num_gates()},
        {"AIG level", depth.depth()}};
}

ALICE_PRINT_STORE_STATISTICS(aig_ntk, os, aig)
{
    mockturtle::depth_view depth{*aig};
    os << "nodes: " << aig->size() << std::endl;
    os << "inputs: " << aig->num_pis() << std::endl;
    os << "latches: " << aig->num_latches() << std::endl;
    os << "outputs: " << aig->num_pos() << std::endl;
    os << "AIG nodes: " << aig->num_gates() << std::endl;
    os << "AIG level: " << depth.depth() << std::endl;

}

ALICE_ADD_STORE(part_man_aig_ntk, "part_man_aig", "pm_a", "part_man_aig",
                "PART_MAN_AIGs")

/* Implements the short string to describe a store element in store -a */
ALICE_DESCRIBE_STORE(part_man_aig_ntk, part_man)
{

    const auto name = "partition manager for Named MIG networks";
    const auto part_num = part_man->get_part_num();

    return fmt::format("{} # partitions = {}", name, part_num);
}//end partition manager<mig_network> describe store

ALICE_LOG_STORE_STATISTICS(part_man_aig_ntk, part_man)
{

    return {
        {"partition number", part_man->get_part_num()}};
}//end partition manager<mig_network> log store statistics

/* Implements the functionality of ps -b */
ALICE_PRINT_STORE_STATISTICS(part_man_aig_ntk, os, part_man)
{
    os << "partition number: " << part_man->get_part_num() << std::endl;
}//end partition manager<mig_network> print store statistics


ALICE_ADD_STORE(part_man_jr_aig_ntk, "part_man_jr_aig", "pm_a", "part_man_jr_aig",
                "PART_MAN_JR_AIGs")

/* Implements the short string to describe a store element in store -a */
ALICE_DESCRIBE_STORE(part_man_jr_aig_ntk, part_man_jr)
{

    const auto name = "partition manager for Named MIG networks";
    const auto part_num = part_man_jr->count();

    return fmt::format("{} # partitions = {}", name, part_num);
}//end partition manager<mig_network> describe store

ALICE_LOG_STORE_STATISTICS(part_man_jr_aig_ntk, part_man_jr)
{

    return {
        {"partition number", part_man_jr->count()}};
}//end partition manager<mig_network> log store statistics

/* Implements the functionality of ps -b */
ALICE_PRINT_STORE_STATISTICS(part_man_jr_aig_ntk, os, part_man_jr)
{
    os << "partition number: " << part_man_jr->count() << std::endl;
}//end partition manager<mig_network> print store statistics
}
