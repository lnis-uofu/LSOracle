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
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{
class read_verilog_command : public alice::command
{

public:
    explicit read_verilog_command(const environment::ptr &env)
        : command(env, "Uses the lorina library to read in an Verilog file")
    {

        opts.add_option("--filename,filename", filename,
                        "Verilog file to read in")->required();
        add_flag("--mig,-m",
                 "Store Verilog file as MIG network (AIG network is default)");
    }

protected:
    void execute()
    {

        if (oracle::checkExt(filename, "v")) {
            if (is_set("mig")) {
                mockturtle::mig_network mig;
                mockturtle::names_view<mockturtle::mig_network> names_view{mig};
                lorina::return_code result = lorina::read_verilog(filename,
                                             mockturtle::verilog_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read verilog file." << std::endl;
                    return;
                }
                store<mig_ntk>().extend() = std::make_shared<mig_names>(names_view);
                env->out() << "MIG network stored" << std::endl;

                filename.erase(filename.end() - 2, filename.end());
                names_view.set_network_name(filename);
            } else {
                mockturtle::aig_network aig;
                mockturtle::names_view<mockturtle::aig_network> names_view{aig};
                lorina::return_code result = lorina::read_verilog(filename,
                                             mockturtle::verilog_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read verilog file." << std::endl;
                    return;
                }
                store<aig_ntk>().extend() = std::make_shared<aig_names>(names_view);
                env->out() << "AIG network stored" << std::endl;

                filename.erase(filename.end() - 2, filename.end());
                names_view.set_network_name(filename);
            }
        } else {
            env->err() << filename << " is not a valid Verilog file\n";
        }
    }
private:
    std::string filename{};
};

ALICE_ADD_COMMAND(read_verilog, "Input");
}
