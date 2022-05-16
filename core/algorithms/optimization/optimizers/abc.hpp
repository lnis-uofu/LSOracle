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
#include <string>
#include <mockturtle/mockturtle.hpp>
#include "algorithms/optimization/optimizers/techmapping.hpp"
#include "algorithms/optimization/optimizers/aig.hpp"

namespace oracle {

template< typename network>
class abc_optimizer: public aig_optimizer<network>
{
    using partition = mockturtle::window_view<mockturtle::names_view<network>>;
public:
    abc_optimizer(int index, const partition &original, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(index, original, target, abc_exec) {}

    const std::string optimizer_name()
    {
        return "abc resyn2";
    }

    optimizer<mockturtle::xmg_network> *reapply(int index, const xmg_partition &part)
    {
        return new abc_optimizer<mockturtle::xmg_network>(index, part, this->strategy, this->abc_exec);
    }

    void optimize()
    {
        char *blif_name_char = strdup("/tmp/lsoracle_XXXXXX.blif");
        if (mkstemps(blif_name_char, 5) == -1) {
            throw std::exception();
        }
        std::string blif_name = std::string(blif_name_char);
        std::cout << "writing blif to " << blif_name  << std::endl;

        char *blif_output_name_char = strdup("/tmp/lsoracle_XXXXXX_optimized.blif");
        if (mkstemps(blif_output_name_char, 15) == -1) {
            throw std::exception();
        }
        std::string blif_output_name = std::string(blif_output_name_char);
        std::cout << "writing abc output to " << blif_output_name  << std::endl;

        mockturtle::write_blif_params ps;
        ps.skip_feedthrough = 1u;
        mockturtle::write_blif(this->converted, blif_name, ps);
        std::string script = "abc -c \"read_blif " + blif_name + "; resyn2; write_blif " + blif_output_name + " \"";
        int code = system((script).c_str());
        assert(code == 0);
        std::cout << "optimized with abc" << std::endl;

        mockturtle::names_view<mockturtle::klut_network> klut;
        lorina::return_code read_blif_return_code = lorina::read_blif(blif_output_name, mockturtle::blif_reader(klut));
        assert(read_blif_return_code == lorina::return_code::success);
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
        mockturtle::node_resynthesis(this->optimal, klut, resyn);
        this->optimal.set_network_name(this->converted.get_network_name());
    }
};
};
