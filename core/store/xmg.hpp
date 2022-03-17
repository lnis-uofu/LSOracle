/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2021 Laboratory for Nano Integrated Systems (LNIS)
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
using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
using xmg_ntk = std::shared_ptr<xmg_names>;
using part_man_jr_xmg = oracle::partition_manager_junior<mockturtle::xmg_network>;
using part_man_jr_xmg_ntk = std::shared_ptr<part_man_jr_xmg>;

ALICE_ADD_STORE(xmg_ntk, "xmg", "g", "xmg", "XMGs")

ALICE_DESCRIBE_STORE(xmg_ntk, xmg)
{

    return fmt::format("i/o = {}/{} gates = {}", xmg->num_pis(), xmg->num_pos(),
                       xmg->num_gates());
}

ALICE_LOG_STORE_STATISTICS(xmg_ntk, xmg)
{
    mockturtle::depth_view depth{*xmg};
    return {
        {"nodes", xmg->size()},
        {"inputs", xmg->num_pis()},
        {"outputs", xmg->num_pos()},
        {"latches", xmg->num_latches()},
        {"XMG nodes", xmg->num_gates()},
        {"XMG level", depth.depth()}};
}

ALICE_PRINT_STORE_STATISTICS(xmg_ntk, os, xmg)
{
    mockturtle::depth_view depth{*xmg};
    os << "nodes: " << xmg->size() << std::endl;
    os << "inputs: " << xmg->num_pis() << std::endl;
    os << "outputs: " << xmg->num_pos() << std::endl;
    os << "latches: " << xmg->num_latches() << std::endl;
    os << "XMG nodes: " << xmg->num_gates() << std::endl;
    os << "XMG level: " << depth.depth() << std::endl;

}


ALICE_ADD_STORE(part_man_jr_xmg_ntk, "part_man_jr_xmg", "pm_g", "part_man_jr_xmg",
                "PART_MAN_JR_XMGs")

/* Implements the short string to describe a store element in store -a */
ALICE_DESCRIBE_STORE(part_man_jr_xmg_ntk, part_man_jr)
{

    const auto name = "partition manager for Named XMG networks";
    const auto part_num = part_man_jr->count();

    return fmt::format("{} # partitions = {}", name, part_num);
}//end partition manager<xmg_network> describe store

ALICE_LOG_STORE_STATISTICS(part_man_jr_xmg_ntk, part_man_jr)
{

    return {
        {"partition number", part_man_jr->count()}};
}//end partition manager<xmg_network> log store statistics

/* Implements the functionality of ps -b */
ALICE_PRINT_STORE_STATISTICS(part_man_jr_xmg_ntk, os, part_man_jr)
{
    os << "partition number: " << part_man_jr->count() << std::endl;
}//end partition manager<xmg_network> print store statistics

}
