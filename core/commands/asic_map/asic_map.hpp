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


namespace alice
{
class techmap_command : public alice::command
{

public:
    explicit techmap_command(const environment::ptr &env)
        : command(env,
                  "Performs ASIC technology mapping by first LUT mapping, then reading a json file with a techmapped list of NPN functions")
    {

        opts.add_option("--filename,-o", filename,
                        "Verilog file to write out to")->required();
        //opts.add_option( "--lut_size,-K", lut_size, "LUT size for mapping [DEFAULT = 6]" );
        //opts.add_option( "--cut_size,-C", cut_size, "Max number of priority cuts [DEFAULT = 8]" );
        add_flag("--aig,-a", "Read from the stored AIG network");
        add_flag("--NPN, -n", "outputs the NPN classes that make up the function");
    }

protected:
    void execute()
    {
        if (is_set("NPN")) {
            std::unordered_map<std::string, int> npn_count;
            if (is_set("aig")) {
                if (!store<aig_ntk>().empty()) {
                    auto &aig = *store<aig_ntk>().current();
                    mockturtle::topo_view aig_topo{aig};
                    mockturtle::mapping_view <mockturtle::aig_network, true> mapped_aig{aig_topo};
                    mockturtle::lut_mapping_params ps;
                    ps.cut_enumeration_ps.cut_size = 6;
                    ps.cut_enumeration_ps.cut_limit = 6;
                    mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>
                    (mapped_aig, ps);
                    const auto klut_opt =
                        mockturtle::collapse_mapped_network<mockturtle::klut_network>(mapped_aig);
                    auto const &klut = *klut_opt;
                    klut.foreach_node([&](auto const n) {
                        if (klut.is_constant(n) || klut.is_pi(n)) {
                            return;
                        }
                        auto func = klut.node_function(n);
                        auto NPNconfig = kitty::exact_npn_canonization(func);
                        std::string tempstr = kitty::to_hex(std::get<0>(NPNconfig));
                        if (npn_count.find(tempstr) != npn_count.end()) {
                            npn_count.at(tempstr) = npn_count.at(tempstr) + 1;
                        } else {
                            npn_count.insert({tempstr, 1});
                        }
                    });
                    for (std::pair<std::string, int> npnclass : npn_count) {
                        if (npnclass.first.size() > 8)
                            spdlog::debug("{}{}:\t{}",setw(20), npnclass.first, npnclass.second);
                    }
                } else {
                    spdlog::error("There is not an AIG network stored.");
                }
            } else {
                if (!store<mig_ntk>().empty()) {
                    spdlog::info("Beginning tech-mapping");
                    auto &mig = *store<mig_ntk>().current();
                    mockturtle::topo_view mig_topo{mig};
                    mockturtle::mapping_view <mockturtle::mig_network, true> mapped_mig{mig_topo};
                    mockturtle::lut_mapping_params ps;
                    ps.cut_enumeration_ps.cut_size = 6;
                    ps.cut_enumeration_ps.cut_limit = 6;
                    spdlog::info("LUT mapping");
                    mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>
                    (mapped_mig, ps);
                    const auto klut_opt =
                        mockturtle::collapse_mapped_network<mockturtle::klut_network>(mapped_mig);
                    auto const &klut = *klut_opt;
                    klut.foreach_node([&](auto const n) {
                        if (klut.is_constant(n) || klut.is_pi(n)) {
                            return;
                        }
                        auto func = klut.node_function(n);
                        auto NPNconfig = kitty::exact_npn_canonization(func);
                        std::string tempstr = kitty::to_hex(std::get<0>(NPNconfig));
                        if (npn_count.find(tempstr) != npn_count.end()) {
                            npn_count.at(tempstr) = npn_count.at(tempstr) + 1;
                        } else {
                            npn_count.insert({tempstr, 1});
                        }
                    });
                    for (std::pair<std::string, int> npnclass : npn_count) {
                        if (npnclass.first.size() > 8)
                            spdlog::debug("{}{}:\t{}",setw(20), npnclass.first, npnclass.second);
                    }
                } else {
                    spdlog::error("There is not an MIG network stored.");
                }
            }
        } else if (is_set("aig")) {
            if (!store<aig_ntk>().empty()) {
                spdlog::info("Beginning tech-mapping");
                auto &aig = *store<aig_ntk>().current();
                mockturtle::topo_view aig_topo{aig};
                mockturtle::mapping_view <mockturtle::aig_network, true> mapped_aig{aig_topo};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4;
                ps.cut_enumeration_ps.cut_limit = 4;
                spdlog::info("LUT mapping");
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>
                (mapped_aig, ps);
                const auto klut_opt =
                    mockturtle::collapse_mapped_network<mockturtle::klut_network>(mapped_aig);
                auto const &klut = *klut_opt;
                mockturtle::topo_view klut_topo{klut};
                mockturtle::write_bench(klut_topo, filename + "KLUT.bench");
                std::tuple<mockturtle::klut_network, std::unordered_map <int, std::string>>
                        techmap_test = oracle::techmap_mapped_network<mockturtle::klut_network>
                                       (klut_topo);
                mockturtle::write_bench(std::get<0>(techmap_test),
                                        filename + "Techmapped.bench");
                spdlog::info("Outputing mapped netlist");
                oracle::write_techmapped_verilog(std::get<0>(techmap_test), filename,
                                                 std::get<1>(techmap_test), "test_top");
                mockturtle::write_bench(mockturtle::cleanup_dangling(std::get<0>(techmap_test)),
                                        filename + "cleanup.bench");
                mockturtle::depth_view mapped_depth {std::get<0>(techmap_test)};
                // spdlog::info("Final network size: {} Depth: {}", std::get<1>(techmap_test).size(), mapped_depth.depth());

            } else {
                spdlog::error("There is not an AIG network stored.");
            }
        } else {
            if (!store<mig_ntk>().empty()) {
                spdlog::info("Beginning tech-mapping");
                auto &mig = *store<mig_ntk>().current();
                mockturtle::topo_view mig_topo{mig};
                mockturtle::mapping_view <mockturtle::mig_network, true> mapped_mig{mig_topo};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4;
                ps.cut_enumeration_ps.cut_limit = 4;
                spdlog::info("LUT mapping");
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>
                (mapped_mig, ps);
                const auto klut_opt =
                    mockturtle::collapse_mapped_network<mockturtle::klut_network>(mapped_mig);
                auto const &klut = *klut_opt;
                mockturtle::topo_view klut_topo{klut};
                mockturtle::write_bench(klut_topo, filename + "KLUT.bench");
                std::tuple<mockturtle::klut_network, std::unordered_map <int, std::string>>
                        techmap_test = oracle::techmap_mapped_network<mockturtle::klut_network>
                                       (klut_topo);
                mockturtle::write_bench(std::get<0>(techmap_test),
                                        filename + "Techmapped.bench");
                spdlog::info("Outputing mapped netlist");
                oracle::write_techmapped_verilog(std::get<0>(techmap_test), filename,
                                                 std::get<1>(techmap_test), "top");
                mockturtle::write_bench(mockturtle::cleanup_dangling(std::get<0>(techmap_test)),
                                        filename + "cleanup.bench");
                mockturtle::depth_view mapped_depth {std::get<0>(techmap_test)};
                spdlog::info"Final network size: {} Depth: {}",std::get<1>(techmap_test).size(), mapped_depth.depth());

            } else {
                spdlog::error("There is not an MIG network stored.");
            }
        }

    }

private:
    std::string filename{};
};

ALICE_ADD_COMMAND(techmap, "Output");
}
