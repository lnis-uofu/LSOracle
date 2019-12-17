#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>

namespace mockturtle{
    class xmg_script{
    public:
        mockturtle::xmg_network run(mockturtle::xmg_network& xmg){
            mockturtle::depth_view xmg_depth{xmg};

            // std::cout << "1st round depth optimization " << std::endl;

            mockturtle::xmg_algebraic_depth_rewriting(xmg_depth);

            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "1st round area recovering " << std::endl;

            // AREA RECOVERING
            mockturtle::xmg_npn_resynthesis resyn;
            mockturtle::cut_rewriting_params ps;

            ps.cut_enumeration_ps.cut_size = 4;

            mockturtle::cut_rewriting(xmg, resyn, ps);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "2nd round area recovering " << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(xmg, resyn, ps);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "2nd round depth optimization" << std::endl;

            //DEPTH REWRITING
            mockturtle::depth_view xmg_depth1{xmg};

            mockturtle::xmg_algebraic_depth_rewriting(xmg_depth1);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "3rd round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(xmg, resyn, ps);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "4th round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(xmg, resyn, ps);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "3rd round depth optimization" << std::endl;

            //DEPTH REWRITING
            mockturtle::depth_view xmg_depth2{xmg};

            mockturtle::xmg_algebraic_depth_rewriting(xmg_depth2);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "5th round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(xmg, resyn, ps);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "6th round area recovering" << std::endl;

            // AREA RECOVERING
            mockturtle::cut_rewriting(xmg, resyn, ps);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "Final depth optimization" << std::endl;

            //DEPTH REWRITING
            mockturtle::depth_view xmg_depth3{xmg};

            // std::cout << "Network Optimized" << std::endl;

            mockturtle::xmg_algebraic_depth_rewriting(xmg_depth3);
            xmg = mockturtle::cleanup_dangling( xmg );

            // std::cout << "Majority nodes " << xmg.num_gates() << " xmg depth " << xmg_depth3.depth() << std::endl;

            return xmg;
        }
    };
}