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

#include <kitty/kitty.hpp>

int main( int argc, char ** argv )
{
  /* we initialize TT using a lambda, since we do not know its size yet */
  const auto tt = [&]() {
    if ( argc == 1 ) /* no command line argument -> chose random */
    {
      kitty::dynamic_truth_table tt( 10 );
      kitty::create_random( tt );
      return tt;
    }
    else /* command line argument is binary string */
    {
      std::string spec( argv[1] );
      auto size = spec.size();
      auto num_vars = 0;

      /* compute number of variables */
      while ( size >>= 1 )
        ++num_vars;

      kitty::dynamic_truth_table tt( num_vars );
      kitty::create_from_binary_string( tt, spec );

      return tt;
    }
  }();

  /* compute ISOP */
  const auto cubes = kitty::isop( tt );

  /* print ISOP */
  for ( auto cube : cubes )
  {
    cube.print( tt.num_vars() );
    std::cout << std::endl;
  }

  return 0;
}
