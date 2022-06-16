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
#include <mockturtle/mockturtle.hpp>
#include <sys/stat.h>
#include <stdio.h>

namespace oracle
{
    class file_cache
    {
        network_cache( std::string directory): directory(directory) {}

        std::string location(std::string hash, std::string recipe) {
        }

        bool exists(std::string hash, std::string recipe) {
            std::string filename = location(hash, recipe);
            struct stat buffer;
            return (stat (filename.c_str(), &buffer) == 0);
        }

        mockturtle::names_view<klut_network> fetch(std::string hash, std::string recipe) {
            std::string filename = location(hash, recipe);
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            auto const result = lorina::read_blif(filename, mockturtle::blif_reader(klut_name_view));

            if (result != lorina::return_code::success) {
                spdlog::error("parsing failed");
                throw;
            }
            filename.erase(filename.end() - 5, filename.end());
            result.set_network_name(filename);

            return result;
        }

        template <class network> void store(std::string hash, std::string recipe, network &ntk) {

        }

        template <class network, class optimizer, class resynth_func>
        class file_cached_optimizer: optimizer<network>
        {
            using names = mockturtle::names_view<network>;
            using partition = mockturtle::window_view<names>;

        public:
            file_cached_optimizer( resynth_func resyn, optimizer<network> opt ): optimizer(opt), resyn(resyn)
            {
            }

            names optimized()
            {
                    return cached;
            }

            void optimize()
            {
                hash_view<names> hash(original);
                if (exists(copy.network_id(), optimizer<network>::optimizer_name())) {
                    mockturtle::names_view<mockturtle::klut_network> cached_lut =
                        fetch(copy.network_id(), optimizer<network>::optimizer_name());
                    cached = mockturtle::node_resynthesis(cached_lut, resyn);
                } else {
                    optimizer::optimize();
                    cached = optimizer::optimized();
                    store(hash.network_id(), optimizer<network>::optimizer_name(), cached);
                }
            }
        private:
            names cached;
            resynth_func resyn;
        };

        template <class network, class noop_cache_optimizer>
        optimizer<network>* generate_optimizer(const std::string &id,
                                               int index,
                                               mockturtle::window_view<mockturtle::names_view<network> window,
                                               optimization_strategy strategy,
                                               const std::string &abc_exec)
        {
            mockturtle::xag_npn_resynthesis<mockturtle::xag_network> xag_resyn;
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> aig_resyn;
            mockturtle::xmg_npn_resynthesis xmg_resyn;
            mockturtle::mig_npn_resynthesis mig_resyn;
            switch(id)
            {
            case "noop":
                return new file_cached_optimizer(xag_resyn, new noop_optimizer<network>(index, part, strategy, abc_exec));
            case "abc resyn2":
                return new file_cached_optimizer(aig_resyn, new abc_optimizer<network>(index, part, strategy, abc_exec));
            case "migscript":
                return new file_cached_optimizer(mig_resyn, new migscript_optimizer<network>(index, window, strategy, abc_exec));
            case "migscript2":
                return new file_cached_optimizer(mig_resyn, new migscript2_optimizer<network>(index, window, strategy, abc_exec));
            case "migscript3":
                return new file_cached_optimizer(mig_resyn, new migscript3_optimizer<network>(index, window, strategy, abc_exec));
            case "aigscript":
                return new file_cached_optimizer(aig_resyn, new aigscript_optimizer<network>(index, window, strategy, abc_exec));
            case "aigscript2":
                return new file_cached_optimizer(aig_resyn, new aigscript2_optimizer<network>(index, window, strategy, abc_exec));
            case "aigscript3":
                return new file_cached_optimizer(aig_resyn, new aigscript3_optimizer<network>(index, window, strategy, abc_exec));
            case "aigscript4":
                return new file_cached_optimizer(aig_resyn, new aigscript4_optimizer<network>(index, window, strategy, abc_exec));
            case "aigscript5":
                return new file_cached_optimizer(aig_resyn, new aigscript5_optimizer<network>(index, window, strategy, abc_exec));
            case "xmgscript":
                return new file_cached_optimizer(xmg_resyn, new xmgscript_optimizer<network>(index, window, strategy, abc_exec));
            case "xagscript":
                return new file_cached_optimizer(xag_resyn, new xagscript_optimizer<network>(index, window, strategy, abc_exec));
            default:
                throw;
            }

        }

    private:
        std::string directory;
    }
}
