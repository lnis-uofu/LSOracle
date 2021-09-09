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
#include <lorina/bench.hpp>
#include <lorina/blif.hpp>
#include <mockturtle/mockturtle.hpp>

#include <fmt/format.h>

namespace alice
{
using klut_names = mockturtle::names_view<mockturtle::klut_network>;
using klut_ntk = std::shared_ptr<klut_names>;

ALICE_ADD_STORE(klut_ntk, "klut", "k", "klut", "kluts")

ALICE_DESCRIBE_STORE(klut_ntk, klut)
{

    return fmt::format("i/o = {}/{} gates = {}", klut->num_pis(), klut->num_pos(),
                       klut->num_gates());
}

ALICE_LOG_STORE_STATISTICS(klut_ntk, klut)
{
    mockturtle::depth_view depth{*klut};
    return {
        {"nodes", klut->size()},
        {"inputs", klut->num_pis() - klut->num_latches()},
        {"latches", klut->num_latches()},
        {"outputs", klut->num_pos() - klut->num_latches()},
        {"klut nodes", klut->num_gates()},
        {"klut level", depth.depth()}};
}

ALICE_PRINT_STORE_STATISTICS(klut_ntk, os, klut)
{
    mockturtle::depth_view depth{*klut};
    os << "nodes: " << klut->size() << std::endl;
    os << "inputs: " << klut->num_pis() - klut->num_latches() << std::endl;
    os << "latches: " << klut->num_latches() << std::endl;
    os << "outputs: " << klut->num_pos() - klut->num_latches() << std::endl;
    os << "klut nodes: " << klut->num_gates() << std::endl;
    os << "klut level: " << depth.depth() << std::endl;

}

}
