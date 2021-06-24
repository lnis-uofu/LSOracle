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
  \file python.hpp
  \brief Python interface

  \author Mathias Soeken
*/

#pragma once

#if defined ALICE_PYTHON

#include <string>

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../command.hpp"

namespace py = pybind11;

namespace alice
{

namespace detail
{

py::object json_to_python( const nlohmann::json& json )
{
  if ( json.is_null() )
  {
    return py::none();
  }
  else if ( json.is_boolean() )
  {
    return py::bool_( json.get<bool>() );
  }
  else if ( json.is_number() )
  {
    using nlohmann::detail::value_t;
    if ( static_cast<value_t>( json ) == value_t::number_unsigned )
    {
      return py::int_( json.get<unsigned long>() );
    }
    else if ( static_cast<value_t>( json ) == value_t::number_integer )
    {
      return py::int_( json.get<long>() );
    }
    else
    {
      return py::float_( json.get<double>() );
    }
  }
  else if ( json.is_object() )
  {
    py::dict _dict;

    for ( auto it = json.begin(); it != json.end(); ++it )
    {
      _dict[py::str( it.key() )] = json_to_python( it.value() );
    }

    return std::move( _dict );
  }
  else if ( json.is_array() )
  {
    py::list _list;

    for ( const auto& element : json )
    {
      _list.append( json_to_python( element ) );
    }

    return std::move( _list );
  }
  else if ( json.is_string() )
  {
    return py::str( json.get<std::string>() );
  }
  else
  {
    /* should not happen */
    return py::none();
  }
}

class return_value_dict
{
public:
  explicit return_value_dict( const nlohmann::json& map )
  {
    for ( auto it = map.begin(); it != map.end(); ++it )
    {
      if ( it.key() == "__repr__" )
      {
        repr = it.value().get<std::string>();
      }
      else if ( it.key() == "_repr_html_" )
      {
        repr_html = it.value().get<std::string>();
      }
      else
      {
        _dict[py::str( it.key() )] = json_to_python( it.value() );
      }
    }
  }

  py::object __getitem__( const std::string& key ) const
  {
    return _dict.attr( "__getitem__" )( py::str( key ) );
  }

  std::string __repr__() const
  {
    if ( repr.empty() )
    {
      return py::str( _dict.attr( "__repr__" )() );
    }
    else
    {
      return repr;
    }
  }

  std::string _repr_html_() const
  {
    if ( repr_html.empty() )
    {
      return fmt::format( "<pre>{}</pre>", __repr__() );
    }
    else
    {
      return repr_html;
    }
  }

  const py::dict& dict() const
  {
    return _dict;
  }

private:
  py::dict _dict;

  std::string repr;
  std::string repr_html;
};

inline std::vector<std::string> make_args( const std::string& name, py::kwargs kwargs )
{
  std::vector<std::string> pargs = {name};

  for ( const auto& kp : kwargs )
  {
    // get the key as string
    const auto skey = kp.first.cast<std::string>();
    const auto value = kp.second;

    // TODO cast float to string?
    if ( py::isinstance<py::bool_>( value ) )
    {
      if ( value.cast<bool>() )
      {
        pargs.push_back( "--" + skey );
      }
    }
    else if ( py::isinstance<py::int_>( value ) )
    {
      pargs.push_back( "--" + skey );
      pargs.push_back( std::to_string( value.cast<int>() ) );
    }
    else
    {
      pargs.push_back( "--" + skey );
      pargs.push_back( value.cast<std::string>() );
    }
  }

  return pargs;
}

template<typename CLI>
void create_python_module( CLI& cli, py::module& m )
{
  m.doc() = "Python bindings";

  py::class_<return_value_dict> representer( m, "ReturnValueDict" );
  representer
      .def( "__getitem__", &return_value_dict::__getitem__ )
      .def( "__repr__", &return_value_dict::__repr__ )
      .def( "_repr_html_", &return_value_dict::_repr_html_ )
      .def( "dict", &return_value_dict::dict );

  for ( const auto& p : cli.env->commands() )
  {
    m.def( p.first.c_str(), [p]( py::kwargs kwargs ) -> py::object {
      p.second->run( make_args( p.first, kwargs ) );

      const auto log = p.second->log();

      if ( log.is_object() )
      {
        return py::cast( return_value_dict( log ) );
      }
      else
      {
        return py::none();
      }
    },
           p.second->caption().c_str() );
  }
}
}
}

#endif
