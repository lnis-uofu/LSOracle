/* alice: C++ command shell library
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

#include <alice/alice.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace alice
{

ALICE_ADD_STORE( std::string, "str", "s", "string", "strings" )

ALICE_PRINT_STORE( std::string, os, element )
{
  os << element << std::endl;
}

ALICE_ADD_FILE_TYPE( text, "Text" )

ALICE_READ_FILE( std::string, text, filename, cmd )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

ALICE_WRITE_FILE( std::string, text, element, filename, cmd )
{
  std::ofstream out( filename.c_str(), std::ofstream::out );
  out << element;
}

ALICE_COMMAND(upper, "Manipulation", "changes string to upper bound")
{
  auto& str = store<std::string>().current();
  std::transform( str.begin(), str.end(), str.begin(), ::toupper );
}

}

ALICE_MAIN( tutorial3 )
