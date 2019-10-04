#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
  ALICE_COMMAND( find_xor, "Testing", "Convert AIG to XAG and find XOR gates"){
    auto ntk = *store<aig_ntk>().current();
    // mockturtle::write_dot(ntk, "aig_test.dot");
    mockturtle::direct_resynthesis<mockturtle::xag_network> resyn_xag;
    auto xag = mockturtle::node_resynthesis<mockturtle::xag_network>(ntk, resyn_xag);
    std::cout << "size = " << xag.num_gates() << "\n";
    xag.foreach_node( [&]( auto node ) {
      std::cout << "Node = " << node << "\n";
      if(xag.is_xor(node)){
        std::cout << "XOR\n";
      }
    });
    mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;
    mockturtle::cut_rewriting(xag, resyn);
    xag = mockturtle::cleanup_dangling(xag);
    std::cout << "size = " << xag.num_gates() << "\n";
    xag.foreach_node( [&]( auto node ) {
      std::cout << "Node = " << node << "\n";
      if(xag.is_xor(node)){
        std::cout << "XOR\n";
      }
    });
    mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;
    auto aig2 = mockturtle::node_resynthesis<mockturtle::aig_network>(xag, resyn_aig);
    // mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_to_aig;
    // mockturtle::cut_rewriting(aig2, resyn_to_aig);
    std::cout << "size = " << aig2.num_gates() << "\n";
    aig2.foreach_node( [&]( auto node ) {
      std::cout << "Node = " << node << "\n";
      if(aig2.is_xor(node)){
        std::cout << "XOR\n";
      }
    });
    // mockturtle::write_dot(xag, "xag_test.dot");
    // mockturtle::aig_network aig;
    // auto a = aig.create_pi();
    // auto b = aig.create_pi();
    // auto c = aig.create_pi();
    // auto d = aig.create_and(a, b);
    // auto e = aig.create_xor(c, d);
    // aig.create_po(e);

    // aig.foreach_node([&](auto node){
    //   std::cout << "Node = " << node << "\n";
    //   if(aig.is_xor(node)){
    //     std::cout << "XOR\n";
    //   }
    // });
  }
}