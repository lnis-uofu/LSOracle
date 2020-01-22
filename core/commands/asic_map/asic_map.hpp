

namespace alice
{
    class techmap_command : public alice::command{

        public:
        explicit techmap_command( const environment::ptr& env )
            : command( env, "Performs ASIC technology mapping by first LUT mapping, then reading a json file with a techmapped list of NPN functions" ){

            opts.add_option( "--filename,-o", filename, "Verilog file to write out to" )->required();
            //opts.add_option( "--lut_size,-K", lut_size, "LUT size for mapping [DEFAULT = 6]" );
            //opts.add_option( "--cut_size,-C", cut_size, "Max number of priority cuts [DEFAULT = 8]" );
            add_flag("--aig,-a", "Read from the stored AIG network");
            add_flag("--NPN, -n", "outputs the NPN classes that make up the function");
        }

        protected:
        void execute(){
          if(is_set("NPN")){
            std::unordered_map<std::string, int> npn_count;
            if(is_set("aig")){
              if(!store<aig_ntk>().empty()){
                auto& aig = *store<aig_ntk>().current();
                mockturtle::topo_view aig_topo{aig};
                mockturtle::mapping_view <mockturtle::aig_network, true> mapped_aig{aig_topo};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 6;
                ps.cut_enumeration_ps.cut_limit = 6;
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>( mapped_aig, ps );
                const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_aig );
                auto const& klut = *klut_opt;
                klut.foreach_node( [&]( auto const n ) {
                  if ( klut.is_constant( n ) || klut.is_pi( n ) ){
                    return;
                  }
                  auto func = klut.node_function( n );
                  auto NPNconfig = kitty::exact_npn_canonization(func);
                  std::string tempstr = kitty::to_hex(std::get<0>(NPNconfig));
                  if (npn_count.find(tempstr) != npn_count.end()){
                    npn_count.at(tempstr) = npn_count.at(tempstr) + 1;
                  } else {
                    npn_count.insert({tempstr, 1});
                  }
                });
                for(std::pair<std::string, int> npnclass : npn_count){
                  if (npnclass.first.size() > 8)
                    std::cout << setw(20)  << npnclass.first <<":\t"<< npnclass.second <<"\n";
                }
              }
              else{
                std::cout << "There is not an AIG network stored.\n";
              }
            }
            else{
              if(!store<mig_ntk>().empty()){
                std::cout << "Beginning tech-mapping\n";
                auto& mig = *store<mig_ntk>().current();
                mockturtle::topo_view mig_topo{mig};
                mockturtle::mapping_view <mockturtle::mig_network, true> mapped_mig{mig_topo};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 6;
                ps.cut_enumeration_ps.cut_limit = 6;
                std::cout << "LUT mapping\n";
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped_mig, ps );
                const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_mig );
                auto const& klut = *klut_opt;
                klut.foreach_node( [&]( auto const n ) {
                  if ( klut.is_constant( n ) || klut.is_pi( n ) ){
                    return;
                  }
                  auto func = klut.node_function( n );
                  auto NPNconfig = kitty::exact_npn_canonization(func);
                  std::string tempstr = kitty::to_hex(std::get<0>(NPNconfig));
                  if (npn_count.find(tempstr) != npn_count.end()){
                    npn_count.at(tempstr) = npn_count.at(tempstr) + 1;
                  } else {
                    npn_count.insert({tempstr, 1});
                  }
                });
                for(std::pair<std::string, int> npnclass : npn_count){
                  if (npnclass.first.size() > 8)
                    std::cout << setw(20)  << npnclass.first <<":\t"<< npnclass.second <<"\n";
                }
              }
              else{
                std::cout << "There is not an MIG network stored.\n";
              }
            }
          }
          else if(is_set("aig")){
            if(!store<aig_ntk>().empty()){
              std::cout << "Beginning tech-mapping\n";
              auto& aig = *store<aig_ntk>().current();
              mockturtle::topo_view aig_topo{aig};
              mockturtle::mapping_view <mockturtle::aig_network, true> mapped_aig{aig_topo};
              mockturtle::lut_mapping_params ps;
              ps.cut_enumeration_ps.cut_size = 4;
              ps.cut_enumeration_ps.cut_limit = 4;
              std::cout << "LUT mapping\n";
              mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>( mapped_aig, ps );
              const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_aig );
              auto const& klut = *klut_opt;
              mockturtle::topo_view klut_topo{klut};
              mockturtle::write_bench(klut_topo, filename + "KLUT.bench");
              std::tuple<mockturtle::klut_network, std::unordered_map <int, std::string>> techmap_test = oracle::techmap_mapped_network<mockturtle::klut_network>(klut_topo); 
              mockturtle::write_bench(std::get<0>(techmap_test), filename + "Techmapped.bench");
              std::cout << "Outputing mapped netlist\n";
              oracle::write_techmapped_verilog(std::get<0>(techmap_test), filename, std::get<1>(techmap_test), "test_top");
              mockturtle::write_bench(mockturtle::cleanup_dangling(std::get<0>(techmap_test)), filename + "cleanup.bench" );
              mockturtle::depth_view mapped_depth {std::get<0>(techmap_test)};
              // std::cout << "\n\nFinal network size: " << std::get<1>(techmap_test).size() << " Depth: " << mapped_depth.depth()<<"\n";
 
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
          else{
            if(!store<mig_ntk>().empty()){
              std::cout << "Beginning tech-mapping\n";
              auto& mig = *store<mig_ntk>().current();
              mockturtle::topo_view mig_topo{mig};
              mockturtle::mapping_view <mockturtle::mig_network, true> mapped_mig{mig_topo};
              mockturtle::lut_mapping_params ps;
              ps.cut_enumeration_ps.cut_size = 4;
              ps.cut_enumeration_ps.cut_limit = 4;
              std::cout << "LUT mapping\n";
              mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped_mig, ps );
              const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_mig );
              auto const& klut = *klut_opt;
              mockturtle::topo_view klut_topo{klut};
              mockturtle::write_bench(klut_topo, filename + "KLUT.bench");
              std::tuple<mockturtle::klut_network, std::unordered_map <int, std::string>> techmap_test = oracle::techmap_mapped_network<mockturtle::klut_network>(klut_topo); 
              mockturtle::write_bench(std::get<0>(techmap_test), filename + "Techmapped.bench");
              std::cout << "Outputing mapped netlist\n";
              oracle::write_techmapped_verilog(std::get<0>(techmap_test), filename, std::get<1>(techmap_test), "top");
              mockturtle::write_bench(mockturtle::cleanup_dangling(std::get<0>(techmap_test)), filename + "cleanup.bench" );
              mockturtle::depth_view mapped_depth {std::get<0>(techmap_test)};
              std::cout << "\n\nFinal network size: " << std::get<1>(techmap_test).size() << " Depth: " << mapped_depth.depth()<<"\n";
          
            }
            else{
                std::cout << "There is not an MIG network stored.\n";
            }
          }
                
        }

        private:
          std::string filename{};
        };

    ALICE_ADD_COMMAND(techmap, "Output");
}