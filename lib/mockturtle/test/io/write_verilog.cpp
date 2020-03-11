#include <catch.hpp>

#include <algorithm>
#include <sstream>
#include <vector>

#include <mockturtle/generators/arithmetic.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/networks/mig.hpp>

using namespace mockturtle;

TEST_CASE( "write single-gate AIG into Verilog file", "[write_verilog]" )
{
  aig_network aig;

  const auto a = aig.create_pi();
  const auto b = aig.create_pi();

  const auto f1 = aig.create_or( a, b );
  aig.create_po( f1 );

  std::ostringstream out;
  write_verilog( aig, out );

  CHECK( out.str() == "module top( pi0 , pi1 , po0 );\n"
                      "  input pi0 , pi1 ;\n"
                      "  output po0 ;\n"
                      "  wire new_n3 ;\n"
                      "  assign new_n3 = ~pi0 & ~pi1 ;\n"
                      "  assign po0 = ~new_n3 ;\n"
                      "endmodule\n" );
}

TEST_CASE( "write AIG for XOR into Verilog file", "[write_verilog]" )
{
  aig_network aig;

  const auto a = aig.create_pi();
  const auto b = aig.create_pi();

  const auto f1 = aig.create_nand( a, b );
  const auto f2 = aig.create_nand( a, f1 );
  const auto f3 = aig.create_nand( b, f1 );
  const auto f4 = aig.create_nand( f2, f3 );
  aig.create_po( f4 );

  std::ostringstream out;
  write_verilog( aig, out );

  CHECK( out.str() == "module top( pi0 , pi1 , po0 );\n"
                      "  input pi0 , pi1 ;\n"
                      "  output po0 ;\n"
                      "  wire new_n3 , new_n4 , new_n5 , new_n6 ;\n"
                      "  assign new_n3 = pi0 & pi1 ;\n"
                      "  assign new_n4 = pi0 & ~new_n3 ;\n"
                      "  assign new_n5 = pi1 & ~new_n3 ;\n"
                      "  assign new_n6 = ~new_n4 & ~new_n5 ;\n"
                      "  assign po0 = ~new_n6 ;\n"
                      "endmodule\n" );
}

TEST_CASE( "write MIG into Verilog file", "[write_verilog]" )
{
  mig_network aig;

  const auto a = aig.create_pi();
  const auto b = aig.create_pi();
  const auto c = aig.create_pi();

  const auto f1 = aig.create_and( a, b );
  const auto f2 = aig.create_or( a, b );
  const auto f3 = aig.create_maj( f1, f2, c );
  aig.create_po( f3 );

  std::ostringstream out;
  write_verilog( aig, out );

  CHECK( out.str() == "module top( pi0 , pi1 , pi2 , po0 );\n"
                      "  input pi0 , pi1 , pi2 ;\n"
                      "  output po0 ;\n"
                      "  wire new_n4 , new_n5 , new_n6 ;\n"
                      "  assign new_n4 = pi0 & pi1 ;\n"
                      "  assign new_n5 = pi0 | pi1 ;\n"
                      "  assign new_n6 = ( pi2 & new_n4 ) | ( pi2 & new_n5 ) | ( new_n4 & new_n5 ) ;\n"
                      "  assign po0 = new_n6 ;\n"
                      "endmodule\n" );
}

TEST_CASE( "write Verilog with register names", "[write_verilog]" )
{
  mig_network mig;

  std::vector<mig_network::signal> as( 3u );
  std::vector<mig_network::signal> bs( 3u );
  std::generate( as.begin(), as.end(), [&]() { return mig.create_pi(); } );
  std::generate( bs.begin(), bs.end(), [&]() { return mig.create_pi(); } );
  auto carry = mig.get_constant( false );
  carry_ripple_adder_inplace( mig, as, bs, carry );
  as.push_back( carry );
  std::for_each( as.begin(), as.end(), [&]( auto const& f ) { mig.create_po( f ); } );

  std::ostringstream out;
  write_verilog_params ps;
  ps.input_names = {{"a", 3u}, {"b", 3u}};
  ps.output_names = {{"y", 4u}};
  write_verilog( mig, out, ps );

  CHECK( out.str() == "module top( a , b , y );\n"
                      "  input [2:0] a ;\n"
                      "  input [2:0] b ;\n"
                      "  output [3:0] y ;\n"
                      "  wire new_n7 , new_n8 , new_n9 , new_n10 , new_n11 , new_n12 , new_n13 , new_n14 , new_n15 , new_n16 , new_n17 , new_n18 ;\n"
                      "  assign new_n8 = a[0] & ~b[0] ;\n"
                      "  assign new_n9 = a[0] | b[0] ;\n"
                      "  assign new_n10 = ( ~a[0] & new_n8 ) | ( ~a[0] & new_n9 ) | ( new_n8 & new_n9 ) ;\n"
                      "  assign new_n7 = a[0] & b[0] ;\n"
                      "  assign new_n12 = ( a[1] & ~b[1] ) | ( a[1] & new_n7 ) | ( ~b[1] & new_n7 ) ;\n"
                      "  assign new_n13 = ( a[1] & b[1] ) | ( a[1] & ~new_n7 ) | ( b[1] & ~new_n7 ) ;\n"
                      "  assign new_n14 = ( ~a[1] & new_n12 ) | ( ~a[1] & new_n13 ) | ( new_n12 & new_n13 ) ;\n"
                      "  assign new_n11 = ( a[1] & b[1] ) | ( a[1] & new_n7 ) | ( b[1] & new_n7 ) ;\n"
                      "  assign new_n16 = ( a[2] & ~b[2] ) | ( a[2] & new_n11 ) | ( ~b[2] & new_n11 ) ;\n"
                      "  assign new_n17 = ( a[2] & b[2] ) | ( a[2] & ~new_n11 ) | ( b[2] & ~new_n11 ) ;\n"
                      "  assign new_n18 = ( ~a[2] & new_n16 ) | ( ~a[2] & new_n17 ) | ( new_n16 & new_n17 ) ;\n"
                      "  assign new_n15 = ( a[2] & b[2] ) | ( a[2] & new_n11 ) | ( b[2] & new_n11 ) ;\n"
                      "  assign y[0] = new_n10 ;\n"
                      "  assign y[1] = new_n14 ;\n"
                      "  assign y[2] = new_n18 ;\n"
                      "  assign y[3] = new_n15 ;\n"
                      "endmodule\n" );
}
