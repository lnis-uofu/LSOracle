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
#include "algorithms/optimization/optimizers.hpp"
#include "algorithms/optimization/optimizers/aig.hpp"
#include "algorithms/optimization/optimizers/mig.hpp"
#include "algorithms/optimization/optimizers/xag.hpp"
#include "algorithms/optimization/optimizers/xmg.hpp"
#include "algorithms/optimization/optimizers/abc.hpp"
#include "algorithms/optimization/optimizers/noop.hpp"
namespace oracle {
template <typename network>
optimizer<network> *optimize(optimization_strategy_comparator<network> &comparator,
                             optimization_strategy strategy,
                             const mockturtle::window_view<mockturtle::names_view<network>> &part,
                             int index,
                             const std::string &abc_exec)
{
    std::cout << "******************************** optimizing partition " << index << " ********************************" << std::endl;
    std::cout << "Optimizing based on strategy " << comparator.name() << std::endl;
    // depth_view part_depth(part);
    // std::cout << "Original depth " << part_depth.depth() << " gates " << part_depth.num_gates() << " size " << part_depth.size() << std::endl;
    // todo this is gonna leak memory.
    std::vector<optimizer<network>*>optimizers {
        new noop<network>(index, part, strategy, abc_exec),
        new migscript_optimizer<network>(index, part, strategy, abc_exec),
        new migscript2_optimizer<network>(index, part, strategy, abc_exec),
        new migscript3_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript2_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript3_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript4_optimizer<network>(index, part, strategy, abc_exec),
        new aigscript5_optimizer<network>(index, part, strategy, abc_exec),
        new xmg_optimizer<network>(index, part, strategy, abc_exec),
        new xag_optimizer<network>(index, part, strategy, abc_exec),
        new abc_optimizer<network>(index, part, strategy, abc_exec),
   };
    optimizer<network> *best = nullptr;
    for (auto opt = optimizers.begin(); opt != optimizers.end(); opt++) {
        std::cout << "running optimization " << (*opt)->optimizer_name() << std::endl;
        // std::cout << "converting network" << std::endl;
        (*opt)->convert();
        // std::cout << "trying to optimize" << std::endl;
        (*opt)->optimize();
        // std::cout << "checking tech independent metrics." << std::endl;
        node_depth result = (*opt)->independent_metric();
        std::cout << "result depth " << result.depth
                  << " size " << result.nodes << std::endl;

        if (best == nullptr) {
            best = *opt;
            continue;
        }

        if (comparator(**opt, *best)) {
            best = *opt;
            //std::cout << "found a better result" << std::endl;
            continue;
        }
    }
    std::cout << "using " << best->optimizer_name() << " for " << index << std::endl;

    return best;

}
template optimizer<mockturtle::aig_network> *optimize(
    optimization_strategy_comparator<mockturtle::aig_network> &,
    optimization_strategy,
    const mockturtle::window_view<mockturtle::names_view<mockturtle::aig_network>> &,
    int,
    const std::string &);
}
