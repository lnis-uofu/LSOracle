/* kitty: C++ truth table library
 * Copyright (C) 2017-2018  EPFL
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

#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/static_truth_table.hpp>

using namespace kitty;

TEST( ConstructorsTest, create )
{
  dynamic_truth_table tt_d( 5 );
  EXPECT_EQ( tt_d, create<dynamic_truth_table>( 5 ) );

  static_truth_table<5> tt_s;
  EXPECT_EQ( tt_s, create<static_truth_table<5>>( 5 ) );
}

TEST( ConstructorsTest, create_nth_var5 )
{
  static_truth_table<5> tt_s;
  dynamic_truth_table tt_d( 5 );

  const auto mask = 0xffffffff;

  for ( auto i = 0; i <= 4; ++i )
  {
    create_nth_var( tt_s, i );
    EXPECT_EQ( tt_s._bits, detail::projections[i] & mask );
    create_nth_var( tt_d, i );
    EXPECT_EQ( tt_s._bits, tt_d._bits[0] );
  }
}

TEST( ConstructorsTest, create_from_binary_string )
{
  static_truth_table<3> tt_s, tt_s_str;
  create_majority( tt_s );
  create_from_binary_string( tt_s_str, "11101000" );

  dynamic_truth_table tt_d( 3 ), tt_d_str( 3 );
  create_majority( tt_d );
  create_from_binary_string( tt_d_str, "11101000" );

  EXPECT_TRUE( equal( tt_s, tt_s_str ) );
  EXPECT_TRUE( equal( tt_d, tt_d_str ) );
}

TEST( ConstructorsTest, create_from_hex_string )
{
  static_truth_table<9> tt_s, tt_s_str;
  create_majority( tt_s );
  create_from_hex_string( tt_s_str, "fffffffefffefee8fffefee8fee8e880fffefee8fee8e880fee8e880e8808000fffefee8fee8e880fee8e880e8808000fee8e880e8808000e880800080000000" );

  dynamic_truth_table tt_d( 9 ), tt_d_str( 9 );
  create_majority( tt_d );
  create_from_hex_string( tt_d_str, "FFFFFFFEFFFEFEE8FFFEFEE8FEE8E880FFFEFEE8FEE8E880FEE8E880E8808000FFFEFEE8FEE8E880FEE8E880E8808000FEE8E880E8808000E880800080000000" );

  EXPECT_TRUE( equal( tt_s, tt_s_str ) );
  EXPECT_TRUE( equal( tt_d, tt_d_str ) );
}

TEST( ConstructorsTest, create_constants )
{
  static_truth_table<0> tt_s;
  EXPECT_EQ( tt_s.num_vars(), 0 );
  EXPECT_EQ( tt_s.num_bits(), 1 );

  create_from_hex_string( tt_s, "0" );
  EXPECT_EQ( tt_s._bits, 0x0u );

  create_from_hex_string( tt_s, "1" );
  EXPECT_EQ( tt_s._bits, 0x1u );
}

TEST( ConstructorsTest, create_one_variable_functions )
{
  static_truth_table<1> tt_s;
  EXPECT_EQ( tt_s.num_vars(), 1 );
  EXPECT_EQ( tt_s.num_bits(), 2 );

  create_from_hex_string( tt_s, "0" );
  EXPECT_EQ( tt_s._bits, 0x0u );

  create_from_hex_string( tt_s, "1" );
  EXPECT_EQ( tt_s._bits, 0x1u );

  create_from_hex_string( tt_s, "2" );
  EXPECT_EQ( tt_s._bits, 0x2u );

  create_from_hex_string( tt_s, "3" );
  EXPECT_EQ( tt_s._bits, 0x3u );
}

TEST( ConstructorsTest, create_random )
{
  static_truth_table<5> tt_s5;
  static_truth_table<7> tt_s7;
  dynamic_truth_table tt_d5( 5 );
  dynamic_truth_table tt_d7( 7 );

  create_random( tt_s5 );
  create_random( tt_s7 );
  create_random( tt_d5 );
  create_random( tt_d7 );

  // std::cout << tt_s5._bits[0] << std::endl;

  EXPECT_TRUE( true ); /* a dummy test to enable the print out */
}

TEST( ConstructorsTest, create_from_words )
{
  std::vector<uint64_t> words_vec{UINT64_C( 0xfee8e880e8808000 ), UINT64_C( 0xfffefee8fee8e880 )};
  constexpr uint64_t words_arr[]{UINT64_C( 0xfee8e880e8808000 ), UINT64_C( 0xfffefee8fee8e880 )};

  static_truth_table<7> tt_v, tt_a, tt_s;

  create_from_words( tt_v, words_vec.begin(), words_vec.end() );
  create_from_words( tt_a, words_arr, words_arr + 2 );
  create_majority( tt_s );

  EXPECT_EQ( tt_v, tt_s );
  EXPECT_EQ( tt_a, tt_s );
}

TEST( ConstructorsTest, create_majority5 )
{
  static_truth_table<5> tt_s;
  create_majority( tt_s );

  EXPECT_EQ( tt_s._bits, 0xfee8e880 );

  dynamic_truth_table tt_d( 5 );
  create_majority( tt_d );

  EXPECT_EQ( tt_s._bits, tt_d._bits[0u] );
}

TEST( ConstructorsTest, create_majority7 )
{
  static_truth_table<7> tt_s;
  create_majority( tt_s );

  EXPECT_EQ( tt_s._bits[0u], UINT64_C( 0xfee8e880e8808000 ) );
  EXPECT_EQ( tt_s._bits[1u], UINT64_C( 0xfffefee8fee8e880 ) );

  dynamic_truth_table tt_d( 7 );
  create_majority( tt_d );

  EXPECT_TRUE( std::equal( std::begin( tt_d._bits ), std::end( tt_d._bits ), std::begin( tt_s._bits ), std::end( tt_s._bits ) ) );
}

TEST( ConstructorsTest, create_threshold7 )
{
  static_truth_table<9> t1, t2;

  create_threshold( t1, 4 );
  create_threshold( t2, 5 );

  const auto t3 = t1 & ~t2;

  EXPECT_EQ( count_ones( t3 ), uint64_t( 126 ) );

  for_each_one_bit( t3, []( auto index ) {
    EXPECT_EQ( __builtin_popcount( index ), 5 );
  } );
}

TEST( ConstructorsTest, create_equals )
{
  static_truth_table<9> total;
  uint64_t total_bits{0};

  for ( auto k = 0; k <= total.num_vars(); ++k )
  {
    auto tt = total.construct();
    create_equals( tt, k );
    total_bits += count_ones( tt );
    total |= tt;
  }

  EXPECT_TRUE( is_const0( ~total ) );
  EXPECT_EQ( total_bits, uint64_t( 1 ) << total.num_vars() );
}

TEST( ConstructorsTest, create_symmetric )
{
  static_truth_table<2> small;

  create_symmetric( small, 0 );
  EXPECT_EQ( small._bits, 0x0u );

  create_symmetric( small, 1 );
  EXPECT_EQ( small._bits, 0x1u );

  create_symmetric( small, 2 );
  EXPECT_EQ( small._bits, 0x6u );

  create_symmetric( small, 3 );
  EXPECT_EQ( small._bits, 0x7u );

  create_symmetric( small, 4 );
  EXPECT_EQ( small._bits, 0x8u );

  create_symmetric( small, 5 );
  EXPECT_EQ( small._bits, 0x9u );

  create_symmetric( small, 6 );
  EXPECT_EQ( small._bits, 0xeu );

  create_symmetric( small, 7 );
  EXPECT_EQ( small._bits, 0xfu );
}

TEST( ConstructorsTest, create_from_cubes )
{
  std::vector<cube> cubes;
  cubes.emplace_back( "11--" );
  cubes.emplace_back( "00--" );
  cubes.emplace_back( "--00" );
  cubes.emplace_back( "--11" );

  kitty::dynamic_truth_table tt( 4u );
  kitty::create_random( tt ); /* fill with noise */
  kitty::create_from_cubes( tt, cubes, true );

  EXPECT_EQ( tt._bits[0u], 0x6996u );
}

TEST( ConstructorsTest, create_from_chain_success )
{
  static_truth_table<3> maj, maj_v1, maj_v2, maj_v3, maj_s;

  create_majority( maj );

  EXPECT_TRUE( create_from_chain( maj_v1, {"x4 = x1 & x2", "x5 = x1 & x3", "x6 = x2 & x3", "x7 = x4 | x5", "x8 = x6 | x7"} ) );
  EXPECT_TRUE( create_from_chain( maj_v2, {"x4=x1&x2", "x5=x1&x3", "x6=x2&x3", "x7=x4|x5", "x8=x6|x7"} ) );
  EXPECT_TRUE( create_from_chain( maj_v3, {"x4=  x1 & x2", "x5=  x1 &   x3", "x6= x2   &  x3", "x7   = x4 |   x5", "  x8 = x6 | x7  "} ) );

  std::string prog = "x4 = x1 & x2\nx5 = x1 & x3\n\nx6 = x2 & x3\nx7 = x4 | x5\nx8 = x6 | x7";
  std::istringstream in( prog );
  EXPECT_TRUE( create_from_chain( maj_s, in ) );

  EXPECT_EQ( maj, maj_v1 );
  EXPECT_EQ( maj, maj_v2 );
  EXPECT_EQ( maj, maj_v3 );
  EXPECT_EQ( maj, maj_s );

  static_truth_table<2> xor_o, xor_v1, xor_v2, xor_v3;

  create_from_binary_string( xor_o, "0110" );
  EXPECT_TRUE( create_from_chain( xor_v1, {"x3 = x1 ^ x2"} ) );
  EXPECT_TRUE( create_from_chain( xor_v2, {"x3 = x1 !& x2", "x4 = x1 !& x3", "x5 = x2 !& x3", "x6 = x4 !& x5"} ) );
  EXPECT_TRUE( create_from_chain( xor_v3, {"x3 = x1 < x2", "x4 = x1 > x2", "x5 = x3 | x4"} ) );

  EXPECT_EQ( xor_o, xor_v1 );
  EXPECT_EQ( xor_o, xor_v2 );
  EXPECT_EQ( xor_o, xor_v3 );
}

TEST( ConstructorsTest, create_from_chain_fail )
{
  auto check_for = []( const std::string& chain, const std::string& msg ) {
    static_truth_table<3> tt;
    std::istringstream in( chain );
    std::string error;
    EXPECT_FALSE( create_from_chain( tt, in, &error ) );
    EXPECT_EQ( error, msg );
  };

  check_for( "y4 = x1 & x2", "error in \"y4 = x1 & x2\": variables must be prefixed with x" );
  check_for( "x3 = x1 & x2", "error in \"x3 = x1 & x2\": steps are not in order" );
  check_for( "x4", "error in \"x4\": no equal sign found" );
  check_for( "x4 = ", "error in \"\": line uncompleted" );
  check_for( "x4 = y1 & x2", "error in \"y1 & x2\": variables must be prefixed with x" );
  check_for( "x4 = x4 & x2", "error in \"x4 & x2\": invalid operand index" );
  check_for( "x4 = x3", "error in \"\": line uncompleted" );
  check_for( "x4 = x1 & y2", "error in \"& y2\": variables must be prefixed with x" );
  check_for( "x4 = x1 & x4", "error in \"x4\": invalid operand index" );
  check_for( "x4 = x1 @ x2", "error in \"@\": invalid operator" );
}

TEST( ConstructorsTst, create_multiple_from_chain )
{
  std::vector<std::string> steps{
    "x5 = x1 | x3",
    "x6 = x1 < x4",
    "x7 = x2 > x4",
    "x8 = x2 < x4",
    "x9 = x5 | x7",
    "x10 = x3 & x8",
    "x11 = x9 ^ x10",
    "x12 = x6 ^ x9"
  };

  static_truth_table<4> f1, f2;
  std::vector<static_truth_table<4>> fs;

  create_multiple_from_chain( 4, fs, steps );

  create_from_hex_string( f1, "cafe" );
  create_from_hex_string( f2, "affe" );

  EXPECT_EQ( fs[10], f1 );
  EXPECT_EQ( fs[11], f2 );
}
