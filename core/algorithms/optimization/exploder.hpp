/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
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
#pragma once
#include <vector>
#include "algorithms/optimization/optimizers.hpp"

namespace oracle {

template <typename network>
class exploderizer
{
public:
    using network_names = mockturtle::names_view<network>;
    using window = mockturtle::window_view<mockturtle::names_view<network>>;
    using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;

    exploderizer(mockturtle::names_view<network> ntk, std::string abc_exec): ntk(ntk), abc_exec(abc_exec) {
    }

    xmg_names exploderize(uint32_t target_depth);

    void integrate(xmg_names &output, xmg_names &optim);

    /*
     * generate a window of a logic cone, starting at the given signal.
     */
    const window extract_cone(typename network_names::signal s);
private:
    network_names ntk;
    const std::string abc_exec;
};
};
