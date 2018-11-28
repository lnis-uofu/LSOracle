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
#include <fstream>
#include <iostream>
#include <unordered_set>

#include <kitty/kitty.hpp>

/* compile time constant for the number of variables */
auto constexpr num_vars = 4;

int main( int argc, char ** argv )
{
  (void)argc;

  static_assert( num_vars <= 5, "number of variables is limited to 5" );

  /* truth table type in this example */
  using truth_table = kitty::static_truth_table<num_vars>;

  /* set to store all NPN and spectral representatives */
  using truth_table_set = std::unordered_set<truth_table, kitty::hash<truth_table>>;
  truth_table_set classes;

  truth_table tt;
  auto ctr = 0u;

  std::ifstream in( argv[1] );

  std::string line;
  while ( std::getline( in, line ) )
  {
    create_from_hex_string( tt, line );
    classes.insert( kitty::exact_spectral_canonization( tt ) );

    ++ctr;
  }

  std::cout << "[i] enumerated "
            << ctr << " functions into "
            << classes.size() << " classes." << std::endl;

  return 0;
}
