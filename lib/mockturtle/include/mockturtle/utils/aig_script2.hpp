#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace mockturtle{
    class aig_script2{
    public:
        mockturtle::aig_network run(mockturtle::aig_network& aig){

            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::akers_resynthesis<mockturtle::aig_network> rf_resyn;
            mockturtle::cut_rewriting_params ps;
            mockturtle::refactoring_params rp;

            ps.cut_enumeration_ps.cut_size = 4;
            // rp.allow_zero_gain = false;

            //rw
            mockturtle::cut_rewriting(aig, resyn, ps);
            // std::cout << "done cut rewriting\n";
            aig = mockturtle::cleanup_dangling(aig);
            // std::cout << "2\n";

            //rf
            mockturtle::refactoring(aig, rf_resyn, rp);
            aig = mockturtle::cleanup_dangling(aig);
            // std::cout << "3\n";

            //rw
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);
            // std::cout << "5\n";

            // ps.allow_zero_gain = true;

            //rwz
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);
            // std::cout << "6\n";

            //rfz
            // rp.allow_zero_gain = true;
            // std::cout << "activate zero gain\n";
            mockturtle::refactoring(aig, rf_resyn, rp);
            // std::cout << "refactored\n";
            aig = mockturtle::cleanup_dangling(aig);
            // std::cout << "8\n";

            //rwz
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);
            // std::cout << "9\n";

            return aig;
        }
    };
}