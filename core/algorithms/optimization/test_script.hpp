#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace oracle{
    class test_script{
    public:
        mockturtle::aig_network run(mockturtle::aig_network& aig){
            
            mockturtle::sop_rebalancing<mockturtle::aig_network> balfn;
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::bidecomposition_resynthesis<mockturtle::aig_network> fallback;
            mockturtle::dsd_resynthesis<mockturtle::aig_network, decltype( fallback )> rf_resyn( fallback );
            mockturtle::cut_rewriting_params ps;
            mockturtle::refactoring_params rp;
            mockturtle::balancing_params bs;

            bs.cut_enumeration_ps.cut_size = 4u;
            ps.cut_enumeration_ps.cut_size = 4u;
            rp.allow_zero_gain = false;
            
            //b
            std::cout << "b\n";
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);

            //rw
            std::cout << "rw\n";
            oracle::rw_script aigopt;
            aig = aigopt.run(aig);

            //rf
            std::cout << "rf\n";
            mockturtle::refactoring(aig, rf_resyn, rp);
            aig = mockturtle::cleanup_dangling(aig);

            //b
            std::cout << "b\n";
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);

            //rw
            std::cout << "rw\n";
            aig = aigopt.run(aig);

            //rwz
            std::cout << "rw\n";
            aig = aigopt.run(aig);
            
            //b
            std::cout << "b\n";
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
            aig = aigopt.run(aig);
            
            //b
            std::cout << "b\n";
            aig = mockturtle::balancing(aig, {balfn}, bs);
            aig = mockturtle::cleanup_dangling(aig);
            
            return aig;
        }
    };
}