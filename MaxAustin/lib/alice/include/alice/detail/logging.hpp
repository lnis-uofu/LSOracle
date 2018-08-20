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
  \file logging.hpp
  \brief Data structures for logging

  \author Mathias Soeken
*/

#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <json.hpp>

namespace alice
{

namespace detail
{

class logger
{
public:
  void start( const std::string& filename )
  {
    _filename = filename;
  }

  void log( const nlohmann::json& cmdlog, const std::string& cmdstring, const std::chrono::system_clock::time_point& start )
  {
    auto obj = cmdlog;

    /* add command */
    obj["command"] = cmdstring;

    /* add time */
    const auto start_c = std::chrono::system_clock::to_time_t( start );
    char timestr[20];
    std::strftime( timestr, sizeof( timestr ), "%F %T", std::localtime( &start_c ) );

    obj["time"] = timestr;

    array.push_back( obj );
  }

  void stop()
  {
    std::ofstream os( _filename.c_str(), std::ofstream::out );
    os << array;
  }

private:
  std::string _filename;
  nlohmann::json array = nlohmann::json::array();
};

}
}
