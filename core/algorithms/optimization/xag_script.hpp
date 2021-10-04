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

class xag_script
{
public:
    mockturtle::names_view<mockturtle::xag_network> run(mockturtle::names_view<mockturtle::xag_network> &xag)
    {
        mockturtle::sop_rebalancing<mockturtle::names_view<mockturtle::xag_network>> balfn;
        mockturtle::xag_npn_resynthesis<mockturtle::names_view<mockturtle::xag_network>> resyn;
        mockturtle::refactoring_params rp;
        mockturtle::balancing_params bs;
        mockturtle::cut_rewriting_params ps;
        ps.cut_enumeration_ps.cut_size = 4u;
        bs.cut_enumeration_ps.cut_size = 4u;
        rp.allow_zero_gain = false;
        rp.max_pis = 4;

        mockturtle::cut_rewriting(xag, resyn, ps); //rw
	std::cout << xag.get_network_name() << std::endl;
        xag = mockturtle::cleanup_dangling(xag);
	std::cout << xag.get_network_name() << std::endl;
        mockturtle::cut_rewriting(xag, resyn, ps); //rw
	std::cout << xag.get_network_name() << std::endl;
        xag = mockturtle::cleanup_dangling(xag);
	std::cout << xag.get_network_name() << std::endl;

        xag = mockturtle::balancing(xag, {balfn}, bs); //b
	std::cout << xag.get_network_name() << std::endl;
        xag = mockturtle::cleanup_dangling(xag);
	std::cout << xag.get_network_name() << std::endl;
        mockturtle::refactoring(xag, resyn, rp); //rf
	std::cout << xag.get_network_name() << std::endl;
        xag = mockturtle::cleanup_dangling(xag);
	std::cout << xag.get_network_name() << std::endl;

        xag = mockturtle::xag_constant_fanin_optimization(xag); //find XORs
	std::cout << xag.get_network_name() << std::endl;

        return xag;
    }
};
}
