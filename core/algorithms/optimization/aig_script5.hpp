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
#include "rw_script.hpp"

namespace oracle
{
class aig_script5
{
public:
    mockturtle::names_view<mockturtle::aig_network> run(mockturtle::names_view<mockturtle::aig_network> &aig)
    {

        mockturtle::sop_rebalancing<mockturtle::names_view<mockturtle::aig_network>> balfn;
        mockturtle::xag_npn_resynthesis<mockturtle::names_view<mockturtle::aig_network>> resyn;
        mockturtle::refactoring_params rp;
        mockturtle::balancing_params bs;

        bs.cut_enumeration_ps.cut_size = 4u;

        oracle::rw_script rw_opt;

        rp.allow_zero_gain = false;
        rp.max_pis = 4;

        mockturtle::functional_reduction(aig);
/*
        //b
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);
*/
        mockturtle::depth_view b_depth1{aig};

        //rw
        aig = rw_opt.run(aig);

        mockturtle::depth_view rw_depth1{aig};

        //rf
        mockturtle::refactoring(aig, resyn, rp);
        aig = mockturtle::cleanup_dangling(aig);

        mockturtle::depth_view rf_depth1{aig};

        //b
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);

        mockturtle::depth_view b_depth2{aig};

        //rw
        aig = rw_opt.run(aig);

        mockturtle::depth_view rw_depth2{aig};

        //rwz
        aig = rw_opt.run(aig, true);

        mockturtle::depth_view rwz_depth1{aig};

        //b
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);

        mockturtle::depth_view b_depth3{aig};

        //rfz
        rp.allow_zero_gain = true;
        mockturtle::refactoring(aig, resyn, rp);
        aig = mockturtle::cleanup_dangling(aig);

        mockturtle::depth_view rfz_depth1{aig};

        //rwz
        aig = rw_opt.run(aig, true);

        mockturtle::depth_view rwz_depth2{aig};

        //b
        aig = mockturtle::balancing(aig, {balfn}, bs);
        aig = mockturtle::cleanup_dangling(aig);
        mockturtle::functional_reduction(aig);
        mockturtle::depth_view b_depth4{aig};

        return aig;
    }
};
}
