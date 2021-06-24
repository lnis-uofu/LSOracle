#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace oracle{
    class mig_script2{
    public:
        mockturtle::mig_network run(mockturtle::mig_network& mig){
            
            mockturtle::sop_rebalancing<mockturtle::mig_network> balfn;
            mockturtle::mig_npn_resynthesis resyn;
            mockturtle::akers_resynthesis<mockturtle::mig_network> rf_resyn;
            mockturtle::cut_rewriting_params ps;
            mockturtle::refactoring_params rp;
            mockturtle::balancing_params bs;

            bs.cut_enumeration_ps.cut_size = 4u;
            ps.cut_enumeration_ps.cut_size = 4u;
            rp.allow_zero_gain = false;
            
            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            //rw
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling(mig);

            //rf
            mockturtle::refactoring(mig, rf_resyn, rp);
            mig = mockturtle::cleanup_dangling(mig);

            mockturtle::depth_view mig_depth1{mig};

            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            //rw
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling(mig);

            ps.allow_zero_gain = true;

            //rwz
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling(mig);

            mockturtle::depth_view mig_depth2{mig};
            
            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            //rfz
            rp.allow_zero_gain = true;
            mockturtle::refactoring(mig, rf_resyn, rp);
            mig = mockturtle::cleanup_dangling(mig);

            //rwz
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling(mig);

            mockturtle::depth_view mig_depth3{mig};
            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            return mig;
        }
    };
}