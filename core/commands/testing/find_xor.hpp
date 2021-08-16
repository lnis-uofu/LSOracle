/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

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
    env->out() << "size = " << xag.num_gates() << "\n";
    xag.foreach_node( [&]( auto node ) {
      env->out() << "Node = " << node << "\n";
      if(xag.is_xor(node)){
        env->out() << "XOR\n";
      }
    });
    mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;
    mockturtle::cut_rewriting(xag, resyn);
    xag = mockturtle::cleanup_dangling(xag);
    env->out() << "size = " << xag.num_gates() << "\n";
    xag.foreach_node( [&]( auto node ) {
      env->out() << "Node = " << node << "\n";
      if(xag.is_xor(node)){
        env->out() << "XOR\n";
      }
    });
    mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;
    auto aig2 = mockturtle::node_resynthesis<mockturtle::aig_network>(xag, resyn_aig);
    // mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_to_aig;
    // mockturtle::cut_rewriting(aig2, resyn_to_aig);
    env->out() << "size = " << aig2.num_gates() << "\n";
    aig2.foreach_node( [&]( auto node ) {
      env->out() << "Node = " << node << "\n";
      if(aig2.is_xor(node)){
        env->out() << "XOR\n";
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
    //   env->out() << "Node = " << node << "\n";
    //   if(aig.is_xor(node)){
    //     env->out() << "XOR\n";
    //   }
    // });
  }
}
