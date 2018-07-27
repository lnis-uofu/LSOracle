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
  \file quit.hpp
  \brief Quits the program

  \author Mathias Soeken
*/

#pragma once

#ifdef _WIN32
#include <stdio.h>
#include <windows.h>
#else
#include <sys/utsname.h>
#endif

#include <thread>

#include <json.hpp>

#include "../command.hpp"

namespace alice
{

class quit_command : public command
{
public:
  explicit quit_command( const environment::ptr& env )
      : command( env, "Quits the program" ) {}

protected:
  void execute()
  {
    env->quit = true;
  }

  nlohmann::json log() const
  {
#ifdef _WIN32
    OSVERSIONINFO osvi;
    ZeroMemory( &osvi, sizeof( OSVERSIONINFO ) );
    osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    return nlohmann::json( {{"sysname", "Windows"},
                            {"majorversion", osvi.dwMajorVersion},
                            {"minorversion", osvi.dwMinorVersion},
                            {"supported_threads", static_cast<int>( std::thread::hardware_concurrency() )}} );
#else
    utsname u;
    uname( &u );
    return nlohmann::json( {{"sysname", std::string( u.sysname )},
                            {"nodename", std::string( u.nodename )},
                            {"release", std::string( u.release )},
                            {"version", std::string( u.version )},
                            {"machine", std::string( u.machine )},
                            {"supported_threads", static_cast<int>( std::thread::hardware_concurrency() )}} );
#endif
  }
};
} // namespace alice
