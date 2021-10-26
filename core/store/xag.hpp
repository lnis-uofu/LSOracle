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
using xag_names = mockturtle::names_view<mockturtle::xag_network>;
using xag_ntk = std::shared_ptr<xag_names>;

ALICE_ADD_STORE(xag_ntk, "xag", "x", "xag", "XAGs")

ALICE_DESCRIBE_STORE(xag_ntk, xag)
{

    return fmt::format("i/o = {}/{} gates = {}", xag->num_pis(), xag->num_pos(),
                       xag->num_gates());
}

ALICE_LOG_STORE_STATISTICS(xag_ntk, xag)
{
    mockturtle::depth_view depth{*xag};
    return {
        {"nodes", xag->size()},
        {"inputs", xag->num_pis()},
        {"outputs", xag->num_pos()},
        {"latches", xag->num_latches()},
        {"XAG nodes", xag->num_gates()},
        {"XAG level", depth.depth()}};
}

ALICE_PRINT_STORE_STATISTICS(xag_ntk, os, xag)
{
    mockturtle::depth_view depth{*xag};
    os << "nodes: " << xag->size() << std::endl;
    os << "inputs: " << xag->num_pis() << std::endl;
    os << "outputs: " << xag->num_pos() << std::endl;
    os << "latches: " << xag->num_latches() << std::endl;
    os << "XAG nodes: " << xag->num_gates() << std::endl;
    os << "XAG level: " << depth.depth() << std::endl;

}
}
