/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2021 Laboratory for Nano Integrated Systems (LNIS)
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
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

namespace alice
{
class xmg_convert_command : public alice::command
{

public:
    explicit xmg_convert_command(const environment::ptr &env)
        : command(env, "Convert stored network to XMG.")
    {
        add_flag("--aig,-a", "Load from stored AIG (Default)");
        add_flag("--mig,-m", "Load from stored MIG");
        add_flag("--xag,-x", "Load from stored XAG");
	add_flag("--xmg,-g", "Resynth current XMG");
    }

protected:
    template <typename network> void convert(std::string name)
    {
        if (store<std::shared_ptr<mockturtle::names_view<network>>>().empty()) {
            spdlog::error("{} network not stored", name);
            return;
        }
	mockturtle::names_view<network> src =
            *store<std::shared_ptr<mockturtle::names_view<network>>>().current();

        mockturtle::xmg_npn_resynthesis resyn;
	mockturtle::names_view<mockturtle::xmg_network> dest;

	mockturtle::node_resynthesis(dest, src, resyn);

	store<std::shared_ptr<mockturtle::names_view<mockturtle::xmg_network>>>().extend() =
			      std::make_shared<mockturtle::names_view<mockturtle::xmg_network>>(dest);
	spdlog::info("XMG network stored");
    }
    void execute()
    {
	if (is_set("mig")) {
	    convert<mockturtle::mig_network>("MIG");
	} else if (is_set("xmg")) {
	    convert<mockturtle::xmg_network>("XMG");
        } else if (is_set("xag")) {
            convert<mockturtle::xag_network>("XAG");
        } else {
            convert<mockturtle::aig_network>("AIG");
        }
    }
};
ALICE_ADD_COMMAND(xmg_convert, "Convert");
}
