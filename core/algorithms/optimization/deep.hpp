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
class aig_deep
{
public:
    mockturtle::aig_network run(mockturtle::aig_network &aig, int effort)
    {

        mockturtle::sop_rebalancing<mockturtle::aig_network> balfn;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
        mockturtle::bidecomposition_resynthesis<mockturtle::aig_network> fallback;
        mockturtle::dsd_resynthesis<mockturtle::aig_network, decltype(fallback)>
        rf_resyn(fallback);
        mockturtle::cut_rewriting_params ps;
        mockturtle::refactoring_params rp;
        mockturtle::balancing_params bs;
        mockturtle::akers_resynthesis<mockturtle::mig_network> resyn_mig;

        bs.cut_enumeration_ps.cut_size = 4u;
        /* Cut size */
        ps.cut_enumeration_ps.cut_size = 4;
        rp.allow_zero_gain = false;

        /* Interleaves AIG (try with XAIG) and MIG based optos */
        int i = 0; 

        for ( i = 0; i < effort; i++ ) {
            //rw
            // std::cout << "rw\n";
            // mockturtle::cut_rewriting(aig, resyn, ps);
            // aig = mockturtle::cleanup_dangling(aig);
            
            //rf
            std::cout << "rf\n";
            mockturtle::refactoring(aig, rf_resyn, rp);
            aig = mockturtle::cleanup_dangling(aig);
                    
            //rw
            std::cout << "rw\n";
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            //resub
            std::cout << "resub\n";
            mockturtle::aig_resubstitution(aig);
            aig = mockturtle::cleanup_dangling(aig);

            //rf
            std::cout << "rf\n";
            mockturtle::refactoring(aig, rf_resyn, rp);
            aig = mockturtle::cleanup_dangling(aig);
            
            //resub
            std::cout << "resub\n";
            mockturtle::aig_resubstitution(aig);
            aig = mockturtle::cleanup_dangling(aig);

            //rwz
            std::cout << "rwz\n";
            ps.allow_zero_gain = true;
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            //rfz
            std::cout << "rfz\n";
            rp.allow_zero_gain = true;
            mockturtle::refactoring(aig, rf_resyn, rp);
            aig = mockturtle::cleanup_dangling(aig);
            
            //rw
            std::cout << "rw\n";
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);
            
            //resub
            std::cout << "resub\n";
            mockturtle::aig_resubstitution(aig);
            aig = mockturtle::cleanup_dangling(aig);
        
            // mockturtle::mapping_view<mockturtle::aig_network, true> mapped_aig{aig};
            // mockturtle::lut_mapping_params ps_lut;
            // ps_lut.cut_enumeration_ps.cut_size = 4;
            // mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>( mapped_aig, ps_lut );
            // //mockturtle::satlut_mapping<mockturtle::aig_network> (mapped_aig, 32);

            // /* collapse into k-LUT network */
            // auto klut = *mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_aig );

            // /* node resynthesis */
            // auto mig = mockturtle::node_resynthesis<mockturtle::mig_network>( klut, resyn_mig );
            // mockturtle::mig_algebraic_depth_rewriting_params pm;

            // //rw
            // mockturtle::cut_rewriting(mig, resyn_mig, ps);
            // // std::cout << "done cut rewriting\n";
            // mig = mockturtle::cleanup_dangling(mig);
            // // std::cout << "2\n";

            // //rw depth
            // mockturtle::depth_view mig_ref_depth1{mig};
            // mockturtle::mig_algebraic_depth_rewriting(mig_ref_depth1, pm);
            // mig = mockturtle::cleanup_dangling(mig);

            // //rwz
            // mockturtle::cut_rewriting(mig, resyn_mig, ps);
            // mig = mockturtle::cleanup_dangling(mig);

            // // mockturtle::mig_resubstitution( mig );
            // // mig = cleanup_dangling( mig );

            // mockturtle::refactoring(mig, resyn_mig);
            // mig = mockturtle::cleanup_dangling( mig );

            // mockturtle::cut_rewriting(mig, resyn_mig, ps);
            // mig = mockturtle::cleanup_dangling(mig);

            // mockturtle::mapping_view<mockturtle::mig_network, true> mapped_mig{mig};
            // mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped_mig, ps_lut );
            // //mockturtle::satlut_mapping<mockturtle::mig_network> (mapped_mig, 32);
            // /* collapse into k-LUT network */
            // klut = *mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_mig );

            // /* node resynthesis */
            // aig = mockturtle::node_resynthesis<mockturtle::aig_network>( klut, resyn );

            //rw
            std::cout << "rw\n";
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            //rf
            std::cout << "rf\n";
            mockturtle::refactoring(aig, rf_resyn, rp);
            aig = mockturtle::cleanup_dangling(aig);
                    
            //rw
            std::cout << "rw\n";
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            //resub
            std::cout << "resub\n";
            mockturtle::aig_resubstitution(aig);
            aig = mockturtle::cleanup_dangling(aig);

            //rf
            std::cout << "rf\n";
            mockturtle::refactoring(aig, rf_resyn, rp);
            aig = mockturtle::cleanup_dangling(aig);
            
            //resub
            std::cout << "resub\n";
            mockturtle::aig_resubstitution(aig);
            aig = mockturtle::cleanup_dangling(aig);

            //rwz
            std::cout << "rwz\n";
            ps.allow_zero_gain = true;
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);
            
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);
            // if ( ps_lut.cut_enumeration_ps.cut_size == 8 ) {
            //     ps_lut.cut_enumeration_ps.cut_size = 2;
            // } else {
            //     ps_lut.cut_enumeration_ps.cut_size += 2; 
            // }
        }
        return aig;
    }
};
}
