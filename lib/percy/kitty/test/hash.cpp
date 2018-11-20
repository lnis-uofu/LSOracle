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

#include <cstdlib>
#include <limits>
#include <unordered_map>

#include <gtest/gtest.h>

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/hash.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class HashTest : public kitty::testing::Test
{
};

TEST_F( HashTest, length_of_size_t )
{
  EXPECT_EQ( std::numeric_limits<std::size_t>::digits, 64 );
}

TEST_F( HashTest, hash_static )
{
  std::unordered_map<static_truth_table<10>, uint64_t, hash<static_truth_table<10>>> counts;

  for ( auto i = 0; i < 10; ++i )
  {
    static_truth_table<10> tt;
    create_random( tt );
    counts[tt] = count_ones( tt );
  }

  EXPECT_LE( counts.size(), 10u );
}


TEST_F( HashTest, hash_dynamic )
{
  std::unordered_map<dynamic_truth_table, uint64_t, hash<dynamic_truth_table>> counts;

  for ( auto i = 0; i < 10; ++i )
  {
    dynamic_truth_table tt( 10u );
    create_random( tt );
    counts[tt] = count_ones( tt );
  }

  EXPECT_LE( counts.size(), 10u );
}
