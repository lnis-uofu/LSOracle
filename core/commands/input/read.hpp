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
/*Reads an aig file and stores the AIG network in a store*/
class read_command : public alice::command
{

public:
    explicit read_command(const environment::ptr &env)
        : command(env,
                  "Uses the lorina library to read an RTL file into a specified network type")
    {

        opts.add_option("--filename,filename", filename,
                        "File to read in [.aig, .blif, .v]")->required();
        add_flag("--mig,-m", "Store file as MIG network (AIG network is default)");
        add_flag("--klut,-k", "Store file as KLUT network (AIG network is default)");
        add_flag("--xag,-x", "Store file as XAG network (AIG network is default)");
    }

protected:
    void execute()
    {

        if (oracle::checkExt(filename, "aig")) {
            if (is_set("mig")) {
                mockturtle::mig_network ntk;
                mockturtle::names_view<mockturtle::mig_network> names_view{ntk};
                lorina::return_code result = lorina::read_aiger(filename,
                                             mockturtle::aiger_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read aiger file";
                    return;
                }
                store<mig_ntk>().extend() = std::make_shared<mig_names>(names_view);
                env->out() << "MIG network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            }
            if (is_set("klut")) {
                mockturtle::aig_network ntk;
                mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
                lorina::return_code result = lorina::read_verilog(filename,
                                             mockturtle::verilog_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read verilog file";
                    return;
                }

                mockturtle::mapping_view<aig_names, true> mapped_aig{names_view};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4;
                mockturtle::lut_mapping<mockturtle::mapping_view<aig_names, true>, true>
                (mapped_aig, ps);

                const auto klut = *mockturtle::collapse_mapped_network<klut_names>(mapped_aig);

                store<klut_ntk>().extend() = std::make_shared<klut_names>(klut);
                env->out() << "KLUT network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            } else if (is_set("xag")) {
                mockturtle::xag_network ntk;
                mockturtle::names_view<mockturtle::xag_network> names_view{ntk};
                lorina::return_code result = lorina::read_aiger(filename,
                                             mockturtle::aiger_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read aiger file";
                    return;
                }
                store<xag_ntk>().extend() = std::make_shared<xag_names>(names_view);
                env->out() << "XAG network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            } else {
                mockturtle::aig_network ntk;
                mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
                lorina::return_code result = lorina::read_aiger(filename,
                                             mockturtle::aiger_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read aiger file";
                    return;
                }
                store<aig_ntk>().extend() = std::make_shared<aig_names>(names_view);
                env->out() << "AIG network stored\n";

                filename.erase(filename.end() - 4, filename.end());
                names_view.set_network_name(filename);
            }

        } else if (oracle::checkExt(filename, "blif")) {

            if (is_set("mig")) {
                mockturtle::klut_network klut_ntk;
                mockturtle::names_view<mockturtle::klut_network> names_view{klut_ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read blif file";
                    return;
                }

                mockturtle::mig_npn_resynthesis resyn;

                mockturtle::mig_network ntk;
                mockturtle::names_view<mockturtle::mig_network>named_dest(ntk);

                mockturtle::node_resynthesis(named_dest, names_view, resyn);

                store<mig_ntk>().extend() = std::make_shared<mig_names>(named_dest);
                env->out() << "MIG network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                names_view.set_network_name(filename);
            } else if (is_set("klut")) {
                mockturtle::klut_network ntk;
                mockturtle::names_view<mockturtle::klut_network> names_view{ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read blif file";
                    return;
                }

                store<klut_ntk>().extend() = std::make_shared<klut_names>(names_view);
                env->out() << "KLUT network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                names_view.set_network_name(filename);

            } else if (is_set("xag")) {
                mockturtle::klut_network klut_ntk;
                mockturtle::names_view<mockturtle::klut_network> names_view{klut_ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read blif file";
                    return;
                }

                mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;

                mockturtle::xag_network ntk;
                mockturtle::names_view<mockturtle::xag_network>named_dest(ntk);

                mockturtle::node_resynthesis(named_dest, names_view, resyn);

                store<xag_ntk>().extend() = std::make_shared<xag_names>(named_dest);
                env->out() << "XAG network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                names_view.set_network_name(filename);

            } else {
                mockturtle::klut_network klut_ntk;
                mockturtle::names_view<mockturtle::klut_network> names_view{klut_ntk};
                auto const result = lorina::read_blif(filename,
                                                      mockturtle::blif_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read blif file";
                    return;
                }

                mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;

                mockturtle::aig_network ntk;
                mockturtle::names_view<mockturtle::aig_network>named_dest(ntk);

                mockturtle::node_resynthesis(named_dest, names_view, resyn);

                store<aig_ntk>().extend() = std::make_shared<aig_names>(named_dest);
                env->out() << "AIG network stored\n";

                filename.erase(filename.end() - 5, filename.end());
                names_view.set_network_name(filename);
            }

        } else if (oracle::checkExt(filename, "v")) {

            if (is_set("mig")) {
                mockturtle::mig_network ntk;
                mockturtle::names_view<mockturtle::mig_network> names_view{ntk};
                lorina::return_code result = lorina::read_verilog(filename,
                                             mockturtle::verilog_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read verilog file";
                    return;
                }

                store<mig_ntk>().extend() = std::make_shared<mig_names>(names_view);
                env->out() << "MIG network stored" << std::endl;

                filename.erase(filename.end() - 2, filename.end());
                names_view.set_network_name(filename);
            } else if (is_set("klut")) {
                mockturtle::aig_network ntk;
                mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
                lorina::return_code result = lorina::read_verilog(filename,
                                             mockturtle::verilog_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read verilog file";
                    return;
                }

                mockturtle::mapping_view<aig_names, true> mapped_aig{names_view};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4;
                mockturtle::lut_mapping<mockturtle::mapping_view<aig_names, true>, true>
                (mapped_aig, ps);

                const auto klut = *mockturtle::collapse_mapped_network<klut_names>(mapped_aig);

                store<klut_ntk>().extend() = std::make_shared<klut_names>(klut);
                env->out() << "KLUT network stored\n";

                filename.erase(filename.end() - 2, filename.end());
                names_view.set_network_name(filename);

            } else if (is_set("xag")) {
                mockturtle::xag_network ntk;
                mockturtle::names_view<mockturtle::xag_network> names_view{ntk};
                lorina::return_code result = lorina::read_verilog(filename,
                                             mockturtle::verilog_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read verilog file";
                    return;
                }

                store<xag_ntk>().extend() = std::make_shared<xag_names>(names_view);
                env->out() << "XAG network stored" << std::endl;

                filename.erase(filename.end() - 2, filename.end());
                names_view.set_network_name(filename);
            } else {
                mockturtle::aig_network ntk;
                mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
                lorina::return_code result = lorina::read_verilog(filename,
                                             mockturtle::verilog_reader(names_view));
                if (result != lorina::return_code::success) {
                    env->err() << "Unable to read verilog file";
                    return;
                }
                store<aig_ntk>().extend() = std::make_shared<aig_names>(names_view);
                env->out() << "AIG network stored" << std::endl;

                filename.erase(filename.end() - 2, filename.end());
                names_view.set_network_name(filename);
            }
        } else {
            env->err() << filename <<
                       " is not a valid input file. Accepted file extensions are .aig, .blif, and .v\n";
        }
    }
private:
    std::string filename{};
};

ALICE_ADD_COMMAND(read, "Input");
}
