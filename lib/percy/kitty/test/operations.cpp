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

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class OperationsTest : public kitty::testing::Test
{
};

TEST_F( OperationsTest, unary_for_small )
{
  EXPECT_EQ( unary_not( from_hex<2>( "8" ) ), from_hex<2>( "7" ) );
  EXPECT_EQ( unary_not( from_hex<2>( "6" ) ), from_hex<2>( "9" ) );

  EXPECT_EQ( unary_not_if( from_hex<2>( "8" ), true ), from_hex<2>( "7" ) );
  EXPECT_EQ( unary_not_if( from_hex<2>( "6" ), true ), from_hex<2>( "9" ) );

  EXPECT_EQ( unary_not_if( from_hex<2>( "8" ), false ), from_hex<2>( "8" ) );
  EXPECT_EQ( unary_not_if( from_hex<2>( "6" ), false ), from_hex<2>( "6" ) );
}

TEST_F( OperationsTest, binary_for_small )
{
  EXPECT_EQ( binary_and( nth<2>( 0 ), nth<2>( 1 ) ), from_hex<2>( "8" ) );
  EXPECT_EQ( binary_or( nth<2>( 0 ), nth<2>( 1 ) ), from_hex<2>( "e" ) );
  EXPECT_EQ( unary_not( binary_and( nth<2>( 0 ), nth<2>( 1 ) ) ), from_hex<2>( "7" ) );
  EXPECT_EQ( unary_not( binary_or( nth<2>( 0 ), nth<2>( 1 ) ) ), from_hex<2>( "1" ) );

  EXPECT_EQ( binary_and( nth( 2, 0 ), nth( 2, 1 ) ), from_hex( 2, "8" ) );
  EXPECT_EQ( binary_or( nth( 2, 0 ), nth( 2, 1 ) ), from_hex( 2, "e" ) );
  EXPECT_EQ( unary_not( binary_and( nth( 2, 0 ), nth( 2, 1 ) ) ), from_hex( 2, "7" ) );
  EXPECT_EQ( unary_not( binary_or( nth( 2, 0 ), nth( 2, 1 ) ) ), from_hex( 2, "1" ) );
}

TEST_F( OperationsTest, ternary_for_small )
{
  {
    const auto t1 = nth<3>( 0 ), t2 = nth<3>( 1 ), t3 = nth<3>( 2 );

    const auto maj_expr = binary_or( binary_or( binary_and( t1, t2 ), binary_and( t1, t3 ) ), binary_and( t2, t3 ) );
    const auto maj_direct = ternary_majority( t1, t2, t3 );

    EXPECT_EQ( maj_expr, from_hex<3>( "e8" ) );
    EXPECT_EQ( maj_expr, maj_direct );

    const auto ite_expr = binary_or( binary_and( t1, t2 ), binary_and( unary_not( t1 ), t3 ) );
    const auto ite_direct = ternary_ite( t1, t2, t3 );

    EXPECT_EQ( ite_expr, from_hex<3>( "d8" ) );
    EXPECT_EQ( ite_expr, ite_direct );
  }

  {
    const auto t1 = nth( 3, 0 ), t2 = nth( 3, 1 ), t3 = nth( 3, 2 );

    const auto maj_expr = binary_or( binary_or( binary_and( t1, t2 ), binary_and( t1, t3 ) ), binary_and( t2, t3 ) );
    const auto maj_direct = ternary_majority( t1, t2, t3 );

    EXPECT_EQ( maj_expr, from_hex( 3, "e8" ) );
    EXPECT_EQ( maj_expr, maj_direct );

    const auto ite_expr = binary_or( binary_and( t1, t2 ), binary_and( unary_not( t1 ), t3 ) );
    const auto ite_direct = ternary_ite( t1, t2, t3 );

    EXPECT_EQ( ite_expr, from_hex( 3, "d8" ) );
    EXPECT_EQ( ite_expr, ite_direct );
  }
}

TEST_F( OperationsTest, unary_for_large )
{
  EXPECT_EQ( unary_not( from_hex<7>( "80000000000000000000000000000000" ) ), from_hex<7>( "7fffffffffffffffffffffffffffffff" ) );
  EXPECT_EQ( unary_not( from_hex<7>( "66666666666666666666666666666666" ) ), from_hex<7>( "99999999999999999999999999999999" ) );

  EXPECT_EQ( unary_not_if( from_hex<7>( "80000000000000000000000000000000" ), true ), from_hex<7>( "7fffffffffffffffffffffffffffffff" ) );
  EXPECT_EQ( unary_not_if( from_hex<7>( "66666666666666666666666666666666" ), true ), from_hex<7>( "99999999999999999999999999999999" ) );

  EXPECT_EQ( unary_not_if( from_hex<7>( "80000000000000000000000000000000" ), false ), from_hex<7>( "80000000000000000000000000000000" ) );
  EXPECT_EQ( unary_not_if( from_hex<7>( "66666666666666666666666666666666" ), false ), from_hex<7>( "66666666666666666666666666666666" ) );
}

TEST_F( OperationsTest, binary_for_large )
{
  {
    auto tt = nth<7>( 0 );
    for ( auto i = 1; i < 7; ++i )
    {
      tt = binary_and( tt, nth<7>( i ) );
    }
    EXPECT_EQ( tt, from_hex<7>( "80000000000000000000000000000000" ) );
  }

  {
    auto tt = nth( 7, 0 );
    for ( auto i = 1; i < 7; ++i )
    {
      tt = binary_and( tt, nth( 7, i ) );
    }
    EXPECT_EQ( tt, from_hex( 7, "80000000000000000000000000000000" ) );
  }
}

TEST_F( OperationsTest, ternary_for_large )
{
  {
    const auto t1 = nth<7>( 0 ), t2 = nth<7>( 1 ), t3 = nth<7>( 2 );

    const auto maj_expr = binary_or( binary_or( binary_and( t1, t2 ), binary_and( t1, t3 ) ), binary_and( t2, t3 ) );
    const auto maj_direct = ternary_majority( t1, t2, t3 );

    EXPECT_EQ( maj_expr, maj_direct );

    const auto ite_expr = binary_or( binary_and( t1, t2 ), binary_and( unary_not( t1 ), t3 ) );
    const auto ite_direct = ternary_ite( t1, t2, t3 );

    EXPECT_EQ( ite_expr, ite_direct );
  }

  {
    const auto t1 = nth( 7, 0 ), t2 = nth( 7, 1 ), t3 = nth( 7, 2 );

    const auto maj_expr = binary_or( binary_or( binary_and( t1, t2 ), binary_and( t1, t3 ) ), binary_and( t2, t3 ) );
    const auto maj_direct = ternary_majority( t1, t2, t3 );

    EXPECT_EQ( maj_expr, maj_direct );

    const auto ite_expr = binary_or( binary_and( t1, t2 ), binary_and( unary_not( t1 ), t3 ) );
    const auto ite_direct = ternary_ite( t1, t2, t3 );

    EXPECT_EQ( ite_expr, ite_direct );
  }
}

TEST_F( OperationsTest, comparisons )
{
  EXPECT_TRUE( equal( from_hex<3>( "e8" ), from_hex<3>( "e8" ) ) );
  EXPECT_TRUE( equal( from_hex( 3, "e8" ), from_hex( 3, "e8" ) ) );

  EXPECT_TRUE( less_than( from_hex<3>( "e5" ), from_hex<3>( "f6" ) ) );
  EXPECT_TRUE( less_than( from_hex<3>( "e5" ), from_hex<3>( "f5" ) ) );
  EXPECT_TRUE( less_than( from_hex<3>( "e5" ), from_hex<3>( "f4" ) ) );
  EXPECT_TRUE( less_than( from_hex<3>( "e5" ), from_hex<3>( "e6" ) ) );
  EXPECT_FALSE( less_than( from_hex<3>( "e5" ), from_hex<3>( "e5" ) ) );
  EXPECT_FALSE( less_than( from_hex<3>( "e5" ), from_hex<3>( "e4" ) ) );
  EXPECT_FALSE( less_than( from_hex<3>( "e5" ), from_hex<3>( "d6" ) ) );
  EXPECT_FALSE( less_than( from_hex<3>( "e5" ), from_hex<3>( "d5" ) ) );
  EXPECT_FALSE( less_than( from_hex<3>( "e5" ), from_hex<3>( "d4" ) ) );

  EXPECT_TRUE( less_than( from_hex( 3, "e5" ), from_hex( 3, "f6" ) ) );
  EXPECT_TRUE( less_than( from_hex( 3, "e5" ), from_hex( 3, "f5" ) ) );
  EXPECT_TRUE( less_than( from_hex( 3, "e5" ), from_hex( 3, "f4" ) ) );
  EXPECT_TRUE( less_than( from_hex( 3, "e5" ), from_hex( 3, "e6" ) ) );
  EXPECT_FALSE( less_than( from_hex( 3, "e5" ), from_hex( 3, "e5" ) ) );
  EXPECT_FALSE( less_than( from_hex( 3, "e5" ), from_hex( 3, "e4" ) ) );
  EXPECT_FALSE( less_than( from_hex( 3, "e5" ), from_hex( 3, "d6" ) ) );
  EXPECT_FALSE( less_than( from_hex( 3, "e5" ), from_hex( 3, "d5" ) ) );
  EXPECT_FALSE( less_than( from_hex( 3, "e5" ), from_hex( 3, "d4" ) ) );

  EXPECT_TRUE( equal( from_hex<7>( "e92c774439c72c8955906ef92ecefec9" ), from_hex<7>( "e92c774439c72c8955906ef92ecefec9" ) ) );
  EXPECT_TRUE( equal( from_hex( 7, "e92c774439c72c8955906ef92ecefec9" ), from_hex( 7, "e92c774439c72c8955906ef92ecefec9" ) ) );

  EXPECT_TRUE( less_than( from_hex<7>( "e92c774439c72c8955906ef92ecefec9" ), from_hex<7>( "e92c774439c72c8955906ef92edefec9" ) ) );
  EXPECT_FALSE( less_than( from_hex<7>( "e92c774439c72c8955906ef92ecefec9" ), from_hex<7>( "e92c774439c72c8955906ef92ebefec9" ) ) );

  EXPECT_TRUE( less_than( from_hex( 7, "e92c774439c72c8955906ef92ecefec9" ), from_hex( 7, "e92c774439c72c8955906ef92edefec9" ) ) );
  EXPECT_FALSE( less_than( from_hex( 7, "e92c774439c72c8955906ef92ecefec9" ), from_hex( 7, "e92c774439c72c8955906ef92ebefec9" ) ) );
}

TEST_F( OperationsTest, support )
{
  EXPECT_TRUE( is_const0( from_hex<0>( "0" ) ) );
  EXPECT_TRUE( is_const0( from_hex<1>( "0" ) ) );
  EXPECT_TRUE( is_const0( from_hex<2>( "0" ) ) );
  EXPECT_TRUE( is_const0( from_hex<3>( "00" ) ) );
  EXPECT_TRUE( is_const0( from_hex<4>( "0000" ) ) );
  EXPECT_TRUE( is_const0( from_hex<5>( "00000000" ) ) );
  EXPECT_TRUE( is_const0( from_hex<6>( "0000000000000000" ) ) );
  EXPECT_TRUE( is_const0( from_hex<7>( "00000000000000000000000000000000" ) ) );
  EXPECT_TRUE( is_const0( from_hex<8>( "0000000000000000000000000000000000000000000000000000000000000000" ) ) );

  EXPECT_TRUE( has_var( from_hex<3>( "77" ), 0 ) );
  EXPECT_TRUE( has_var( from_hex<3>( "77" ), 1 ) );
  EXPECT_TRUE( !has_var( from_hex<3>( "77" ), 2 ) );

  EXPECT_TRUE( has_var( from_hex( 3, "77" ), 0 ) );
  EXPECT_TRUE( has_var( from_hex( 3, "77" ), 1 ) );
  EXPECT_TRUE( !has_var( from_hex( 3, "77" ), 2 ) );

  for ( auto i = 0; i < 8; ++i )
  {
    EXPECT_EQ( has_var( from_hex<8>( "3333333333cc33cc3333333333cc33cc33cc33cccccccccc33cc33cccccccccc" ), i ), i % 2 == 1 );
  }

  for ( auto i = 0; i < 8; ++i )
  {
    EXPECT_EQ( has_var( from_hex( 8, "3333333333cc33cc3333333333cc33cc33cc33cccccccccc33cc33cccccccccc" ), i ), i % 2 == 1 );
  }
}

TEST_F( OperationsTest, next )
{
  EXPECT_EQ( next( from_hex<3>( "00" ) ), from_hex<3>( "01" ) );
  EXPECT_EQ( next( from_hex<3>( "ab" ) ), from_hex<3>( "ac" ) );
  EXPECT_EQ( next( from_hex<3>( "ff" ) ), from_hex<3>( "00" ) );

  EXPECT_EQ( next( from_hex( 3, "00" ) ), from_hex( 3, "01" ) );
  EXPECT_EQ( next( from_hex( 3, "ab" ) ), from_hex( 3, "ac" ) );
  EXPECT_EQ( next( from_hex( 3, "ff" ) ), from_hex( 3, "00" ) );

  EXPECT_EQ( next( from_hex<7>( "00000000000000000000000000000000" ) ), from_hex<7>( "00000000000000000000000000000001" ) );
  EXPECT_EQ( next( from_hex<7>( "aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb" ) ), from_hex<7>( "aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbc" ) );
  EXPECT_EQ( next( from_hex<7>( "ffffffffffffffffffffffffffffffff" ) ), from_hex<7>( "00000000000000000000000000000000" ) );

  EXPECT_EQ( next( from_hex( 7, "00000000000000000000000000000000" ) ), from_hex( 7, "00000000000000000000000000000001" ) );
  EXPECT_EQ( next( from_hex( 7, "aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb" ) ), from_hex( 7, "aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbc" ) );
  EXPECT_EQ( next( from_hex( 7, "ffffffffffffffffffffffffffffffff" ) ), from_hex( 7, "00000000000000000000000000000000" ) );
}

TEST_F( OperationsTest, cofactors )
{
  EXPECT_EQ( cofactor0( from_hex<3>( "e8" ), 2 ), from_hex<3>( "88" ) );
  EXPECT_EQ( cofactor0( from_hex<3>( "e8" ), 1 ), from_hex<3>( "a0" ) );
  EXPECT_EQ( cofactor0( from_hex<3>( "e8" ), 0 ), from_hex<3>( "c0" ) );

  EXPECT_EQ( cofactor0( from_hex( 3, "e8" ), 2 ), from_hex( 3, "88" ) );
  EXPECT_EQ( cofactor0( from_hex( 3, "e8" ), 1 ), from_hex( 3, "a0" ) );
  EXPECT_EQ( cofactor0( from_hex( 3, "e8" ), 0 ), from_hex( 3, "c0" ) );

  // Co-factors of [(<abc>d){<efg>h}]
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 0 ), from_hex<8>( "3fff3fff3fff3fff3fff3fff3fff3fff3fff3fff3fffc0003fffc000c000c000" ) );
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 1 ), from_hex<8>( "5fff5fff5fff5fff5fff5fff5fff5fff5fff5fff5fffa0005fffa000a000a000" ) );
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 2 ), from_hex<8>( "77ff77ff77ff77ff77ff77ff77ff77ff77ff77ff77ff880077ff880088008800" ) );
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 3 ), from_hex<8>( "ffffffffffffffffffffffffffffffffffffffffffff0000ffff000000000000" ) );
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 4 ), from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe800e800e800e800e800e800" ) );
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 5 ), from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800e800e800" ) );
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 6 ), from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ffe800e800e80017ffe800e800e800" ) );
  EXPECT_EQ( cofactor0( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 7 ), from_hex<8>( "17ff17ff17ffe80017ffe800e800e80017ff17ff17ffe80017ffe800e800e800" ) );

  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 0 ), from_hex( 8, "3fff3fff3fff3fff3fff3fff3fff3fff3fff3fff3fffc0003fffc000c000c000" ) );
  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 1 ), from_hex( 8, "5fff5fff5fff5fff5fff5fff5fff5fff5fff5fff5fffa0005fffa000a000a000" ) );
  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 2 ), from_hex( 8, "77ff77ff77ff77ff77ff77ff77ff77ff77ff77ff77ff880077ff880088008800" ) );
  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 3 ), from_hex( 8, "ffffffffffffffffffffffffffffffffffffffffffff0000ffff000000000000" ) );
  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 4 ), from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe800e800e800e800e800e800" ) );
  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 5 ), from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800e800e800" ) );
  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 6 ), from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ffe800e800e80017ffe800e800e800" ) );
  EXPECT_EQ( cofactor0( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 7 ), from_hex( 8, "17ff17ff17ffe80017ffe800e800e80017ff17ff17ffe80017ffe800e800e800" ) );

  EXPECT_EQ( cofactor1( from_hex<3>( "e8" ), 2 ), from_hex<3>( "ee" ) );
  EXPECT_EQ( cofactor1( from_hex<3>( "e8" ), 1 ), from_hex<3>( "fa" ) );
  EXPECT_EQ( cofactor1( from_hex<3>( "e8" ), 0 ), from_hex<3>( "fc" ) );

  EXPECT_EQ( cofactor1( from_hex( 3, "e8" ), 2 ), from_hex( 3, "ee" ) );
  EXPECT_EQ( cofactor1( from_hex( 3, "e8" ), 1 ), from_hex( 3, "fa" ) );
  EXPECT_EQ( cofactor1( from_hex( 3, "e8" ), 0 ), from_hex( 3, "fc" ) );

  // Co-factors of [(<abc>d){<efg>h}]
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 0 ), from_hex<8>( "03ff03ff03ff03ff03ff03ff03ff03ff03ff03ff03fffc0003fffc00fc00fc00" ) );
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 1 ), from_hex<8>( "05ff05ff05ff05ff05ff05ff05ff05ff05ff05ff05fffa0005fffa00fa00fa00" ) );
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 2 ), from_hex<8>( "11ff11ff11ff11ff11ff11ff11ff11ff11ff11ff11ffee0011ffee00ee00ee00" ) );
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 3 ), from_hex<8>( "17171717171717171717171717171717171717171717e8e81717e8e8e8e8e8e8" ) );
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 4 ), from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe800e800" ) );
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 5 ), from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800" ) );
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 6 ), from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ff17ff17ffe800" ) );
  EXPECT_EQ( cofactor1( from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 7 ), from_hex<8>( "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff" ) );

  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 0 ), from_hex( 8, "03ff03ff03ff03ff03ff03ff03ff03ff03ff03ff03fffc0003fffc00fc00fc00" ) );
  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 1 ), from_hex( 8, "05ff05ff05ff05ff05ff05ff05ff05ff05ff05ff05fffa0005fffa00fa00fa00" ) );
  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 2 ), from_hex( 8, "11ff11ff11ff11ff11ff11ff11ff11ff11ff11ff11ffee0011ffee00ee00ee00" ) );
  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 3 ), from_hex( 8, "17171717171717171717171717171717171717171717e8e81717e8e8e8e8e8e8" ) );
  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 4 ), from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe800e800" ) );
  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 5 ), from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800" ) );
  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 6 ), from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ff17ff17ffe800" ) );
  EXPECT_EQ( cofactor1( from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ffe80017ffe800e800e800" ), 7 ), from_hex( 8, "17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff17ff" ) );
}

TEST_F( OperationsTest, swap_adjacent_inplace_small )
{
  for ( const auto& p : std::vector<std::pair<unsigned, std::string>>{{0u, "bce8"}, {1u, "e6e8"}, {2u, "dea8"}} )
  {
    auto tt_s = from_hex<4>( "dae8" );
    swap_adjacent_inplace( tt_s, p.first );
    EXPECT_EQ( tt_s, from_hex<4>( p.second ) );

    auto tt_d = from_hex( 4, "dae8" );
    swap_adjacent_inplace( tt_d, p.first );
    EXPECT_EQ( tt_d, from_hex( 4, p.second ) );
  }
}

TEST_F( OperationsTest, swap_adjacent_small )
{
  for ( const auto& p : std::vector<std::pair<unsigned, std::string>>{{0u, "bce8"}, {1u, "e6e8"}, {2u, "dea8"}} )
  {
    EXPECT_EQ( swap_adjacent( from_hex<4>( "dae8" ), p.first ), from_hex<4>( p.second ) );
    EXPECT_EQ( swap_adjacent( from_hex( 4, "dae8" ), p.first ), from_hex( 4, p.second ) );
  }
}

class OperationsTestSwap : public OperationsTest, public ::testing::WithParamInterface<std::pair<unsigned, std::string>>
{
};

TEST_P( OperationsTestSwap, swap_adjacent_inplace_large )
{
  auto tt_s = from_hex<9>( "28e3b8d62855c4b787eef391a93b33297856658b6743aa3cc7e11fde4e9cbf7c98b07f5febfff33bc7ad6f551bc4cbc440453e1bbe24f0cb4f268c6771b55eee" );
  swap_adjacent_inplace( tt_s, GetParam().first );
  EXPECT_EQ( tt_s, from_hex<9>( GetParam().second ) );

  auto tt_d = from_hex( 9, "28e3b8d62855c4b787eef391a93b33297856658b6743aa3cc7e11fde4e9cbf7c98b07f5febfff33bc7ad6f551bc4cbc440453e1bbe24f0cb4f268c6771b55eee" );
  swap_adjacent_inplace( tt_d, GetParam().first );
  EXPECT_EQ( tt_d, from_hex( 9, GetParam().second ) );
}

TEST_P( OperationsTestSwap, swap_adjacent_large )
{
  EXPECT_EQ( swap_adjacent( from_hex<9>( "28e3b8d62855c4b787eef391a93b33297856658b6743aa3cc7e11fde4e9cbf7c98b07f5febfff33bc7ad6f551bc4cbc440453e1bbe24f0cb4f268c6771b55eee" ),
                            GetParam().first ),
             from_hex<9>( GetParam().second ) );

  EXPECT_EQ( swap_adjacent( from_hex( 9, "28e3b8d62855c4b787eef391a93b33297856658b6743aa3cc7e11fde4e9cbf7c98b07f5febfff33bc7ad6f551bc4cbc440453e1bbe24f0cb4f268c6771b55eee" ),
                            GetParam().first ),
             from_hex( 9, GetParam().second ) );
}

INSTANTIATE_TEST_CASE_P( OperationsTestSwapInst,
                         OperationsTestSwap,
                         ::testing::Values( std::make_pair( 0u, "48e5d8b64833a2d787eef591c95d55497836638d6725cc5aa7e11fbe2e9adf7a"
                                                                "98d07f3fedfff55da7cb6f331da2ada220235e1dde42f0ad2f468a6771d33eee" ),
                                            std::make_pair( 1u, "28cbacd62855d09f93facf85a92f0f296c5659a35b43aa3cd3c937f672b4bf7c"
                                                                "a48c7f77ebffcf2fd3b97b5527d0e3d040513e27be18cce3731ab05b4d9d76fa" ),
                                            std::make_pair( 2u, "2e83bd862585cb478e7ef931a39b32397586685b6473a3acce711dfe49ecb7fc"
                                                                "9b8075ffefbff33bca7d65f51cb4ccb4440531ebb2e4fc0b42f686c77b155eee" ),
                                            std::make_pair( 3u, "28b8e3d628c455b787f3ee91a9333b297865568b67aa433cc71fe1de4ebf9c7c"
                                                                "987fb05febf3ff3bc76fad551bcbc4c4403e451bbef024cb4f8c2667715eb5ee" ),
                                            std::make_pair( 4u, "28e32855b8d6c4b787eea93bf391332978566743658baa3cc7e14e9c1fdebf7c"
                                                                "98b0ebff7f5ff33bc7ad1bc46f55cbc44045be243e1bf0cb4f2671b58c675eee" ),
                                            std::make_pair( 5u, "28e3b8d687eef3912855c4b7a93b33297856658bc7e11fde6743aa3c4e9cbf7c"
                                                                "98b07f5fc7ad6f55ebfff33b1bc4cbc440453e1b4f268c67be24f0cb71b55eee" ),
                                            std::make_pair( 6u, "28e3b8d62855c4b77856658b6743aa3c87eef391a93b3329c7e11fde4e9cbf7c"
                                                                "98b07f5febfff33b40453e1bbe24f0cbc7ad6f551bc4cbc44f268c6771b55eee" ),
                                            std::make_pair( 7u, "28e3b8d62855c4b787eef391a93b332998b07f5febfff33bc7ad6f551bc4cbc4"
                                                                "7856658b6743aa3cc7e11fde4e9cbf7c40453e1bbe24f0cb4f268c6771b55eee" ) ) );

TEST_F( OperationsTest, flip_inplace_small )
{
  for ( const auto& p : std::vector<std::pair<unsigned, std::string>>{{0u, "0b34"}, {1u, "0dc2"}, {2u, "7083"}, {3u, "3807"}} )
  {
    auto tt_s = from_hex<4>( "0738" );
    flip_inplace( tt_s, p.first );
    EXPECT_EQ( tt_s, from_hex<4>( p.second ) );

    auto tt_d = from_hex( 4, "0738" );
    flip_inplace( tt_d, p.first );
    EXPECT_EQ( tt_d, from_hex( 4, p.second ) );
  }
}

TEST_F( OperationsTest, flip_small )
{
  for ( const auto& p : std::vector<std::pair<unsigned, std::string>>{{0u, "0b34"}, {1u, "0dc2"}, {2u, "7083"}, {3u, "3807"}} )
  {
    EXPECT_EQ( flip( from_hex<4>( "0738" ), p.first ), from_hex<4>( p.second ) );
    EXPECT_EQ( flip( from_hex( 4, "0738" ), p.first ), from_hex( 4, p.second ) );
  }
}

class OperationsTestFlip : public OperationsTest, public ::testing::WithParamInterface<std::pair<unsigned, std::string>>
{
};

TEST_P( OperationsTestFlip, flip_inplace_large )
{
  auto tt_s = from_hex<9>( "8725ca41421c7bba0ca86e26347847526fc346d7f3e79e76566a9493fbef11e24f74a07643afd946195f6a372757e045f3bca58f110ef00ebf2d81e80ba5679f" );
  flip_inplace( tt_s, GetParam().first );
  EXPECT_EQ( tt_s, from_hex<9>( GetParam().second ) );

  auto tt_d = from_hex( 9, "8725ca41421c7bba0ca86e26347847526fc346d7f3e79e76566a9493fbef11e24f74a07643afd946195f6a372757e045f3bca58f110ef00ebf2d81e80ba5679f" );
  flip_inplace( tt_d, GetParam().first );
  EXPECT_EQ( tt_d, from_hex( 9, GetParam().second ) );
}

TEST_P( OperationsTestFlip, flip_large )
{
  EXPECT_EQ( flip( from_hex<9>( "8725ca41421c7bba0ca86e26347847526fc346d7f3e79e76566a9493fbef11e24f74a07643afd946195f6a372757e045f3bca58f110ef00ebf2d81e80ba5679f" ),
                   GetParam().first ),
             from_hex<9>( GetParam().second ) );
  EXPECT_EQ( flip( from_hex( 9, "8725ca41421c7bba0ca86e26347847526fc346d7f3e79e76566a9493fbef11e24f74a07643afd946195f6a372757e045f3bca58f110ef00ebf2d81e80ba5679f" ),
                   GetParam().first ),
             from_hex( 9, GetParam().second ) );
}

INSTANTIATE_TEST_CASE_P( OperationsTestFlipInst,
                         OperationsTestFlip,
                         ::testing::Values( std::make_pair( 0u, "4b1ac582812cb7750c549d1938b48ba19fc389ebf3db6db9a9956863f7df22d1"
                                                                "8fb850b9835fe68926af953b1babd08af37c5a4f220df00d7f1e42d4075a9b6f" ),
                                            std::make_pair( 1u, "2d853a141843deea03a29b89c1d21d589f3c197dfcbd6bd9599a616cfebf44b8"
                                                                "1fd1a0d91caf7619465f9acd8d5db015fce3a52f440bf00bef8724b20ea59d6f" ),
                                            std::make_pair( 2u, "7852ac1424c1b7abc08ae66243877425f63c647d3f7ee96765a64939bffe112e"
                                                                "f4470a6734fa9d6491f5a67372750e543fcb5af811e00fe0fbd2188eb05a76f9" ),
                                            std::make_pair( 3u, "258741ca1c42ba7ba80c266e78345247c36fd746e7f3769e6a569394effbe211"
                                                                "744f76a0af4346d95f19376a572745e0bcf38fa50e110ef02dbfe881a50b9f67" ),
                                            std::make_pair( 4u, "ca4187257bba421c6e260ca84752347846d76fc39e76f3e79493566a11e2fbef"
                                                                "a0764f74d94643af6a37195fe0452757a58ff3bcf00e110e81e8bf2d679f0ba5" ),
                                            std::make_pair( 5u, "421c7bba8725ca41347847520ca86e26f3e79e766fc346d7fbef11e2566a9493"
                                                                "43afd9464f74a0762757e045195f6a37110ef00ef3bca58f0ba5679fbf2d81e8" ),
                                            std::make_pair( 6u, "0ca86e26347847528725ca41421c7bba566a9493fbef11e26fc346d7f3e79e76"
                                                                "195f6a372757e0454f74a07643afd946bf2d81e80ba5679ff3bca58f110ef00e" ),
                                            std::make_pair( 7u, "6fc346d7f3e79e76566a9493fbef11e28725ca41421c7bba0ca86e2634784752"
                                                                "f3bca58f110ef00ebf2d81e80ba5679f4f74a07643afd946195f6a372757e045" ),
                                            std::make_pair( 8u, "4f74a07643afd946195f6a372757e045f3bca58f110ef00ebf2d81e80ba5679f"
                                                                "8725ca41421c7bba0ca86e26347847526fc346d7f3e79e76566a9493fbef11e2" ) ) );

TEST_F( OperationsTest, min_base )
{
  {
    auto tt = from_hex<3>( "a0" ); /* (ac) */
    const auto support = min_base_inplace( tt );
    EXPECT_EQ( support, std::vector<uint8_t>( {0, 2} ) );
    EXPECT_EQ( tt, from_hex<3>( "88" ) );
    expand_inplace( tt, support );
    EXPECT_EQ( tt, from_hex<3>( "a0" ) );
  }

  {
    auto tt = from_hex<4>( "3c3c" ); /* [bc] */
    const auto support = min_base_inplace( tt );
    EXPECT_EQ( support, std::vector<uint8_t>( {1, 2} ) );
    EXPECT_EQ( tt, from_hex<4>( "6666" ) );
    expand_inplace( tt, support );
    EXPECT_EQ( tt, from_hex<4>( "3c3c" ) );
  }

  {
    auto tt = from_hex<7>( "ff55ff55ff55ff555555555555555555" ); /* {!a(dg)} */
    const auto support = min_base_inplace( tt );
    EXPECT_EQ( support, std::vector<uint8_t>( {0, 3, 6} ) );
    EXPECT_EQ( tt, from_hex<7>( "d5d5d5d5d5d5d5d5d5d5d5d5d5d5d5d5" ) );
    expand_inplace( tt, support );
    EXPECT_EQ( tt, from_hex<7>( "ff55ff55ff55ff555555555555555555" ) );
  }
}

TEST_F( OperationsTest, expand )
{
  /* create [(ac){bd}] from (ab) and {ac} */
  auto f_and = from_hex( 2, "8" );
  auto f_or = from_hex( 2, "e" );

  auto f_and_e = extend_to( f_and, 4 );
  auto f_or_e = extend_to( f_or, 4 );

  expand_inplace( f_and_e, std::vector<uint8_t>{0, 2} );
  expand_inplace( f_or_e, std::vector<uint8_t>{1, 3} );

  EXPECT_EQ( f_and_e ^ f_or_e, from_hex( 4, "5f6c" ) );
}

TEST_F( OperationsTest, extend_to )
{
  static_truth_table<9> maj, th_extend;
  static_truth_table<8> th;

  create_majority( maj );
  create_threshold( th, 4 );
  extend_to_inplace( th_extend, th );

  EXPECT_EQ( cofactor0( maj, 8 ), th_extend );

  EXPECT_EQ( extend_to<3>( from_hex<2>( "a" ) ), from_hex<3>( "aa" ) );
  EXPECT_EQ( extend_to<3>( from_hex( 2, "a" ) ), from_hex<3>( "aa" ) );
  EXPECT_EQ( extend_to( from_hex<2>( "a" ), 3 ), from_hex( 3, "aa" ) );
  EXPECT_EQ( extend_to( from_hex( 2, "a" ), 3 ), from_hex( 3, "aa" ) );
}

TEST_F( OperationsTest, extend_to_same )
{
  static_truth_table<9> tt1, tt2;

  create_random( tt1 );
  extend_to_inplace( tt2, tt1 );

  EXPECT_EQ( tt1, tt2 );
}

TEST_F( OperationsTest, shrink_to )
{
  EXPECT_EQ( shrink_to<2>( from_hex<3>( "aa" ) ), from_hex<2>( "a" ) );
  EXPECT_EQ( shrink_to<2>( from_hex<3>( "1a" ) ), from_hex<2>( "a" ) );
  EXPECT_EQ( shrink_to<2>( from_hex<2>( "8" ) ), from_hex<2>( "8" ) );

  EXPECT_EQ( shrink_to<5>( from_hex<7>( "cafecafecafecafecafecafecafecafe" ) ), from_hex<5>( "cafecafe" ) );

  EXPECT_EQ( shrink_to<2>( from_hex( 3, "aa" ) ), from_hex<2>( "a" ) );
  EXPECT_EQ( shrink_to<2>( from_hex( 3, "1a" ) ), from_hex<2>( "a" ) );
  EXPECT_EQ( shrink_to<2>( from_hex( 2, "8" ) ), from_hex<2>( "8" ) );

  EXPECT_EQ( shrink_to<5>( from_hex( 7, "cafecafecafecafecafecafecafecafe" ) ), from_hex<5>( "cafecafe" ) );

  EXPECT_EQ( shrink_to( from_hex( 3, "aa" ), 2 ), from_hex( 2, "a" ) );
  EXPECT_EQ( shrink_to( from_hex( 3, "1a" ), 2 ), from_hex( 2, "a" ) );
  EXPECT_EQ( shrink_to( from_hex( 2, "8" ), 2 ), from_hex( 2, "8" ) );

  EXPECT_EQ( shrink_to( from_hex( 7, "cafecafecafecafecafecafecafecafe" ), 5 ), from_hex( 5, "cafecafe" ) );
}

TEST_F( OperationsTest, shift_left )
{
  EXPECT_EQ( shift_left( from_hex<3>( "e8" ), 1 ), from_hex<3>( "d0" ) );
  EXPECT_EQ( shift_left( from_hex<4>( "cafe" ), 4 ), from_hex<4>( "afe0" ) );
  EXPECT_EQ( shift_left( from_hex<7>( "cafeaffe12345678acabacab91837465" ), 4 ), from_hex<7>( "afeaffe12345678acabacab918374650" ) );
  EXPECT_EQ( shift_left( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 8 ), from_hex( 7, "feaffe12345678acabacab9183746500" ) );
  EXPECT_EQ( shift_left( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 64 ), from_hex( 7, "acabacab918374650000000000000000" ) );
  EXPECT_EQ( shift_left( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 0 ), from_hex( 7, "cafeaffe12345678acabacab91837465" ) );
  EXPECT_EQ( shift_left( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 128 ), from_hex( 7, "00000000000000000000000000000000" ) );
}

TEST_F( OperationsTest, shift_right )
{
  EXPECT_EQ( shift_right( from_hex<3>( "e8" ), 1 ), from_hex<3>( "74" ) );
  EXPECT_EQ( shift_right( from_hex<4>( "cafe" ), 4 ), from_hex<4>( "0caf" ) );
  EXPECT_EQ( shift_right( from_hex<7>( "cafeaffe12345678acabacab91837465" ), 4 ), from_hex<7>( "0cafeaffe12345678acabacab9183746" ) );
  EXPECT_EQ( shift_right( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 8 ), from_hex( 7, "00cafeaffe12345678acabacab918374" ) );
  EXPECT_EQ( shift_right( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 64 ), from_hex( 7, "0000000000000000cafeaffe12345678" ) );
  EXPECT_EQ( shift_right( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 0 ), from_hex( 7, "cafeaffe12345678acabacab91837465" ) );
  EXPECT_EQ( shift_right( from_hex( 7, "cafeaffe12345678acabacab91837465" ), 128 ), from_hex( 7, "00000000000000000000000000000000" ) );
}

TEST_F( OperationsTest, majority7 )
{
  const auto a = nth<7>( 0 );
  const auto b = nth<7>( 1 );
  const auto c = nth<7>( 2 );
  const auto d = nth<7>( 3 );
  const auto e = nth<7>( 4 );
  const auto f = nth<7>( 5 );
  const auto g = nth<7>( 6 );

  static_truth_table<7> maj7;
  create_majority( maj7 );

  auto special_func = []( auto a, auto b, auto c, auto d, auto e, auto f ) {
    return ternary_majority( ternary_majority( a, b, c ), d, ternary_majority( e, f, ternary_majority( a, b, c ) ) );
  };

  // special_func is symmetric in variables {a, b, c}
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( a, c, b, d, e, f ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( b, a, c, d, e, f ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( b, c, a, d, e, f ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( c, a, b, d, e, f ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( c, b, a, d, e, f ) );

  // special func is symmetric in variables {d, e, f}
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( a, b, c, d, f, e ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( a, b, c, e, d, f ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( a, b, c, e, f, d ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( a, b, c, f, d, e ) );
  EXPECT_EQ( special_func( a, b, c, d, e, f ), special_func( a, b, c, f, e, d ) );

  const auto sf0 = special_func( a, b, c, d, e, f );
  const auto sf1 = special_func( d, e, f, a, b, c );

  const auto one = ~maj7.construct();

  const auto th0 = cofactor0( maj7, 6 ); /* threshold-4 function (hamming weight >= 4) */
  const auto th1 = cofactor1( maj7, 6 ); /* threshold-3 function (hamming weight >= 3) */
  const auto te = th1 & ~th0;            /* =3 function (hamming weight = 3) */

  EXPECT_EQ( ~th0 | ( sf0 & sf1 ), one );
  EXPECT_EQ( th0 | ( ~sf0 | ~sf1 ), one );
  EXPECT_EQ( ~th1 | ( sf0 | sf1 ), one );
  EXPECT_EQ( th1 | ( ~sf0 & ~sf1 ), one );

  const auto factor1 = d ^ e ^ f;
  const auto factor2 = a ^ b ^ c;
  EXPECT_EQ( sf0, ( factor1 & th1 ) ^ ( ~factor1 & th0 ) );
  EXPECT_EQ( sf1, ( factor1 & th0 ) ^ ( ~factor1 & th1 ) );
  EXPECT_EQ( sf0, ( factor2 & th0 ) ^ ( ~factor2 & th1 ) );
  EXPECT_EQ( sf1, ( factor2 & th1 ) ^ ( ~factor2 & th0 ) );

  EXPECT_EQ( sf0, th0 | ( factor1 & te ) );
  EXPECT_EQ( sf1, th0 | ( factor2 & te ) );

  EXPECT_EQ( ternary_majority( sf0, g, sf1 ), maj7 );
}

template<typename TT>
void majority_decomposition_acw( TT& f1, TT& f2 )
{
  /* num_vars is even */
  ASSERT_EQ( f1.num_vars() % 2, 0 );

  /* k */
  auto k = f1.num_vars() >> 1;

  dynamic_truth_table lhs( 2 * k - 1 );
  create_majority( lhs );
  extend_to_inplace( f1, lhs );

  dynamic_truth_table rhs1( 2 * k - 1 );
  dynamic_truth_table rhs2( 2 * k - 1 );

  create_threshold( rhs1, k );
  create_threshold( rhs2, k - 2 );

  auto rhs1_e = f2.construct();
  auto rhs2_e = f2.construct();
  extend_to_inplace( rhs1_e, rhs1 );
  extend_to_inplace( rhs2_e, rhs2 );

  create_nth_var( f2, 2 * k - 1 );

  f2 = rhs1_e | ( f2 & rhs2_e );
}

template<typename TT>
void majority_decomposition_even( TT& f1, TT& f2 )
{
  /* num_vars is even */
  ASSERT_EQ( f1.num_vars() % 2, 0 );

  /* k is odd */
  auto k = f1.num_vars() >> 1;
  ASSERT_EQ( k % 2, 1 );

  create_threshold( f1, k );
  f2 = f1;

  auto eq_k = f1.construct();
  create_equals( eq_k, k );

  auto factor1 = f1.construct();
  auto factor2 = f2.construct();

  for ( auto i = 0; i < k; ++i )
  {
    auto v1 = f1.construct();
    create_nth_var( v1, i );
    factor1 ^= v1;

    auto v2 = f1.construct();
    create_nth_var( v2, k + i );
    factor2 ^= v2;
  }

  f1 |= eq_k & factor1;
  f2 |= eq_k & factor2;
}

TEST_F( OperationsTest, majority_conjecture_small_acw )
{
  constexpr auto k = 4;
  constexpr auto n = 2 * k + 1;

  static_truth_table<n> maj, f1_e, f2_e;
  static_truth_table<n - 1> f1, f2;

  create_majority( maj );
  majority_decomposition_acw( f1, f2 );

  EXPECT_EQ( ternary_majority( extend_to<n>( f1 ), nth<n>( 2 * k ), extend_to<n>( f2 ) ), maj );
}

TEST_F( OperationsTest, majority_conjecture_small )
{
  constexpr auto k = 3;
  constexpr auto n = 2 * k + 1;

  static_truth_table<n> maj, f1_e, f2_e;
  static_truth_table<n - 1> f1, f2;

  create_majority( maj );
  majority_decomposition_even( f1, f2 );

  EXPECT_EQ( ternary_majority( extend_to<n>( f1 ), nth<n>( 2 * k ), extend_to<n>( f2 ) ), maj );
}

TEST_F( OperationsTest, majority_odd_conjecture )
{
  constexpr auto k = 3;
  constexpr auto n = 2 * k + 1;

  static_truth_table<n> maj;
  create_majority( maj );

  /* create f1 */
  static_truth_table<n - 2> f1_m3;
  create_majority( f1_m3 );
  static_truth_table<n - 1> f1;
  extend_to_inplace( f1, f1_m3 );

  /* create f2 */
  static_truth_table<n - 1> f2, rem;
  create_threshold( f2, k );
  create_equals( rem, k );
  f2 |= ~f1 & rem;

  auto f2_alt = nth( n - 1, n - 3 );
  for ( auto i = 4; i <= n; ++i )
  {
    f2_alt = ternary_majority( nth( n - 1, n - i ), nth( n - 1, n - 2 ), f2_alt );
  }

  static_truth_table<n - 1> f2_alt2_p1, f2_alt2_p2;
  create_threshold( f2_alt2_p1, k );
  create_equals( f2_alt2_p2, k );
  auto f2_alt2 = f2_alt2_p1 | ( f2_alt2_p2 & nth<n - 1>( n - 2 ) );

  EXPECT_EQ( f2, f2_alt2 );

  /* create majority */
  static_truth_table<n> f1_e, f2_e;
  extend_to_inplace( f1_e, f1 );
  extend_to_inplace( f2_e, f2 );

  EXPECT_EQ( maj, ternary_majority( nth<n>( n - 1 ), f1_e, f2_e ) );
}

TEST_F( OperationsTest, copy_truth_tables )
{
  kitty::dynamic_truth_table tt1( 3 ), tt2( 7 );
  kitty::static_truth_table<3> tt3;
  kitty::static_truth_table<7> tt4;

  kitty::create_random( tt1 );
  kitty::create_random( tt2 );
  kitty::create_random( tt3 );
  kitty::create_random( tt4 );

  kitty::dynamic_truth_table tt;

  tt = tt1;
  EXPECT_EQ( tt, tt1 );
  tt = tt2;
  EXPECT_EQ( tt, tt2 );
  tt = tt3;
  EXPECT_TRUE( std::equal( tt.begin(), tt.end(), tt3.begin() ) );
  tt = tt4;
  EXPECT_TRUE( std::equal( tt.begin(), tt.end(), tt4.begin() ) );

  static_truth_table<3> tt_s3;
  tt_s3 = tt1;
  EXPECT_TRUE( std::equal( tt_s3.begin(), tt_s3.end(), tt1.begin() ) );
  tt_s3 = tt3;
  EXPECT_EQ( tt_s3, tt3 );

  static_truth_table<7> tt_s7;
  tt_s7 = tt2;
  EXPECT_TRUE( std::equal( tt_s7.begin(), tt_s7.end(), tt2.begin() ) );
  tt_s7 = tt4;
  EXPECT_EQ( tt_s7, tt4 );
}
