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

using network = mockturtle::names_view<mockturtle::aig_network>;

TEST(partition_manager_junior_test, pmj_partition_generation)
{
    network net;
    network::signal pi0 = net.create_pi();
    net.set_name(pi0, "pi0");
    network::signal pi1 = net.create_pi();
    net.set_name(pi1, "pi1");
    network::signal pi2 = net.create_pi();
    net.set_name(pi2, "pi2");
    network::signal pi3 = net.create_pi();
    net.set_name(pi3, "pi3");
    network::signal pi4 = net.create_pi();
    net.set_name(pi4, "pi4");
    network::signal pi5 = net.create_pi();
    net.set_name(pi5, "pi5");
    network::signal pi6 = net.create_pi();
    net.set_name(pi6, "pi6");
    network::signal pi7 = net.create_pi();
    net.set_name(pi7, "pi7");
    network::signal pi8 = net.create_pi();
    net.set_name(pi8, "pi8");
    network::signal pi9 = net.create_pi();
    net.set_name(pi9, "pi9");
    network::signal pi10 = net.create_pi();
    net.set_name(pi10, "pi10");
    network::signal pi11 = net.create_pi();
    net.set_name(pi11, "pi11");
    network::signal pi12 = net.create_pi();
    net.set_name(pi12, "pi12");
    network::signal pi13 = net.create_pi(); // isolated subnet
    net.set_name(pi13, "pi13");
    network::signal pi14 = net.create_pi(); // isolated subnet
    net.set_name(pi14, "pi14");
    network::signal w0 = net.create_nand(pi0, pi1);
    net.set_name(w0, "w0");
    network::signal w1 = net.create_nand(pi4, pi5);
    net.set_name(w1, "w1");
    network::signal w2 = net.create_nand(pi2, w1);
    net.set_name(w2, "w2");
    network::signal w3 = net.create_nand(w0, pi3);
    net.set_name(w3, "w3");
    network::signal w4 = net.create_nand(w0, w3);
    net.set_name(w4, "w4");
    network::signal w5 = net.create_nand(pi6, pi7);
    net.set_name(w5, "w5");
    network::signal w6 = net.create_nand(pi6, w5);
    net.set_name(w6, "w6");
    network::signal w7 = net.create_nand(w2, w6);
    net.set_name(w7, "w7");
    network::signal w8 = net.create_nand(pi8, pi9);
    net.set_name(w8, "w8");
    network::signal w10 = net.create_nand(pi8, pi11);
    net.set_name(w10, "w10");
    network::signal w9 = net.create_nand(w10, pi11);
    net.set_name(w9, "w9");
    network::signal w11 = net.create_nand(w4, w7);
    net.set_name(w11, "w11");
    network::signal w12 = net.create_nand(w7, w6);
    net.set_name(w12, "w12");
    network::signal w13 = net.create_nand(w8, w10);
    net.set_name(w13, "w13");
    network::signal w14 = net.create_nand(net.create_not(w6), w13);
    net.set_name(w14, "w14");
    network::signal w15 = net.create_nand(w0, w4);
    net.set_name(w15, "w15");
    network::signal w16 = net.create_nand(w14, w15);
    net.set_name(w16, "w16");
    network::signal w17 = net.create_nand(w9, w14);
    net.set_name(w17, "w17");
    network::signal w18 = net.create_nand(pi14, pi13);
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

    mockturtle::node_map<int, network> parts(net);
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

    oracle::partition_manager_junior<mockturtle::aig_network> pm(net, parts, 2);
    ASSERT_EQ(pm.count(), 2);
    mockturtle::write_dot(net, "pmj_full.dot");

    mockturtle::window_view<network> part0 = pm.partition(0);
    mockturtle::write_dot(part0, "pmj_part0.dot");
    mockturtle::window_view<network> part1 = pm.partition(1);
    mockturtle::write_dot(part1, "pmj_part1.dot");

    ASSERT_EQ(net.num_cis(), 15);
    ASSERT_EQ(net.num_cos(), 12);
    ASSERT_EQ(net.num_gates(), 19);

    std::vector<network::signal> expect_ci_0 = { pi0, pi1, pi2, pi3, pi5, pi6, pi8, pi9,
                                               pi10, pi11, pi12, pi13, pi14, w1, w6, w13 };
    ASSERT_EQ(part0.num_cis(), 16);
    for (auto i = expect_ci_0.begin(); i != expect_ci_0.end(); i++) {
        ASSERT_TRUE(part0.is_ci(part0.get_node(*i)));
    }

    std::vector<network::signal> expect_co_0 = {pi5, pi6, pi8, pi9, pi10, pi11, pi12,
                                              w4, w7, w10, w14, w15, w18 };
    ASSERT_EQ(part0.num_cos(), 13);
    part0.foreach_co([&](auto s, auto i) {
        ASSERT_EQ(part0.get_node(expect_co_0[i]), part0.get_node(s));
    });

    std::vector<network::signal> expect_gate_0 = {w0, w15, w16, w4, w3, w2, w7,w10, w14, w18 };
    ASSERT_EQ(part0.num_gates(), 10);
    for (auto i = expect_gate_0.begin(); i != expect_gate_0.end(); i++) {
        ASSERT_TRUE(part0.belongs_to(part0.get_node(*i)));
    }

    std::vector<network::signal> expect_ci_1 = { w4, pi4, pi5, pi6, pi7, pi8, w10, pi9, pi11, w7, w14 };
    ASSERT_EQ(part1.num_cis(), 11);
    for (auto i = expect_ci_1.begin(); i != expect_ci_1.end(); i++) {
        ASSERT_TRUE(part1.is_ci(part1.get_node(*i)));
    }

    std::vector<network::signal> expect_co_1 = { w1, w6, w11, w12, w13, w17 };
    ASSERT_EQ(part1.num_cos(), 6);
    part1.foreach_co([&](auto s, auto i) {
        ASSERT_EQ(part1.get_node(expect_co_1[i]), part1.get_node(s));
    });

    std::vector<network::signal> expect_gate_1 = { w1, w6, w5, w11, w12, w13, w9, w17, w8 };
    ASSERT_EQ(part1.num_gates(), 9);
    for (auto i = expect_gate_1.begin(); i != expect_gate_1.end(); i++) {
        ASSERT_TRUE(part1.belongs_to(part1.get_node(*i)));
    }

    ////////////////////////////////
    mockturtle::direct_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn0;
    auto part0_copy = mockturtle::node_resynthesis<mockturtle::names_view<mockturtle::aig_network>, mockturtle::window_view<network>>(part0, resyn0);
    mockturtle::write_dot(part0_copy, "pmj_part0_direct.dot");

    ASSERT_EQ(part0_copy.num_cis(), 16);
    ASSERT_EQ(part0_copy.num_cos(), 13);
    ASSERT_EQ(part0_copy.num_gates(), 9); // the resynthesis drops the dangling edge with no PO.

    oracle::aig_script opt0;
    auto part0_opt = opt0.run(part0_copy);
    mockturtle::write_dot(part0_opt, "pmj_part0_opt.dot");

    ASSERT_EQ(part0_opt.num_cis(), 16);
    ASSERT_EQ(part0_opt.num_cos(), 13);
    ASSERT_EQ(part0_opt.num_gates(), 8); // one gate is optimized out.

    pm.integrate(0, part0, part0_opt);
    mockturtle::write_dot(pm.get_network(), "pmj_part0_integrated.dot");
    ASSERT_EQ(pm.get_network().num_cis(), 15);
    ASSERT_EQ(pm.get_network().num_cos(), 12);
    ASSERT_EQ(pm.get_network().num_gates(), 18); // one gate is optimized out.

////////////////////////////////

    // mockturtle::xag_npn_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn2;
    // auto part0_npn = mockturtle::node_resynthesis<mockturtle::names_view<mockturtle::aig_network>, mockturtle::window_view<network>>(part0, resyn2);
    // mockturtle::write_dot(part0_npn, "pmj_part0_npn.dot");

    // mockturtle::direct_resynthesis<mockturtle::names_view<mockturtle::mig_network>> resyn4;
    // auto part0_mig = mockturtle::node_resynthesis<mockturtle::names_view<mockturtle::mig_network>, mockturtle::window_view<network>>(part0, resyn4);
    // mockturtle::write_dot(part0_mig, "pmj_part0_mig.dot");

    // mockturtle::topo_view part0_topo(part0);
    // mockturtle::write_dot(part0_topo, "pmj_part0_topo.dot");

    // mockturtle::direct_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn3;
    // auto part0_topo_copy = mockturtle::node_resynthesis<mockturtle::names_view<mockturtle::aig_network>, mockturtle::window_view<network>>(part0_topo, resyn3);
    // mockturtle::write_dot(part0_topo_copy, "pmj_part0_topo_direct.dot");

    // mockturtle::topo_view part1_topo(part1);
    // mockturtle::write_dot(part1_topo, "pmj_part1_topo.dot");


    // oracle::aig_script2 opt0_2;
    // auto part0_opt2 = opt0_2.run(part0_copy);
    // mockturtle::write_dot(part0_opt2, "pmj_part0_opt2.dot");
    mockturtle::direct_resynthesis<network> resyn1;
    auto part1_copy = mockturtle::node_resynthesis<network, mockturtle::window_view<network>>(part1, resyn1);
    mockturtle::write_dot(part1_copy, "pmj_part1_direct.dot");
    ASSERT_EQ(part1_copy.num_cis(), 11);
    ASSERT_EQ(part1_copy.num_cos(), 6);
    ASSERT_EQ(part1_copy.num_gates(), 9);

    oracle::aig_script opt1;
    auto part1_opt = opt1.run(part1_copy);
    mockturtle::write_dot(part1_opt, "pmj_part1_opt.dot");

    ASSERT_EQ(part1_opt.num_cis(), 11);
    ASSERT_EQ(part1_opt.num_cos(), 6);
    ASSERT_EQ(part1_opt.num_gates(), 8); // one gate is optimized out.

    pm.integrate(1, part1, part1_opt);

    mockturtle::write_dot(pm.get_network(), "pmj_final.dot");
    ASSERT_EQ(pm.get_network().num_cis(), 15);
    ASSERT_EQ(pm.get_network().num_cos(), 12);
    ASSERT_EQ(pm.get_network().num_gates(), 17); // one gate is optimized out.
}
