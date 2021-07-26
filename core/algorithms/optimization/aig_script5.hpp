#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace oracle{
    class aig_script5{
    public:
        mockturtle::aig_network run(mockturtle::aig_network& aig){
            
            mockturtle::sop_rebalancing<mockturtle::aig_network> balfn;
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::refactoring_params rp;
            mockturtle::balancing_params bs;

            bs.cut_enumeration_ps.cut_size = 4u;

            oracle::rw_script rw_opt;

            rp.allow_zero_gain = false;
            rp.max_pis = 4;
            
            //b
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);

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

            mockturtle::depth_view b_depth4{aig};
            
            return aig;
        }
    };
}