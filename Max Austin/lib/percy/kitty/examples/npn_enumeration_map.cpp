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

#include <cstdint>
#include <iostream>
#include <unordered_set>

#include <kitty/kitty.hpp>

/* compile time constant for the number of variables */
auto constexpr num_vars = 4;

int main()
{
  static_assert( num_vars <= 5, "number of variables is limited to 5" );

  /* truth table type in this example */
  using truth_table = kitty::static_truth_table<num_vars>;

  /* set to store all NPN representatives (dynamic to store bits on heap) */
  kitty::dynamic_truth_table map( truth_table::NumBits );

  /* invert bits: 1 means not classified yet */
  std::transform( map.cbegin(), map.cend(), map.begin(), []( auto word ) { return ~word; } );

  /* hash set to store all NPN classes */
  std::unordered_set<truth_table, kitty::hash<truth_table>> classes;

  /* start from 0 */
  int64_t index = 0;
  truth_table tt;

  while ( index != -1 )
  {
    /* create truth table from index value */
    kitty::create_from_words( tt, &index, &index + 1 );

    /* apply NPN canonization and add resulting representative to set;
       while canonization, mark all encountered truth tables in map
     */
    const auto res = kitty::exact_npn_canonization( tt, [&map]( const auto& tt ) { kitty::clear_bit( map, *tt.cbegin() ); } );
    classes.insert( std::get<0>( res ) );

    /* find next non-classified truth table */
    index = find_first_one_bit( map );
  }

  std::cout << "[i] enumerated "
            << map.num_bits() << " functions into "
            << classes.size() << " classes." << std::endl;

  return 0;
}
