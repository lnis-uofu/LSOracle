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

#include <kitty/constructors.hpp>
#include <kitty/cnf.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class CNFTest : public kitty::testing::Test
{
};

TEST_F( CNFTest, small_cnf_dynamic )
{
  auto f = from_hex( 3, "c2" );
  const auto cubes = cnf_characteristic( f );

  dynamic_truth_table char1( f.num_vars() + 1 ), char2( f.num_vars() + 1 );
  create_from_clauses( char1, cubes );
  create_characteristic( char2, f );
  EXPECT_EQ( char1, char2 );
}

TEST_F( CNFTest, random_cnf_dynamic )
{
  dynamic_truth_table f( 10u ), f_c1( 11u ), f_c2( 11u );
  for ( auto i = 0; i < 50; ++i )
  {
    create_random( f );
    create_from_clauses( f_c1, cnf_characteristic( f ) );
    create_characteristic( f_c2, f );
    EXPECT_EQ( f_c1, f_c2 );
  }
}

TEST_F( CNFTest, random_cnf_static )
{
  static_truth_table<10> f;
  static_truth_table<11> f_c1, f_c2;
  for ( auto i = 0; i < 50; ++i )
  {
    create_random( f );
    create_from_clauses( f_c1, cnf_characteristic( f ) );
    create_characteristic( f_c2, f );
    EXPECT_EQ( f_c1, f_c2 );
  }
}