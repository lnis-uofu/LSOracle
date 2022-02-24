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
class mig_script3
{
public:
    mockturtle::names_view<mockturtle::mig_network> run(mockturtle::names_view<mockturtle::mig_network> &mig)
    {

        mockturtle::sop_rebalancing<mockturtle::names_view<mockturtle::mig_network>> balfn;
        mockturtle::mig_npn_resynthesis resyn;
        mockturtle::cut_rewriting_params ps;
        mockturtle::mig_algebraic_depth_rewriting_params pm;
        mockturtle::balancing_params bs;

        bs.cut_enumeration_ps.cut_size = 4u;
        //pm.selective; //this line does nothing after mockturtle upgrade.  Revisit.

        ps.cut_enumeration_ps.cut_size = 4u;
        //rf
        mockturtle::depth_view mig_ref_depth1{mig};
        mockturtle::mig_algebraic_depth_rewriting(mig_ref_depth1, pm);
        mig = mockturtle::cleanup_dangling(mig);
/*
        //b
        mig = mockturtle::balancing(mig, {balfn}, bs);
        mig = mockturtle::cleanup_dangling(mig);
*/
        //rw
        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling(mig);

        //rw
        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling(mig);

        //rwz
        ps.allow_zero_gain = true;
        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling(mig);
        ps.allow_zero_gain = false;
/*
        //b
        mig = mockturtle::balancing(mig, {balfn}, bs);
        mig = mockturtle::cleanup_dangling(mig);
*/    
        //rw
        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling(mig);
        
        //rw
        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling(mig);

        //rwz
        ps.allow_zero_gain = true;
        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling(mig);
/*
        //b
        mig = mockturtle::balancing(mig, {balfn}, bs);
        mig = mockturtle::cleanup_dangling(mig);
*/
        return mig;
    }
};
}
