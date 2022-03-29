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

        // std::cout << "HERE\n";
        mockturtle::xag_npn_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn;
        mockturtle::cut_rewriting_params ps;
        ps.cut_enumeration_ps.cut_size = 4;
        if (zero_gain)
            ps.allow_zero_gain = true;

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "1\n";
        // aig.foreach_pi([&](auto pi){
        //   std::cout << "PI: " << pi << "\n";
        // });
        // aig.foreach_gate([&](auto gate){
        //   std::cout << "Gate: " << gate << "\n";
        //   if(aig.is_po(gate))
        //     std::cout << "IS PO\n";
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     std::cout << "child[" << i << "] = " << conn.index << "\n";
        //   });
        // });
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "1 clean\n";
        // aig.foreach_pi([&](auto pi){
        //   std::cout << "PI: " << pi << "\n";
        // });
        // aig.foreach_gate([&](auto gate){
        //   std::cout << "Gate: " << gate << "\n";
        //   if(aig.is_po(gate))
        //     std::cout << "IS PO\n";
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     std::cout << "child[" << i << "] = " << conn.index << "\n";
        //   });
        // });

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "2\n";
        // aig.foreach_pi([&](auto pi){
        //   std::cout << "PI: " << pi << "\n";
        // });
        // aig.foreach_gate([&](auto gate){
        //   std::cout << "Gate: " << gate << "\n";
        //   if(aig.is_po(gate))
        //     std::cout << "IS PO\n";
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     std::cout << "child[" << i << "] = " << conn.index << "\n";
        //   });
        // });
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "2 clean\n";
        // aig.foreach_pi([&](auto pi){
        //   std::cout << "PI: " << pi << "\n";
        // });
        // aig.foreach_gate([&](auto gate){
        //   std::cout << "Gate: " << gate << "\n";
        //   if(aig.is_po(gate))
        //     std::cout << "IS PO\n";
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     std::cout << "child[" << i << "] = " << conn.index << "\n";
        //   });
        // });

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "3\n";
        // aig.foreach_pi([&](auto pi){
        //   std::cout << "PI: " << pi << "\n";
        // });
        // aig.foreach_gate([&](auto gate){
        //   std::cout << "Gate: " << gate << "\n";
        //   if(aig.is_po(gate))
        //     std::cout << "IS PO\n";
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     std::cout << "child[" << i << "] = " << conn.index << "\n";
        //   });
        // });
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "3 clean\n";
        // aig.foreach_pi([&](auto pi){
        //   std::cout << "PI: " << pi << "\n";
        // });
        // aig.foreach_gate([&](auto gate){
        //   std::cout << "Gate: " << gate << "\n";
        //   if(aig.is_po(gate))
        //     std::cout << "IS PO\n";
        //   aig.foreach_fanin(gate, [&](auto conn, auto i){
        //     std::cout << "child[" << i << "] = " << conn.index << "\n";
        //   });
        // });

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "4\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "4 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "5\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "5 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "6\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "6 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "7\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "7 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "8\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "8 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "9\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "9 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "10\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "10 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "11\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "11 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "12\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "12 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "13\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "13 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "14\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "14 clean\n";

        mockturtle::cut_rewriting(aig, resyn, ps);
        // std::cout << "15\n";
        aig = mockturtle::cleanup_dangling(aig);
        // std::cout << "15 clean\n";

        return aig;
    }
};
}
