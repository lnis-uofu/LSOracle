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
#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace oracle
{

class rw_script
{
public:
    mockturtle::names_view<mockturtle::aig_network> run(mockturtle::names_view<mockturtle::aig_network> &aig,
                                bool zero_gain = false)
    {

        // spdlog::debug("HERE");
        mockturtle::xag_npn_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn;
        mockturtle::cut_rewriting_params ps;
        ps.cut_enumeration_ps.cut_size = 4;
        if (zero_gain)
            ps.allow_zero_gain = true;

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("1");
        // aig.foreach_pi([&](auto pi){
        //   spdlog::debug("PI: {}",pi);
        // });
        // aig.foreach_gate([&](auto gate){
        //   spdlog::debug("Gate: {}",gate);
        //   if(aig.is_po(gate))
        //     spdlog::debug("IS PO");
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     spdlog::debug("child[{}",i << "] = {}",conn.index);
        //   });
        // });
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("1 clean");
        // aig.foreach_pi([&](auto pi){
        //   spdlog::debug("PI: {}",pi);
        // });
        // aig.foreach_gate([&](auto gate){
        //   spdlog::debug("Gate: {}",gate);
        //   if(aig.is_po(gate))
        //     spdlog::debug("IS PO");
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     spdlog::debug("child[{}",i << "] = {}",conn.index);
        //   });
        // });

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("2");
        // aig.foreach_pi([&](auto pi){
        //   spdlog::debug("PI: {}",pi);
        // });
        // aig.foreach_gate([&](auto gate){
        //   spdlog::debug("Gate: {}",gate);
        //   if(aig.is_po(gate))
        //     spdlog::debug("IS PO");
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     spdlog::debug("child[{}",i << "] = {}",conn.index);
        //   });
        // });
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("2 clean");
        // aig.foreach_pi([&](auto pi){
        //   spdlog::debug("PI: {}",pi);
        // });
        // aig.foreach_gate([&](auto gate){
        //   spdlog::debug("Gate: {}",gate);
        //   if(aig.is_po(gate))
        //     spdlog::debug("IS PO");
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     spdlog::debug("child[{}",i << "] = {}",conn.index);
        //   });
        // });

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("3");
        // aig.foreach_pi([&](auto pi){
        //   spdlog::debug("PI: {}",pi);
        // });
        // aig.foreach_gate([&](auto gate){
        //   spdlog::debug("Gate: {}",gate);
        //   if(aig.is_po(gate))
        //     spdlog::debug("IS PO");
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     spdlog::debug("child[{}",i << "] = {}",conn.index);
        //   });
        // });
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("3 clean");
        // aig.foreach_pi([&](auto pi){
        //   spdlog::debug("PI: {}",pi);
        // });
        // aig.foreach_gate([&](auto gate){
        //   spdlog::debug("Gate: {}",gate);
        //   if(aig.is_po(gate))
        //     spdlog::debug("IS PO");
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     spdlog::debug("child[{}",i << "] = {}",conn.index);
        //   });
        // });

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("4");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("4 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("5");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("5 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("6");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("6 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("7");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("7 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("8");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("8 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("9");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("9 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("10");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("10 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("11");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("11 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("12");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("12 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("13");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("13 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("14");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("14 clean");

        mockturtle::cut_rewriting(aig, resyn, ps);
        // spdlog::debug("15");
        aig = mockturtle::cleanup_dangling(aig);
        // spdlog::debug("15 clean");
        return aig;
    }
};
}
