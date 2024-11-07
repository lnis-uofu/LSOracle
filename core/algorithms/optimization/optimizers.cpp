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

#include <mockturtle/mockturtle.hpp>
namespace oracle
{
template<typename T>
std::string basic_techmap(const std::string &tech_script, const std::string &abc_exec, const T &optimal, const std::string &temp_prefix)
{
    std::cout << "starting basic techmapping" << std::endl;
    char *blif = strdup("/tmp/lsoracle_XXXXXX.blif");
    if (mkstemps(blif, 5) == -1) {
        throw std::exception();
    }
    std::string input_blif = std::string(blif);
    // std::string input_blif = fmt::format("{}.{}.tech.blif", temp_prefix, optimal.get_network_name());
    std::cout << "generated blif " << input_blif << std::endl;

    char *verilog = strdup("/tmp/lsoracle_XXXXXX.v");
    if (mkstemps(verilog, 2) == -1) {
        throw std::exception();
    }
    std::string output_verilog = std::string(verilog);
    // std::string output_verilog = fmt::format("{}.{}.tech.v", temp_prefix, optimal.get_network_name());
    std::cout << "writing output to " << output_verilog << std::endl;

    char *abc = strdup("/tmp/lsoracle_XXXXXX.abc");
    if (mkstemps(abc, 4) == -1) {
        throw std::exception();
    }
    std::string abc_script = std::string(abc);
    // std::string abc_script = fmt::format("{}.{}.tech.abc", temp_prefix, optimal.get_network_name());
    std::cout << "generated ABC script " << abc_script << std::endl;

    std::ofstream script(abc_script);
    script << "print_fanio; " << tech_script << "; print_fanio;" << std::endl;
    script.close();
    std::cout << "calling ABC" << std::endl;
    mockturtle::write_blif_params ps;
    ps.skip_feedthrough = 1u;
    mockturtle::write_blif(optimal, input_blif, ps);
    int code = system((abc_exec + " -F " + abc_script +
                       " -o " + output_verilog +
                       " " + input_blif).c_str());
    assert(code == 0);
    std::cout << "done techmapping" << std::endl;
    // TODO close everything
    return output_verilog;
};
template std::string basic_techmap<aig_names>(const std::string &, const std::string &, const aig_names &, const std::string &);
template std::string basic_techmap<xag_names>(const std::string &, const std::string &, const xag_names &, const std::string &);
template std::string basic_techmap<mig_names>(const std::string &, const std::string &, const mig_names &, const std::string &);
template std::string basic_techmap<xmg_names>(const std::string &, const std::string &, const xmg_names &, const std::string &);
