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
  \file store.hpp
  \brief Data structure for datatype stores

  \author Mathias Soeken
*/

#pragma once

#include <string>
#include <vector>

#include <fmt/format.h>

namespace alice
{

/*! \brief Store container
 */
template<class T>
class store_container
{
public:
  /*! \brief Default constructor

    \param name Store name
  */
  explicit store_container( const std::string& name ) : _name( name ) {}

  /*! \brief Retrieve mutable reference to current store item
   */
  inline T& current()
  {
    if ( _current < 0 )
    {
      throw fmt::format( "[e] no current {} available", _name );
    }
    return _data[_current];
  }

  /*! \brief Retrieve constant reference to current store item
   */
  inline const T& current() const
  {
    if ( _current < 0 )
    {
      throw fmt::format( "[e] no current {} available", _name );
    }
    return _data[_current];
  }

  /*! \brief Retrieve mutable reference to current store item
   */
  inline T& operator*()
  {
    return current();
  }

  /*! \brief Retrieve constant reference to current store item
   */
  inline const T& operator*() const
  {
    return current();
  }

  /*! \brief Retreive mutable reference at given index

    \param index Index
  */
  inline T& operator[]( std::size_t index )
  {
    if ( index >= _data.size() )
    {
      throw fmt::format( "[e] index {} is out of bounds", index );
    }
    return _data[index];
  }

  /*! \brief Retreive const reference at given index

    \param index Index
  */
  inline const T& operator[]( std::size_t index ) const
  {
    if ( index >= _data.size() )
    {
      throw fmt::format( "[e] index {} is out of bounds", index );
    }
    return _data[index];
  }

  /*! \brief Returns whether store is empty
   */
  inline bool empty() const
  {
    return _data.empty();
  }

  /*! \brief Returns the number of elements in the store
   */
  inline auto size() const
  {
    return _data.size();
  }

  /*! \brief Constant access to store elements
   */
  inline const std::vector<T>& data() const
  {
    return _data;
  }

  /*! \brief Returns the current index in the store */
  inline int current_index() const
  {
    return _current;
  }

  /* \brief Sets the current index */
  inline void set_current_index( unsigned i )
  {
    if ( i < _data.size() )
    {
      _current = i;
    }
  }

  /*! \brief Extend the store by one element and update current element

    The current element is set to the added store element.
  */
  template<class... Args>
  T& extend( Args&&... args )
  {
    _current = _data.size();
    _data.push_back( T(std::forward<Args>( args )...) );
    return _data.back();
  }

  /*! \brief Removes current store element

    If the current element is the only element, the store is empty after this
    operation.  If the current element is last element, the store points to
    the next last-but-one element after this operation.  Otherwise, the element
    after the operation will be the one at the same position.
  */
  void pop_current()
  {
    if ( _data.empty() || _current == -1 ) return;

    _data.erase( _data.begin() + _current );
    if ( _current == static_cast<int>( _data.size() ) )
    {
      --_current;
    }
  }

  /*! \brief Clears all elements in the store
   */
  void clear()
  {
    _data.clear();
    _current = -1;
  }

  template<class... Args>
  T& extend_if_empty( Args&&... args )
  {
    if ( empty() )
    {
      return extend( std::forward<Args>( args )... );
    }
    else
    {
      return current();
    }
  }

private:
  std::string _name;
  std::vector<T> _data;
  int _current{-1};
};

}
