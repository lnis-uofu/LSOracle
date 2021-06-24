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

namespace oracle{
    class aig_script2{
    public:
        mockturtle::aig_network run(mockturtle::aig_network& aig){
            mockturtle::sop_rebalancing<mockturtle::aig_network> balfn;
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::bidecomposition_resynthesis<mockturtle::aig_network> rf_resyn;
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
            //mockturtle::depth_view aig_depth{aig};
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);

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
            //mockturtle::depth_view aig_depth1{aig};
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);

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
            //mockturtle::depth_view aig_depth2{aig};
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);

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
            //mockturtle::depth_view aig_depth3{aig};
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);
            
            return aig;
        }
    };
}