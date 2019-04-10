#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace mockturtle{
    class aig_script{
    public:
        mockturtle::aig_network run(mockturtle::aig_network& aig){

            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
            mockturtle::cut_rewriting_params ps;
            ps.cut_enumeration_ps.cut_size = 4;

            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "2nd round area recovering " << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "2nd round depth optimization" << std::endl;

            //DEPTH REWRITING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "3rd round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "4th round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "3rd round depth optimization" << std::endl;

            //DEPTH REWRITING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "5th round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "6th round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "Final depth optimization" << std::endl;

            //DEPTH REWRITING
            mockturtle::cut_rewriting(aig, resyn, ps);
            aig = mockturtle::cleanup_dangling(aig);

            // std::cout << "Majority nodes " << mig.num_gates() << " MIG depth " << mig_depth3.depth() << std::endl;

            return aig;
        }
    };
}