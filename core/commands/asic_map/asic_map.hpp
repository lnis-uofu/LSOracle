namespace alice
{
    class techmap_command : public alice::command{

        public:
        explicit techmap_command( const environment::ptr& env )
            : command( env, "Performs ASIC technology mapping by first LUT mapping, then reading a json file with a techmapped list of NPN functions" ){

            opts.add_option( "--filename,-o", filename, "Verilog file to write out to" )->required();
            //opts.add_option( "--lut_size,-K", lut_size, "LUT size for mapping [DEFAULT = 6]" );
            //opts.add_option( "--cut_size,-C", cut_size, "Max number of priority cuts [DEFAULT = 8]" );
            add_flag("--mig,-m", "Read from the stored MIG network");
        }

        protected:
        void execute(){
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
                auto& mig = *store<mig_ntk>().current();
                mockturtle::mapping_view<mockturtle::mig_network, true> mapped_mig{mig};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4;
                ps.cut_enumeration_ps.cut_limit = 4;
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped_mig, ps );
                const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_mig );
                auto const& klut = *klut_opt;
                mockturtle::topo_view klut_topo{klut};
                std::tuple<mockturtle::klut_network, std::unordered_map <int, std::string>> techmap_test = oracle::techmap_mapped_network<mockturtle::klut_network>(klut_topo); 
                oracle::write_techmapped_verilog(std::get<0>(techmap_test), filename, std::get<1>(techmap_test), "test_top");
                mockturtle::depth_view mapped_depth {std::get<0>(techmap_test)};
                std::cout << "\n\nDepth: " << mapped_depth.depth()<<"\n";

                mockturtle::write_bench(klut_topo, filename + "KLUT.bench");
                mockturtle::write_bench(std::get<0>(techmap_test), filename + "Techmapped.bench");          
            }
            else{
                std::cout << "There is not an MIG network stored.\n";
            }
            }
            else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
                mockturtle::topo_view aig_topo{aig};
                mockturtle::mapping_view <mockturtle::aig_network, true> mapped_aig{aig_topo};
                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = 4;
                ps.cut_enumeration_ps.cut_limit = 4;
                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>( mapped_aig, ps );
                const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_aig );
                auto const& klut = *klut_opt;
                mockturtle::topo_view klut_topo{klut};
                std::tuple<mockturtle::klut_network, std::unordered_map <int, std::string>> techmap_test = oracle::techmap_mapped_network<mockturtle::klut_network>(klut_topo); 
                oracle::write_techmapped_verilog(std::get<0>(techmap_test), filename, std::get<1>(techmap_test), "test_top");
                mockturtle::depth_view mapped_depth {std::get<0>(techmap_test)};
                std::cout << "\n\nDepth: " << mapped_depth.depth()<<"\n";
                mockturtle::write_bench(klut_topo, filename + "KLUT.bench");
                mockturtle::write_bench(std::get<0>(techmap_test), filename + "Techmapped.bench");
            }
            else{
                std::cout << "There is not an AIG network stored.\n";
            }
            }
                
        }

        private:
        std::string filename{};
        };

    ALICE_ADD_COMMAND(techmap, "Output");
}