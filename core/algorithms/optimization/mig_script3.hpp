#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace oracle{
    class mig_script3{
    public:
        mockturtle::mig_network run(mockturtle::mig_network& mig){
            
            mockturtle::sop_rebalancing<mockturtle::mig_network> balfn;
            mockturtle::mig_npn_resynthesis resyn;
            mockturtle::cut_rewriting_params ps;
            mockturtle::mig_algebraic_depth_rewriting_params pm;
            mockturtle::balancing_params bs;

            bs.cut_enumeration_ps.cut_size = 4u;
            //pm.selective; //this line does nothing after mockturtle upgrade.  Revisit.

            ps.cut_enumeration_ps.cut_size = 4u;
            
            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            //rw
            mockturtle::cut_rewriting(mig, resyn, ps);
            // std::cout << "done cut rewriting\n";
            mig = mockturtle::cleanup_dangling(mig);
            // std::cout << "2\n";

            //rf
            mockturtle::depth_view mig_ref_depth1{mig};

            mockturtle::mig_algebraic_depth_rewriting(mig_ref_depth1, pm);
            mig = mockturtle::cleanup_dangling( mig );

            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            //rw
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling(mig);
            // std::cout << "5\n";

            ps.allow_zero_gain = true;

            //rwz
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling(mig);
            // std::cout << "6\n";
            
            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            //rfz
            // std::cout << "activate zero gain\n";
            mockturtle::depth_view mig_ref_depth2{mig};

            mockturtle::mig_algebraic_depth_rewriting(mig_ref_depth2, pm);
            mig = mockturtle::cleanup_dangling( mig );
            // std::cout << "8\n";

            //rwz
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling(mig);
            // std::cout << "9\n";
            
            //b
            mig = mockturtle::balancing(mig, {balfn}, bs);
            mig = mockturtle::cleanup_dangling(mig);

            return mig;
        }
    };
}