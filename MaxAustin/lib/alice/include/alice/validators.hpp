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

/*
  \file validators.hpp
  \brief Some validators which do not exist in CLI11

  \author Mathias Soeken
*/

#pragma once

#include <string>

#include <CLI11.hpp>

#include "detail/utils.hpp"

namespace alice
{
/*! \brief Checks whether file exists (after expansion)

  Before checking whether the file exists, tilde characters for home directories
  and environment variables are expanded (using `wordexp`).

  In Windows this method is just forwarding to `CLI::EistingFile` and does not
  perform any substitution.

  Camel case convention for this function is intentional to match the convention
  of CLI11, since this function is used when declaring CLI options.

  \param filename Filename
*/
#ifdef _WIN32
inline std::string ExistingFileWordExp( const std::string& filename )
{
  return CLI::ExistingFile( filename );
}
#else
inline std::string ExistingFileWordExp( const std::string& filename )
{
  std::string ret;
  for ( const auto& name : detail::split( detail::word_exp_filename( filename ), " " ) )
  {
    ret += CLI::ExistingFile( name );
  }
  return ret;
}
#endif
}
