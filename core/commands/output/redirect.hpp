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
#include <alice/alice.hpp>
#include <stdio.h>

namespace alice
{
class redirect_command : public alice::command
{

public:
    explicit redirect_command(const environment::ptr &env)
        : command(env, "Redirect command output to specified file.")
    {
        opts.add_option("--file,-f,file", filename, "File to write output to.");
        add_flag("--stdout", "Write output to standard output");
    }

protected:
    void execute()
    {
        // if (env->out() != std::cout) {
        //   env->out().close();
        // }
        if (filename != "") {
            std::ofstream *output = new std::ofstream(filename);
            *output << "Redirected output" << std::endl;
            env->reroute(*output, std::cerr);
        } else {
            env->reroute(std::cout, std::cerr);
        }
    }
private:
    std::string filename{};
};

ALICE_ADD_COMMAND(redirect, "Output");
}
