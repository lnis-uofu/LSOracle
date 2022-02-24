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
#include <mockturtle/algorithms/balancing.hpp>
#include <mockturtle/algorithms/balancing/sop_balancing.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace oracle
{
class aig_script2
{
public:
    mockturtle::names_view<mockturtle::aig_network> run(mockturtle::names_view<mockturtle::aig_network> &aig)
    {
        mockturtle::sop_rebalancing<mockturtle::names_view<mockturtle::aig_network>> balfn;
        mockturtle::xag_npn_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn;
        mockturtle::bidecomposition_resynthesis<mockturtle::names_view<mockturtle::aig_network>> rf_resyn;
        mockturtle::cut_rewriting_params ps;
        mockturtle::refactoring_params rp;
        mockturtle::balancing_params bs;
        //mockturtle::balancing_stats st4;

        //bs.progress = true;
        bs.cut_enumeration_ps.cut_size = 4u;
        ps.cut_enumeration_ps.cut_size = 4u;
        rp.allow_zero_gain = false;

        //b
        std::cout << "b\n";
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);
        mockturtle::functional_reduction(aig);

        //rw
        std::cout << "rw\n";
        mockturtle::cut_rewriting(aig, resyn, ps);
        aig = mockturtle::cleanup_dangling(aig);

        //rf
        std::cout << "rf\n";
        mockturtle::refactoring(aig, rf_resyn, rp);
        aig = mockturtle::cleanup_dangling(aig);

        //b
        std::cout << "b\n";
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);
        mockturtle::functional_reduction(aig);

        //rw
        std::cout << "rw\n";
        mockturtle::cut_rewriting(aig, resyn, ps);
        aig = mockturtle::cleanup_dangling(aig);

        //rwz
        std::cout << "rwz\n";
        ps.allow_zero_gain = true;
        mockturtle::cut_rewriting(aig, resyn, ps);
        aig = mockturtle::cleanup_dangling(aig);

        //b
        std::cout << "b\n";
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);
        mockturtle::functional_reduction(aig);

        //rfz
        std::cout << "rfz\n";
        rp.allow_zero_gain = true;
        mockturtle::refactoring(aig, rf_resyn, rp);
        std::cout << "refactored\n";
        aig = mockturtle::cleanup_dangling(aig);

        //rwz
        std::cout << "rwz\n";
        mockturtle::cut_rewriting(aig, resyn, ps);
        aig = mockturtle::cleanup_dangling(aig);

        //b
        std::cout << "b\n";
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);
        mockturtle::functional_reduction(aig);

        return aig;
    }
};
}
