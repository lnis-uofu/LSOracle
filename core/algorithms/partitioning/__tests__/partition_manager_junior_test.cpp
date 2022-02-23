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
#include <mockturtle/mockturtle.hpp>

#include <mockturtle/io/write_dot.hpp>
#include "algorithms/partitioning/partition_manager_junior.hpp"
#include "algorithms/optimization/aig_script.hpp"
#include "algorithms/optimization/aig_script2.hpp"
#include "algorithms/optimization/mig_script.hpp"
#include "algorithms/optimization/xag_script.hpp"
#include "algorithms/optimization/xmg_script.hpp"
using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using aig_window = mockturtle::window_view<aig_names>;
using aig_manager = oracle::partition_manager_junior<mockturtle::aig_network>;
using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
using xmg_window = mockturtle::window_view<xmg_names>;
using xmg_manager = oracle::partition_manager_junior<mockturtle::xmg_network>;
using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using mig_window = mockturtle::window_view<mig_names>;
using mig_manager = oracle::partition_manager_junior<mockturtle::mig_network>;
using xag_names = mockturtle::names_view<mockturtle::xag_network>;
using xag_window = mockturtle::window_view<xag_names>;
using xag_manager = oracle::partition_manager_junior<mockturtle::xag_network>;

TEST(partition_manager_junior_test, pmj_partition_generation)
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

    mockturtle::node_map<int, aig_names> parts(net);
    parts[net.get_node(pi0)] = 0; // internal PI
    parts[net.get_node(pi1)] = 0; // internal PI
    parts[net.get_node(pi2)] = 0; // internal PI
    parts[net.get_node(pi3)] = 0; // internal PI accessed twice.
    parts[net.get_node(pi4)] = 1; // internal PI
    parts[net.get_node(pi5)] = 0; // external PI
    parts[net.get_node(pi6)] = 0; // external PI accessed twice
    parts[net.get_node(pi7)] = 1; // internal PI
    parts[net.get_node(pi8)] = 0; // shared PI accessed twice.
    parts[net.get_node(pi9)] = 0; // internal PI
    parts[net.get_node(pi10)] = 0; // PI passthrough
    parts[net.get_node(pi11)] = 0; // shared PI accessed twice.
    parts[net.get_node(pi12)] = 0; // PI passthrough
    parts[net.get_node(pi13)] = 0; // isolated PI
    parts[net.get_node(pi14)] = 0; // isolated PI
    parts[net.get_node(w0)] = 0; // internal gate, fanin both from internal PI
    parts[net.get_node(w1)] = 1; // internal gate, fanin from internal PI and external PI
    parts[net.get_node(w2)] = 0; // internal gate, fanin from internal PI and external gate
    parts[net.get_node(w3)] = 0; // internal gate, fanin from internal PI and internal gate
    parts[net.get_node(w4)] = 0; // internal gate, fanin both from internal gate
    parts[net.get_node(w5)] = 1;
    parts[net.get_node(w6)] = 1; // internal gate, fanin from internal gate and external PI
    parts[net.get_node(w7)] = 0; // internal gate, fanin from internal gate and external gate
    parts[net.get_node(w8)] = 1; // internal gate, fanin both from external PI
    parts[net.get_node(w9)] = 1; // internal gate, fanin from external PI and external gate
    parts[net.get_node(w10)] = 0; // internal gate, fanin both from internal PI
    parts[net.get_node(w11)] = 1; // internal gate, fanin both from external gate
    parts[net.get_node(w12)] = 1;
    parts[net.get_node(w13)] = 1;
    parts[net.get_node(w14)] = 0;
    parts[net.get_node(w15)] = 0;
    parts[net.get_node(w16)] = 0;
    parts[net.get_node(w17)] = 1;
    parts[net.get_node(w18)] = 0;

    aig_manager pm(net, parts, 2);
    ASSERT_EQ(pm.count(), 2);
//    mockturtle::write_dot(net, "pmj_full.dot");

    aig_window part0 = pm.partition(0);
//    mockturtle::write_dot(part0, "pmj_part0.dot");
    aig_window part1 = pm.partition(1);
//    mockturtle::write_dot(part1, "pmj_part1.dot");

    ASSERT_EQ(net.num_cis(), 15);
    ASSERT_EQ(net.num_cos(), 12);
    ASSERT_EQ(net.num_gates(), 19);

    std::vector<aig_names::signal> expect_ci_0 = { pi0, pi1, pi2, pi3, pi5, pi6, pi8, pi9,
                                               pi10, pi11, pi12, pi13, pi14, w1, w6, w13 };
    ASSERT_EQ(part0.num_cis(), 16);
    for (auto i = expect_ci_0.begin(); i != expect_ci_0.end(); i++) {
        ASSERT_TRUE(part0.is_ci(part0.get_node(*i)));
    }

    std::vector<aig_names::signal> expect_co_0 = {pi5, pi6, pi8, pi9, pi10, pi11, pi12,
                                              w4, w7, w10, w14, w15, w18 };
    ASSERT_EQ(part0.num_cos(), 13);
    part0.foreach_co([&](auto s, auto i) {
        ASSERT_EQ(part0.get_node(expect_co_0[i]), part0.get_node(s));
    });

    std::vector<aig_names::signal> expect_gate_0 = {w0, w15, w16, w4, w3, w2, w7,w10, w14, w18 };
    ASSERT_EQ(part0.num_gates(), 10);
    for (auto i = expect_gate_0.begin(); i != expect_gate_0.end(); i++) {
        ASSERT_TRUE(part0.belongs_to(part0.get_node(*i)));
    }

    std::vector<aig_names::signal> expect_ci_1 = { w4, pi4, pi5, pi6, pi7, pi8, w10, pi9, pi11, w7, w14 };
    ASSERT_EQ(part1.num_cis(), 11);
    for (auto i = expect_ci_1.begin(); i != expect_ci_1.end(); i++) {
        ASSERT_TRUE(part1.is_ci(part1.get_node(*i)));
    }

    std::vector<aig_names::signal> expect_co_1 = { w1, w6, w11, w12, w13, w17 };
    ASSERT_EQ(part1.num_cos(), 6);
    part1.foreach_co([&](auto s, auto i) {
        ASSERT_EQ(part1.get_node(expect_co_1[i]), part1.get_node(s));
    });

    std::vector<aig_names::signal> expect_gate_1 = { w1, w6, w5, w11, w12, w13, w9, w17, w8 };
    ASSERT_EQ(part1.num_gates(), 9);
    for (auto i = expect_gate_1.begin(); i != expect_gate_1.end(); i++) {
        ASSERT_TRUE(part1.belongs_to(part1.get_node(*i)));
    }

    ////////////////////////////////
    mockturtle::direct_resynthesis<aig_names> resyn0;
    auto part0_copy = mockturtle::node_resynthesis<aig_names, aig_window>(part0, resyn0);
//    mockturtle::write_dot(part0_copy, "pmj_part0_direct.dot");

    ASSERT_EQ(part0_copy.num_cis(), 16);
    ASSERT_EQ(part0_copy.num_cos(), 13);
    ASSERT_EQ(part0_copy.num_gates(), 9); // the resynthesis drops the dangling edge with no PO.

    oracle::aig_script opt0;
    auto part0_opt = opt0.run(part0_copy);
//    mockturtle::write_dot(part0_opt, "pmj_part0_opt.dot");

    ASSERT_EQ(part0_opt.num_cis(), 16);
    ASSERT_EQ(part0_opt.num_cos(), 13);
    ASSERT_EQ(part0_opt.num_gates(), 8); // one gate is optimized out.
    mockturtle::direct_resynthesis<aig_names> resyn0_back;
    auto part0_back = mockturtle::node_resynthesis<aig_names, aig_names>(part0_opt, resyn0_back);

    pm.integrate(0, part0, part0_back);
//    mockturtle::write_dot(pm.get_network(), "pmj_part0_integrated.dot");
    // ASSERT_EQ(pm.get_network().num_cis(), 15);
    // ASSERT_EQ(pm.get_network().num_cos(), 12);
    // ASSERT_EQ(pm.get_network().num_gates(), 18); // one gate is optimized out.

////////////////////////////////
    mockturtle::direct_resynthesis<aig_names> resyn1;
    auto part1_copy = mockturtle::node_resynthesis<aig_names, aig_window>(part1, resyn1);
//    mockturtle::write_dot(part1_copy, "pmj_part1_direct.dot");
    ASSERT_EQ(part1_copy.num_cis(), 11);
    ASSERT_EQ(part1_copy.num_cos(), 6);
    ASSERT_EQ(part1_copy.num_gates(), 9);

    oracle::aig_script opt1;
    auto part1_opt = opt1.run(part1_copy);
//    mockturtle::write_dot(part1_opt, "pmj_part1_opt.dot");

    ASSERT_EQ(part1_opt.num_cis(), 11);
    ASSERT_EQ(part1_opt.num_cos(), 6);
    ASSERT_EQ(part1_opt.num_gates(), 8); // one gate is optimized out.

    mockturtle::direct_resynthesis<aig_names> resyn1_back;
    auto part1_back = mockturtle::node_resynthesis<aig_names, aig_names>(part1_opt, resyn1_back);

    pm.integrate(1, part1, part1_back);
    pm.substitute_nodes();
//    mockturtle::write_dot(pm.get_network(), "pmj_final.dot");
    ASSERT_EQ(pm.get_network().num_cis(), 15);
    ASSERT_EQ(pm.get_network().num_cos(), 12);
    ASSERT_EQ(pm.get_network().num_gates(), 17); // one gate is optimized out.
}

TEST(partition_manager_junior_test, pmj_partition_xmg_minimal)
{
    xmg_names net;
    xmg_names::signal pi0 = net.create_pi();
    net.set_name(pi0, "pi0");
    xmg_names::signal pi1 = net.create_pi();
    net.set_name(pi1, "pi1");
    xmg_names::signal pi2 = net.create_pi();
    net.set_name(pi2, "pi2");
    xmg_names::signal w0 = net.create_nand(pi0, pi1);
    net.set_name(w0, "w0");
    xmg_names::signal w1 = net.create_nand(pi1, pi2);;
    net.set_name(w1, "w1");
    xmg_names::signal w2 = net.create_nand(w0, w1);
    net.set_name(w1, "w2");
    uint32_t po0 = net.create_po(w2);
    uint32_t po1 = net.create_po(pi0);

    mockturtle::node_map<int, xmg_names> parts(net);
    parts[net.get_node(pi0)] = 0;
    parts[net.get_node(pi1)] = 1;
    parts[net.get_node(pi2)] = 1;
    parts[net.get_node(w0)] = 0;
    parts[net.get_node(w1)] = 1;
    parts[net.get_node(w2)] = 0;
    xmg_manager pm(net, parts, 2);
    mockturtle::write_dot(pm.get_network(), "minimal_start.dot");

    ASSERT_EQ(pm.get_network().num_cis(), 3);
    ASSERT_EQ(pm.get_network().num_cos(), 2);
    ASSERT_EQ(pm.get_network().num_gates(), 3);

    xmg_window part0 = pm.partition(0);
    mockturtle::write_dot(part0, "minimal_part0.dot");
    ASSERT_EQ(part0.num_cis(), 3);
    ASSERT_EQ(part0.num_cos(), 2);
    ASSERT_EQ(part0.num_gates(), 2);

    xmg_window part1 = pm.partition(1);
    mockturtle::write_dot(part1, "minimal_part1.dot");
    ASSERT_EQ(part1.num_cis(), 2);
    ASSERT_EQ(part1.num_cos(), 2);
    ASSERT_EQ(part1.num_gates(), 1);


    mockturtle::direct_resynthesis<xmg_names> resyn0;
    auto part0_copy = mockturtle::node_resynthesis<xmg_names, xmg_window>(part0, resyn0);
    pm.integrate(0, part0, part0_copy);

    mockturtle::write_dot(pm.get_network(), "minimal.dot");
    ASSERT_EQ(pm.get_network().num_cis(), 3);
    ASSERT_EQ(pm.get_network().num_cos(), 2);
    ASSERT_EQ(pm.get_network().num_gates(), 3);

}

TEST(partition_manager_junior_test, pmj_partition_xmg)
{
    xmg_names net;
    xmg_names::signal pi0 = net.create_pi();
    net.set_name(pi0, "pi0");
    xmg_names::signal pi1 = net.create_pi();
    net.set_name(pi1, "pi1");
    xmg_names::signal pi2 = net.create_pi();
    net.set_name(pi2, "pi2");
    xmg_names::signal pi3 = net.create_pi();
    net.set_name(pi3, "pi3");
    xmg_names::signal pi4 = net.create_pi();
    net.set_name(pi4, "pi4");
    xmg_names::signal pi5 = net.create_pi();
    net.set_name(pi5, "pi5");
    xmg_names::signal pi6 = net.create_pi();
    net.set_name(pi6, "pi6");
    xmg_names::signal pi7 = net.create_pi();
    net.set_name(pi7, "pi7");
    xmg_names::signal pi8 = net.create_pi();
    net.set_name(pi8, "pi8");
    xmg_names::signal pi9 = net.create_pi();
    net.set_name(pi9, "pi9");
    xmg_names::signal pi10 = net.create_pi();
    net.set_name(pi10, "pi10");
    xmg_names::signal pi11 = net.create_pi();
    net.set_name(pi11, "pi11");
    xmg_names::signal pi12 = net.create_pi();
    net.set_name(pi12, "pi12");
    xmg_names::signal pi13 = net.create_pi(); // isolated subnet
    net.set_name(pi13, "pi13");
    xmg_names::signal pi14 = net.create_pi(); // isolated subnet
    net.set_name(pi14, "pi14");
    xmg_names::signal w0 = net.create_nand(pi0, pi1);
    net.set_name(w0, "w0");
    xmg_names::signal w1 = net.create_nand(pi4, pi5);
    net.set_name(w1, "w1");
    xmg_names::signal w2 = net.create_nand(pi2, w1);
    net.set_name(w2, "w2");
    xmg_names::signal w3 = net.create_nand(w0, pi3);
    net.set_name(w3, "w3");
    xmg_names::signal w4 = net.create_nand(w0, w3);
    net.set_name(w4, "w4");
    xmg_names::signal w5 = net.create_nand(pi6, pi7);
    net.set_name(w5, "w5");
    xmg_names::signal w6 = net.create_nand(pi6, w5);
    net.set_name(w6, "w6");
    xmg_names::signal w7 = net.create_nand(w2, w6);
    net.set_name(w7, "w7");
    xmg_names::signal w8 = net.create_nand(pi8, pi9);
    net.set_name(w8, "w8");
    xmg_names::signal w10 = net.create_nand(pi8, pi11);
    net.set_name(w10, "w10");
    xmg_names::signal w9 = net.create_nand(w10, pi11);
    net.set_name(w9, "w9");
    xmg_names::signal w11 = net.create_nand(w4, w7);
    net.set_name(w11, "w11");
    xmg_names::signal w12 = net.create_nand(w7, w6);
    net.set_name(w12, "w12");
    xmg_names::signal w13 = net.create_nand(w8, w10);
    net.set_name(w13, "w13");
    xmg_names::signal w14 = net.create_nand(net.create_not(w6), w13);
    net.set_name(w14, "w14");
    xmg_names::signal w15 = net.create_nand(w0, w4);
    net.set_name(w15, "w15");
    xmg_names::signal w16 = net.create_nand(w14, w15);
    net.set_name(w16, "w16");
    xmg_names::signal w17 = net.create_nand(w9, w14);
    net.set_name(w17, "w17");
    xmg_names::signal w18 = net.create_nand(pi14, pi13);
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

    mockturtle::node_map<int, xmg_names> parts(net);
    parts[net.get_node(pi0)] = 0; // internal PI
    parts[net.get_node(pi1)] = 0; // internal PI
    parts[net.get_node(pi2)] = 0; // internal PI
    parts[net.get_node(pi3)] = 0; // internal PI accessed twice.
    parts[net.get_node(pi4)] = 1; // internal PI
    parts[net.get_node(pi5)] = 0; // external PI
    parts[net.get_node(pi6)] = 0; // external PI accessed twice
    parts[net.get_node(pi7)] = 1; // internal PI
    parts[net.get_node(pi8)] = 0; // shared PI accessed twice.
    parts[net.get_node(pi9)] = 0; // internal PI
    parts[net.get_node(pi10)] = 0; // PI passthrough
    parts[net.get_node(pi11)] = 0; // shared PI accessed twice.
    parts[net.get_node(pi12)] = 0; // PI passthrough
    parts[net.get_node(pi13)] = 0; // isolated PI
    parts[net.get_node(pi14)] = 0; // isolated PI
    parts[net.get_node(w0)] = 0; // internal gate, fanin both from internal PI
    parts[net.get_node(w1)] = 1; // internal gate, fanin from internal PI and external PI
    parts[net.get_node(w2)] = 0; // internal gate, fanin from internal PI and external gate
    parts[net.get_node(w3)] = 0; // internal gate, fanin from internal PI and internal gate
    parts[net.get_node(w4)] = 0; // internal gate, fanin both from internal gate
    parts[net.get_node(w5)] = 1;
    parts[net.get_node(w6)] = 1; // internal gate, fanin from internal gate and external PI
    parts[net.get_node(w7)] = 0; // internal gate, fanin from internal gate and external gate
    parts[net.get_node(w8)] = 1; // internal gate, fanin both from external PI
    parts[net.get_node(w9)] = 1; // internal gate, fanin from external PI and external gate
    parts[net.get_node(w10)] = 0; // internal gate, fanin both from internal PI
    parts[net.get_node(w11)] = 1; // internal gate, fanin both from external gate
    parts[net.get_node(w12)] = 1;
    parts[net.get_node(w13)] = 1;
    parts[net.get_node(w14)] = 0;
    parts[net.get_node(w15)] = 0;
    parts[net.get_node(w16)] = 0;
    parts[net.get_node(w17)] = 1;
    parts[net.get_node(w18)] = 0;

    mockturtle::direct_resynthesis<xmg_names> orig_resyn;
    xmg_names orig = mockturtle::node_resynthesis<xmg_names, xmg_names>(net, orig_resyn);

    xmg_manager pm(net, parts, 2);
    ASSERT_EQ(pm.count(), 2);
    mockturtle::write_dot(net, "pmj_full.dot");

    xmg_window part0 = pm.partition(0);
    mockturtle::write_dot(part0, "pmj_part0.dot");
    xmg_window part1 = pm.partition(1);
    mockturtle::write_dot(part1, "pmj_part1.dot");

    ASSERT_EQ(net.num_cis(), 15);
    ASSERT_EQ(net.num_cos(), 12);
    ASSERT_EQ(net.num_gates(), 19);

    std::vector<xmg_names::signal> expect_ci_0 = { pi0, pi1, pi2, pi3, pi5, pi6, pi8, pi9,
                                               pi10, pi11, pi12, pi13, pi14, w1, w6, w13 };
    ASSERT_EQ(part0.num_cis(), 16);
    for (auto i = expect_ci_0.begin(); i != expect_ci_0.end(); i++) {
        ASSERT_TRUE(part0.is_ci(part0.get_node(*i)));
    }

    std::vector<xmg_names::signal> expect_co_0 = {pi5, pi6, pi8, pi9, pi10, pi11, pi12,
                                              w4, w7, w10, w14, w15, w18 };
    ASSERT_EQ(part0.num_cos(), 13);
    part0.foreach_co([&](auto s, auto i) {
        ASSERT_EQ(part0.get_node(expect_co_0[i]), part0.get_node(s));
    });

    std::vector<xmg_names::signal> expect_gate_0 = {w0, w15, w16, w4, w3, w2, w7,w10, w14, w18 };
    ASSERT_EQ(part0.num_gates(), 10);
    for (auto i = expect_gate_0.begin(); i != expect_gate_0.end(); i++) {
        ASSERT_TRUE(part0.belongs_to(part0.get_node(*i)));
    }

    std::vector<xmg_names::signal> expect_ci_1 = { w4, pi4, pi5, pi6, pi7, pi8, w10, pi9, pi11, w7, w14 };
    ASSERT_EQ(part1.num_cis(), 11);
    for (auto i = expect_ci_1.begin(); i != expect_ci_1.end(); i++) {
        ASSERT_TRUE(part1.is_ci(part1.get_node(*i)));
    }

    std::vector<xmg_names::signal> expect_co_1 = { w1, w6, w11, w12, w13, w17 };
    ASSERT_EQ(part1.num_cos(), 6);
    part1.foreach_co([&](auto s, auto i) {
        ASSERT_EQ(part1.get_node(expect_co_1[i]), part1.get_node(s));
    });

    std::vector<xmg_names::signal> expect_gate_1 = { w1, w6, w5, w11, w12, w13, w9, w17, w8 };
    ASSERT_EQ(part1.num_gates(), 9);
    for (auto i = expect_gate_1.begin(); i != expect_gate_1.end(); i++) {
        ASSERT_TRUE(part1.belongs_to(part1.get_node(*i)));
    }

    ////////////////////////////////
    // Optimize partition 0.
    // Convert to XAG.
    mockturtle::direct_resynthesis<xag_names> resyn0;
    auto part0_copy = mockturtle::node_resynthesis<xag_names, xmg_window>(part0, resyn0);
    mockturtle::write_dot(part0_copy, "pmj_part0_direct.dot");

    ASSERT_EQ(part0_copy.num_cis(), 16);
    ASSERT_EQ(part0_copy.num_cos(), 13);
    ASSERT_EQ(part0_copy.num_gates(), 9); // the resynthesis drops the dangling edge with no PO.

    // Optimize XAG
    oracle::xag_script opt0;
    auto part0_opt = opt0.run(part0_copy);
    mockturtle::write_dot(part0_opt, "pmj_part0_opt.dot");

    ASSERT_EQ(part0_opt.num_cis(), 16);
    ASSERT_EQ(part0_opt.num_cos(), 13);
    ASSERT_EQ(part0_opt.num_gates(), 8);

    // Convert back to XAG
    mockturtle::direct_resynthesis<xmg_names> resyn0_back;
    auto part0_back = mockturtle::node_resynthesis<xmg_names, xag_names>(part0_opt, resyn0_back);
    ASSERT_EQ(part0_back.num_cis(), 16);
    ASSERT_EQ(part0_back.num_cos(), 13);
    ASSERT_EQ(part0_back.num_gates(), 8);
    mockturtle::write_dot(part0_back, "pmj_part0_back.dot");

    // Integrate into parent
    pm.integrate(0, part0, part0_back);
    mockturtle::write_dot(pm.get_network(), "pmj_part0_integrated.dot");
    // ASSERT_EQ(pm.get_network().num_cis(), 15);
    // ASSERT_EQ(pm.get_network().num_cos(), 12);
    // ASSERT_EQ(pm.get_network().num_gates(), 18); // one gate is optimized out.

    ////////////////////////////////
    // Optimize partition 1
    // Convert to AIG
    mockturtle::direct_resynthesis<aig_names> resyn1;
    // part1 = pm.partition(1);
    auto part1_copy = mockturtle::node_resynthesis<aig_names, xmg_window>(part1, resyn1);
    mockturtle::write_dot(part1_copy, "pmj_part1_direct.dot");
    ASSERT_EQ(part1_copy.num_cis(), 11);
    ASSERT_EQ(part1_copy.num_cos(), 6);
    ASSERT_EQ(part1_copy.num_gates(), 9);

    // Optimize AIG
    oracle::aig_script opt1;
    auto part1_opt = opt1.run(part1_copy);
    mockturtle::write_dot(part1_opt, "pmj_part1_opt.dot");

    ASSERT_EQ(part1_opt.num_cis(), 11);
    ASSERT_EQ(part1_opt.num_cos(), 6);
    ASSERT_EQ(part1_opt.num_gates(), 8); // one gate is optimized out.

    // Convert back to XMG
    mockturtle::direct_resynthesis<xmg_names> resyn1_back;
    auto part1_back = mockturtle::node_resynthesis<xmg_names, aig_names>(part1_opt, resyn1_back);
    ASSERT_EQ(part1_back.num_cis(), 11);
    ASSERT_EQ(part1_back.num_cos(), 6);
    ASSERT_EQ(part1_back.num_gates(), 8); // one gate is optimized out.
    mockturtle::write_dot(part1_back, "pmj_part1_back.dot");

    // Integrate into parent
    pm.integrate(1, part1, part1_back);
    // Final resubstitution
    pm.substitute_nodes();
    mockturtle::write_dot(pm.get_network(), "pmj_part1_integrated.dot");
    ASSERT_EQ(pm.get_network().num_cis(), 15);
    ASSERT_EQ(pm.get_network().num_cos(), 12);
    ASSERT_EQ(pm.get_network().num_gates(), 16); // one gate is optimized out.

    const auto miter = *mockturtle::miter<xmg_names>(orig, pm.get_network());
    const auto result = mockturtle::equivalence_checking(miter);

    ASSERT_TRUE(result);
}
