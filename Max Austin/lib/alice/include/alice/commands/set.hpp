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
  \file set.hpp
  \brief Sets environment variables

  \author Mathias Soeken
*/

#pragma once

#include "../command.hpp"

namespace alice
{

class set_command : public command
{
public:
  explicit set_command( const environment::ptr& env ) : command( env, "Sets environment variables" )
  {
    add_option( "--var,var", var, "variable name" )->required();
    add_option( "--value,value", value, "variable value" )->required();
  }

protected:
  void execute()
  {
    env->_variables[var] = value;
  }

  nlohmann::json log() const
  {
    return nlohmann::json( {{"var", var}, {"value", value}} );
  }

private:
  std::string var;
  std::string value;
};
}
