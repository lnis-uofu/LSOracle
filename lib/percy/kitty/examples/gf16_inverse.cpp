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

#include <iostream>
#include <vector>

#include <kitty/kitty.hpp>

int main()
{
  /* This is the Boolean chain described in Fig. 1 of [J. Boyar and R. Peralta,
     SEC (2012), 287-298] */
  std::vector<std::string> chain{
      "x5 = x3 ^ x4", "x6 = x1 & x3", "x7 = x2 ^ x6",
      "x8 = x1 ^ x2", "x9 = x4 ^ x6", "x10 = x8 & x9",
      "x11 = x5 & x7", "x12 = x1 & x4", "x13 = x8 & x12",
      "x14 = x8 ^ x13", "x15 = x2 & x3", "x16 = x5 & x15",
      "x17 = x5 ^ x16", "x18 = x6 ^ x17", "x19 = x4 ^ x11",
      "x20 = x6 ^ x14", "x21 = x2 ^ x10"};

  /* create truth tables */
  std::vector<kitty::static_truth_table<4>> steps;
  kitty::create_multiple_from_chain( 4, steps, chain );

  /* output functions are in the last four steps */
  std::vector<kitty::static_truth_table<4>> y{steps[17], steps[18], steps[19], steps[20]};

  /* for each output ... */
  for ( auto i = 0; i < 4; ++i )
  {
    /* print hex representation of function */
    std::cout << "y" << ( i + 1 ) << " = " << kitty::to_hex( y[i] ) << "\n";

    /* print PPRM (algebraic normal form) as in Sect. 3 of the above cited
       paper */
    const auto cubes = esop_from_pprm( y[i] );
    kitty::print_cubes( cubes, 4 );
  }

  std::cout << std::flush;
}
