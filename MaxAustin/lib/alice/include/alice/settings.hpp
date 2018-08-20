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
  \file settings.hpp
  \brief Global static settings for alice

  \author Mathias Soeken
*/

#pragma once

#ifndef ALICE_SETTINGS_WITH_DEFAULT_OPTION
/*! \brief Controls whether default store options are supported

  Default store options are useful, when a shell interface contains several
  stores.  If one command uses a store, e.g., to read a file, it can remember
  the last store, such that a following command does not necessarily need to be
  provided with a flag to select the store again.  For example, if one has a
  store for strings (accessed via option ``--str``) and one for numbers (access
  via option ``--int``), then a call to ``read_text --str file`` would set the
  default option to ``--str``, such that a immediate call to ``print`` would
  not need the ``--str`` option to print the string.  The default store option
  is displayed in the prompt.

  The default value for this setting is ``false``.
*/
#define ALICE_SETTINGS_WITH_DEFAULT_OPTION false
#endif
