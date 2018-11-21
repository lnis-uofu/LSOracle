#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace mockturtle{
    class mig_script{
    public:
        mockturtle::mig_network run(mockturtle::mig_network& mig){
            mockturtle::depth_view mig_depth{mig};

            mockturtle::mig_algebraic_depth_rewriting_params pm;
            pm.aggressive;

            mockturtle::mig_algebraic_depth_rewriting(mig_depth, pm);

            mig = mockturtle::cleanup_dangling( mig );

            // AREA RECOVERING
            mockturtle::mig_npn_resynthesis resyn;
            mockturtle::cut_rewriting_params ps;

            ps.cut_enumeration_ps.cut_size = 4;

            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling( mig );

            // AREA RECOVERING
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling( mig );

            //DEPTH REWRITING
            mockturtle::depth_view mig_depth1{mig};

            mockturtle::mig_algebraic_depth_rewriting(mig_depth1, pm);
            mig = mockturtle::cleanup_dangling( mig );

            // AREA RECOVERING
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling( mig );

            // AREA RECOVERING
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling( mig );

            //DEPTH REWRITING
            mockturtle::depth_view mig_depth2{mig};

            mockturtle::mig_algebraic_depth_rewriting(mig_depth2, pm);
            mig = mockturtle::cleanup_dangling( mig );

            // AREA RECOVERING
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling( mig );

            // AREA RECOVERING
            mockturtle::cut_rewriting(mig, resyn, ps);
            mig = mockturtle::cleanup_dangling( mig );

            //DEPTH REWRITING
            mockturtle::depth_view mig_depth3{mig};

            mockturtle::mig_algebraic_depth_rewriting(mig_depth3, pm);
            mig = mockturtle::cleanup_dangling( mig );

            std::cout << "Mig gates after area optimization " << mig.num_gates() << " MIG depth " << mig_depth3.depth() << std::endl;

            return mig;
        }
    };
}