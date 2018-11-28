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

/*! \cond PRIVATE */

#pragma once

#include <cstdio>
#include <cstdlib>
#ifndef _WIN32
#include <wordexp.h>
#endif

#include <algorithm>
#include <cctype>
#include <locale>
#include <memory>
#include <string>

#include <fmt/format.h>

namespace alice
{

namespace detail
{

/* string utils are from https://stackoverflow.com/a/217605 */

inline void ltrim( std::string& s )
{
  s.erase( s.begin(), std::find_if( s.begin(), s.end(), []( int ch ) {
             return !std::isspace( ch );
           } ) );
}

inline void rtrim( std::string& s )
{
  s.erase( std::find_if( s.rbegin(), s.rend(), []( int ch ) {
             return !std::isspace( ch );
           } )
               .base(),
           s.end() );
}

inline void trim( std::string& s )
{
  ltrim( s );
  rtrim( s );
}

inline std::string ltrim_copy( std::string s )
{
  ltrim( s );
  return s;
}

inline std::string rtrim_copy( std::string s )
{
  rtrim( s );
  return s;
}

inline std::string trim_copy( std::string s )
{
  trim( s );
  return s;
}

/* format with vector (see https://stackoverflow.com/questions/39493542/building-a-dynamic-list-of-named-arguments-for-fmtlib) */
inline std::string format_with_vector( const std::string& fmtstr, const std::vector<std::string>& values )
{
  assert( values.size() <= 16u );

  std::vector<fmt::internal::Value> data( values.size() );
  fmt::ULongLong types = 0;

  for ( auto i = 0u; i < values.size(); ++i )
  {
    types |= static_cast<uint64_t>( fmt::internal::Value::STRING ) << ( i * 4 );
    data[i].string.value = values[i].data();
    data[i].string.size = values[i].size();
  }

  return fmt::format( fmtstr, fmt::ArgList( types, &data[0] ) );
}

template<char sep>
inline std::vector<std::string> split_with_quotes( const std::string& commands )
{
  std::vector<std::string> result;
  std::string current;

  enum _state
  {
    normal,
    quote,
    escape
  };

  _state s = normal;

  for ( auto c : commands )
  {
    switch ( s )
    {
    case normal:
      switch ( c )
      {
      case '"':
        current += c;
        s = quote;
        break;

      case sep:
        trim( current );
        result.push_back( current );
        current.clear();
        break;

      default:
        current += c;
        break;
      }
      break;

    case quote:
      switch ( c )
      {
      case '"':
        current += c;
        s = normal;
        break;

      case '\\':
        current += c;
        s = escape;
        break;

      default:
        current += c;
        break;
      };
      break;

    case escape:
      current += c;
      s = quote;
      break;
    }
  }

  trim( current );
  if ( !current.empty() )
  {
    result.push_back( current );
  }

  return result;
}

// https://stackoverflow.com/a/14266139
inline std::vector<std::string> split( const std::string& str, const std::string& sep )
{
  std::vector<std::string> result;

  size_t last = 0;
  size_t next = 0;
  while ( ( next = str.find( sep, last ) ) != std::string::npos )
  {
    result.push_back( str.substr( last, next - last ) );
    last = next + 1;
  }
  result.push_back( str.substr( last ) );

  return result;
}

#ifdef _WIN32
inline std::pair<int, std::string> execute_program( const std::string& cmd )
{
  return {-1, "[w] program execution not yet supported in Windows\n"};
}
#else
// from http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
inline std::pair<int, std::string> execute_program( const std::string& cmd )
{
  char buffer[128];
  std::string result;
  int exit_status;

  std::shared_ptr<FILE> pipe( popen( cmd.c_str(), "r" ), [&exit_status]( FILE* fp ) { auto status = pclose( fp ); exit_status = WEXITSTATUS( status ); } );
  if ( !pipe )
  {
    throw std::runtime_error( "[e] popen() failed" );
  }
  while ( !feof( pipe.get() ) )
  {
    if ( fgets( buffer, 128, pipe.get() ) != NULL )
    {
      result += buffer;
    }
  }
  return {exit_status, result};
}
#endif

// based on https://stackoverflow.com/questions/5612182/convert-string-with-explicit-escape-sequence-into-relative-character
inline std::string unescape_quotes( const std::string& s )
{
  std::string res;
  std::string::const_iterator it = s.begin();

  while ( it != s.end() )
  {
    char c = *it++;
    if ( c == '\\' && it != s.end() && *it == '"' )
    {
      c = '"';
      ++it;
    }
    res += c;
  }

  return res;
}

#ifdef _WIN32
inline const std::string& word_exp_filename( const std::string& filename )
{
  return filename;
}
#else
inline std::string word_exp_filename( const std::string& filename )
{
  std::string result;

  wordexp_t p;
  wordexp( filename.c_str(), &p, 0 );

  for ( auto i = 0u; i < p.we_wordc; ++i )
  {
    if ( !result.empty() )
    {
      result += " ";
    }
    result += std::string( p.we_wordv[i] );
  }

  wordfree( &p );

  return result;
}
#endif
} /* namespace detail */
} /* namespace alice */

/*! \endcond */
