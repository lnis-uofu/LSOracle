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
#include <vector>
#include <mockturtle/mockturtle.hpp>
#include <mockturtle/io/write_dot.hpp>
#include "algorithms/optimization/exploder.hpp"

using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using aig_window = mockturtle::window_view<aig_names>;
using aig_exploder = oracle::exploderizer<mockturtle::aig_network>;
using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
using xmg_window = mockturtle::window_view<xmg_names>;
using xmg_exploder = oracle::exploderizer<mockturtle::xmg_network>;
using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using mig_window = mockturtle::window_view<mig_names>;
using mig_exploder = oracle::exploderizer<mockturtle::mig_network>;
using xag_names = mockturtle::names_view<mockturtle::xag_network>;
using xag_window = mockturtle::window_view<xag_names>;
using xag_exploder = oracle::exploderizer<mockturtle::xag_network>;

TEST(exploderizer_test, disjoint_cones)
{
    aig_names net;
    aig_names::signal pi0 = net.create_pi();
    net.set_name(pi0, "pi0");
    aig_names::signal pi1 = net.create_pi();
    net.set_name(pi1, "pi1");
    aig_names::signal pi2 = net.create_pi();
    net.set_name(pi2, "pi2");
    aig_names::signal pi3 = net.create_pi();
    net.set_name(pi3, "pi3");
    aig_names::signal pi4 = net.create_pi();
    net.set_name(pi4, "pi4");
    aig_names::signal pi5 = net.create_pi();
    net.set_name(pi5, "pi5");

    aig_names::signal w0 = net.create_nand(pi0, pi1);
    net.set_name(w0, "w0");
    aig_names::signal w1 = net.create_and(pi1, pi2);
    net.set_name(w1, "w1");
    aig_names::signal w2 = net.create_nand(w0, w1);
    net.set_name(w2, "w2");
    uint32_t po0 = net.create_po(w2);
    net.set_output_name(po0, "po0");

    aig_names::signal w3 = net.create_nand(pi3, pi4);
    net.set_name(w3, "w3");
    aig_names::signal w4 = net.create_nand(pi4, pi5);
    net.set_name(w4, "w4");
    aig_names::signal w5 = net.create_nand(w3, w4);
    net.set_name(w5, "w5");
    uint32_t po1 = net.create_po(w5);
    net.set_output_name(po1, "po1");

    aig_exploder boom(net, "yosys-abc");

    ASSERT_EQ(net.num_gates(), 6);
    ASSERT_EQ(net.num_pos(), 2);
    ASSERT_EQ(net.num_pis(), 6);

    xmg_names result = boom.exploderize(10);

    ASSERT_EQ(result.num_gates(), 6);
    ASSERT_EQ(result.num_pos(), 2);
    ASSERT_EQ(result.num_pis(), 6);
    mockturtle::write_dot(net, "disjoint_net.dot");
    mockturtle::write_dot(result, "disjoint_result.dot");

    const auto miter = *mockturtle::miter<xmg_names>(net, result);
    const auto valid = mockturtle::equivalence_checking(miter);
    ASSERT_TRUE(valid && *valid);
}

TEST(exploderizer_test, strashable_cones)
{
    aig_names net;
    aig_names::signal pi0 = net.create_pi();
    net.set_name(pi0, "pi0");
    aig_names::signal pi1 = net.create_pi();
    net.set_name(pi1, "pi1");
    aig_names::signal pi2 = net.create_pi();
    net.set_name(pi2, "pi2");
    aig_names::signal pi3 = net.create_pi();
    net.set_name(pi3, "pi3");

    aig_names::signal w0 = net.create_nand(pi0, pi1);
    net.set_name(w0, "w0");
    aig_names::signal w1 = net.create_nand(pi1, pi2);
    net.set_name(w1, "w1");
    aig_names::signal w2 = net.create_nand(pi2, pi3);
    net.set_name(w2, "w2");
    aig_names::signal w4 = net.create_and(w0, w1);
    net.set_name(w4, "w4");
    aig_names::signal w5 = net.create_nand(w1, w2);
    net.set_name(w5, "w5");
    aig_names::signal w6 = net.create_nand(w4, w5);
    net.set_name(w6, "w6");

    uint32_t po0 = net.create_po(w5);
    net.set_output_name(po0, "po0");
    uint32_t po1 = net.create_po(w6);
    net.set_output_name(po1, "po1");

    aig_exploder boom(net, "yosys-abc");

    ASSERT_EQ(net.num_gates(), 6);
    ASSERT_EQ(net.num_pos(), 2);
    ASSERT_EQ(net.num_pis(), 4);

    xmg_names result = boom.exploderize(10);

    ASSERT_EQ(result.num_pos(), 2);
    ASSERT_EQ(result.num_pis(), 4);
    mockturtle::write_dot(net, "strash_net.dot");
    mockturtle::write_dot(result, "strash_result.dot");

    const auto miter = *mockturtle::miter<xmg_names>(net, result);
    const auto valid = mockturtle::equivalence_checking(miter);
    ASSERT_TRUE(valid && *valid);
}

TEST(exploderizer_test, big_network)
{
    aig_names net;
    aig_names::signal pi0 = net.create_pi();
    net.set_name(pi0, "pi0");
    aig_names::signal pi1 = net.create_pi();
    net.set_name(pi1, "pi1");
    aig_names::signal pi2 = net.create_pi();
    net.set_name(pi2, "pi2");
    aig_names::signal pi3 = net.create_pi();
    net.set_name(pi3, "pi3");
    aig_names::signal pi4 = net.create_pi();
    net.set_name(pi4, "pi4");
    aig_names::signal pi5 = net.create_pi();
    net.set_name(pi5, "pi5");
    aig_names::signal pi6 = net.create_pi();
    net.set_name(pi6, "pi6");
    aig_names::signal pi7 = net.create_pi();
    net.set_name(pi7, "pi7");
    aig_names::signal pi8 = net.create_pi();
    net.set_name(pi8, "pi8");
    aig_names::signal pi9 = net.create_pi();
    net.set_name(pi9, "pi9");
    aig_names::signal pi10 = net.create_pi();
    net.set_name(pi10, "pi10");
    aig_names::signal pi11 = net.create_pi();
    net.set_name(pi11, "pi11");
    aig_names::signal pi12 = net.create_pi();
    net.set_name(pi12, "pi12");
    aig_names::signal pi13 = net.create_pi(); // isolated subnet
    net.set_name(pi13, "pi13");
    aig_names::signal pi14 = net.create_pi(); // isolated subnet
    net.set_name(pi14, "pi14");
    aig_names::signal w0 = net.create_nand(pi0, pi1);
    net.set_name(w0, "w0");
    aig_names::signal w1 = net.create_nand(pi4, pi5);
    net.set_name(w1, "w1");
    aig_names::signal w2 = net.create_nand(pi2, w1);
    net.set_name(w2, "w2");
    aig_names::signal w3 = net.create_nand(w0, pi3);
    net.set_name(w3, "w3");
    aig_names::signal w4 = net.create_nand(w0, w3);
    net.set_name(w4, "w4");
    aig_names::signal w5 = net.create_nand(pi6, pi7);
    net.set_name(w5, "w5");
    aig_names::signal w6 = net.create_nand(pi6, w5);
    net.set_name(w6, "w6");
    aig_names::signal w7 = net.create_nand(w2, w6);
    net.set_name(w7, "w7");
    aig_names::signal w8 = net.create_nand(pi8, pi9);
    net.set_name(w8, "w8");
    aig_names::signal w10 = net.create_nand(pi8, pi11);
    net.set_name(w10, "w10");
    aig_names::signal w9 = net.create_nand(w10, pi11);
    net.set_name(w9, "w9");
    aig_names::signal w11 = net.create_nand(w4, w7);
    net.set_name(w11, "w11");
    aig_names::signal w12 = net.create_nand(w7, w6);
    net.set_name(w12, "w12");
    aig_names::signal w13 = net.create_nand(w8, w10);
    net.set_name(w13, "w13");
    aig_names::signal w14 = net.create_nand(net.create_not(w6), w13);
    net.set_name(w14, "w14");
    aig_names::signal w15 = net.create_nand(w0, w4);
    net.set_name(w15, "w15");
    aig_names::signal w16 = net.create_nand(w14, w15);
    net.set_name(w16, "w16");
    aig_names::signal w17 = net.create_nand(w9, w14);
    net.set_name(w17, "w17");
    aig_names::signal w18 = net.create_nand(pi14, pi13);
    net.set_name(w18, "w18");

    uint32_t po0 = net.create_po(pi10); // PI with PO passthrough only
    uint32_t po1 = net.create_po(pi11); // PI with PO passthrough and fanout
    uint32_t po2 = net.create_po(pi12); // PI with two PO passthrough only
    uint32_t po3 = net.create_po(pi12);  // PI with two PO passthrough only
    uint32_t po4 = net.create_po(w11); // edge PO from edge gate
    uint32_t po5 = net.create_po(net.create_not(w12)); // inverted duplicate PO from edge gate
    uint32_t po6 = net.create_po(w12); // duplicate PO from edge gate
    uint32_t po7 = net.create_po(w13); // PO for gate with external fanout.
    uint32_t po8 = net.create_po(w15); // PO for gate with internal fanout.
    uint32_t po9 = net.create_po(w14); // PO for gate with internal and external fanout.
    uint32_t po10 = net.create_po(w17); // edge PO from edge gate
    uint32_t po11 = net.create_po(w18); // edge PO for isolated subnetwork.

    ASSERT_EQ(net.num_cis(), 15);
    ASSERT_EQ(net.num_cos(), 12);
    ASSERT_EQ(net.num_gates(), 19);

    aig_exploder boom(net, "yosys-abc");
    xmg_names result = boom.exploderize(10);

    ASSERT_EQ(result.num_cos(), 12);
    ASSERT_EQ(result.num_cis(), 15);
    mockturtle::write_dot(net, "big_net.dot");
    mockturtle::write_dot(result, "big_result.dot");

    const auto miter = *mockturtle::miter<xmg_names>(net, result);
    const auto valid = mockturtle::equivalence_checking(miter);
    ASSERT_TRUE(valid && *valid);

}
