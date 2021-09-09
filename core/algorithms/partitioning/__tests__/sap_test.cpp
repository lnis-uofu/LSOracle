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
#include <iostream>
#include <gtest/gtest.h>
#include <queue>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/io/write_dot.hpp>
#include "algorithms/partitioning/structure_partition.hpp"

using network = mockturtle::aig_network;

network::signal add_symmetric(network *net, size_t depth)
{
    if (depth == 0) {
        return net->create_pi();
    } else {
        return net->create_nand(add_symmetric(net, depth - 1), add_symmetric(net,
                                depth - 1));
    }
}

network::signal add_chain(network *net, size_t depth)
{
    network::signal a = net->create_pi();
    network::signal b = net->create_nand(a, net->create_pi());
    for (; depth > 0; depth--) {
        network::signal c = net->create_nand(a, b);
        a = b;
        b = c;
    }
    return b;
}

/*
 * Calculating a fully extracted network.
 */
TEST(sap_test, sap_cones_case)
{
    network *net = new network;
    network::signal pi0 = net->create_pi();
    network::signal pi1 = net->create_pi();
    network::signal pi2 = net->create_pi();
    network::signal pi3 = net->create_pi();
    network::signal w0 = net->create_nand(pi0, pi1);
    network::signal w1 = net->create_nand(pi1, pi2);
    network::signal w2 = net->create_nand(w0, w1);
    network::signal w3 = net->create_nand(pi3, w1);
    uint32_t po0 = net->create_po(w2);
    uint32_t po1 = net->create_po(w3);

    std::vector<kahypar_partition_id_t> partition(net->size(), -1);
    oracle::structure_partition<network> sap(*net);
    sap.sap_fixed(partition, 5, 1);
    ASSERT_EQ(partition[pi0.index], 0);
    ASSERT_EQ(partition[pi1.index], 0);
    ASSERT_EQ(partition[pi2.index], 0);
    ASSERT_EQ(partition[pi3.index], 1);
    ASSERT_EQ(partition[w0.index], 0);
    ASSERT_EQ(partition[w1.index], 0);
    ASSERT_EQ(partition[w2.index], 0);
    ASSERT_EQ(partition[w3.index], 1);
}

/*
 * Calculating halting on the second inflection.
 */
TEST(sap_test, sap_inflection_case)
{
    network *net = new network;
    network::signal w0 = add_symmetric(net, 5);
    network::signal w1 = add_symmetric(net, 4);
    network::signal w2 = add_symmetric(net, 4);
    network::signal w3 = add_symmetric(net, 3);
    network::signal w4 = add_symmetric(net, 3);
    network::signal w5 = add_symmetric(net, 2);
    network::signal w6 = add_symmetric(net, 2);
    network::signal w7 = add_symmetric(net, 1);
    uint32_t po0 = net->create_po(w0);
    uint32_t po1 = net->create_po(w1);
    uint32_t po2 = net->create_po(w2);
    uint32_t po3 = net->create_po(w3);
    uint32_t po4 = net->create_po(w4);
    uint32_t po5 = net->create_po(w5);
    uint32_t po6 = net->create_po(w6);
    uint32_t po7 = net->create_po(w7);

    std::vector<kahypar_partition_id_t> partition(net->size(), -1);
    oracle::structure_partition<network> sap(*net);

    sap.sap_fixed(partition, 1,
                  2); // Minimum size forces cone per bin during bin packing.
    ASSERT_EQ(partition[w0.index], 0);
    ASSERT_EQ(partition[w1.index], 1);
    ASSERT_EQ(partition[w2.index], 2);
    ASSERT_EQ(partition[w3.index], 3);
    ASSERT_EQ(partition[w4.index], 4);
    ASSERT_EQ(partition[w5.index], -1);
}

/*
 * Testing bin packing
 */
TEST(sap_test, sap_packing_case)
{
    network *net = new network;
    network::signal w0 = add_chain(net, 3);
    network::signal w1 = add_chain(net, 3);
    network::signal w2 = add_chain(net, 3);
    network::signal w3 = add_chain(net, 3);
    network::signal w4 = add_chain(net, 3);
    uint32_t po0 = net->create_po(w0);
    uint32_t po1 = net->create_po(w1);
    uint32_t po2 = net->create_po(w2);
    uint32_t po3 = net->create_po(w3);
    uint32_t po4 = net->create_po(w4);

    std::vector<kahypar_partition_id_t> partition(net->size(), -1);
    oracle::structure_partition<network> sap(*net);
    sap.sap_fixed(partition, 12,
                  2); // Minimum size forces cone per bin during bin packing.
    ASSERT_EQ(partition[w0.index], 0);
    ASSERT_EQ(partition[w1.index], 0);
    ASSERT_EQ(partition[w2.index], 1);
    ASSERT_EQ(partition[w3.index], 1);
    ASSERT_EQ(partition[w4.index], 2);
}
