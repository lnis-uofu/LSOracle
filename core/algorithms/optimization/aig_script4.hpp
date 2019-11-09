#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace oracle{
    class aig_script4{
    public:
        mockturtle::aig_network run(mockturtle::aig_network& aig){
            
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::exact_aig_resynthesis<mockturtle::aig_network> fallback; // fallback
            mockturtle::dsd_resynthesis<mockturtle::aig_network, decltype( fallback )> rf_resyn( fallback );
            mockturtle::cut_rewriting_params ps;
            mockturtle::refactoring_params rp;

            ps.cut_enumeration_ps.cut_size = 4;
            rp.allow_zero_gain = false;
            
            //b
            std::cout << "b\n";
            mockturtle::depth_view aig_depth{aig};
            mockturtle::balancing(aig_depth);
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
            mockturtle::depth_view aig_depth1{aig};
            mockturtle::balancing(aig_depth1);
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
            mockturtle::depth_view aig_depth2{aig};
            mockturtle::balancing(aig_depth2);
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
            mockturtle::depth_view aig_depth3{aig};
            mockturtle::balancing(aig_depth3);
            aig = mockturtle::cleanup_dangling(aig);
            
            return aig;
        }
    };
}