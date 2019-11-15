#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{

class test_aig_then_part_command : public alice::command{

    public:
        explicit test_aig_then_part_command( const environment::ptr& env )
                : command( env, "Runs AIG optimization on entire circuit before partitioning and determining best MIG partitions" ){

                opts.add_option( "--num_parts,-p", num_parts, "Number of partitions to create" )->required();
                opts.add_option( "--cnn_model,-c", cnn_model, "Trained CNN model for classification" );
                opts.add_option( "--out,-o", out_file, "Verilog output" )->required();
                add_flag("--brute,-b", "Uses a brute force approach instead of classification");
        }

    protected:
        void execute(){

            mockturtle::mig_npn_resynthesis resyn_mig;
            mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;

            std::vector<int> mig_parts;
            if(!store<mockturtle::aig_network>().empty()){

                auto ntk_aig = store<mockturtle::aig_network>().current();
                std::string file_base = ntk_aig._storage->net_name;
                mockturtle::depth_view orig_ntk_depth{ntk_aig};
                // std::cout << "ntk_aig size = " << ntk_aig.num_gates() << " and depth = " << orig_ntk_depth.depth() << "\n";
                std::string net_name = ntk_aig._storage->net_name;
                
                mockturtle::aig_script aigopt;
                ntk_aig = aigopt.run(ntk_aig);
                mockturtle::depth_view aig_opt_depth{ntk_aig};
                // std::cout << "optimized aig network size = " << ntk_aig.num_gates() << " and depth = " << aig_opt_depth.depth() << "\n";

                oracle::partition_manager<mockturtle::aig_network> partitions_aig(ntk_aig, num_parts);
                if(is_set("brute")){

                    for(int i = 0; i < num_parts; i++){
                        oracle::partition_view<mockturtle::aig_network> part_aig = partitions_aig.create_part(ntk_aig, i);

                        auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
                        mockturtle::depth_view part_aig_depth{opt_aig};
                        // std::cout << "aig part size = " << opt_aig.num_gates() << " and depth = " << part_aig_depth.depth() << "\n";
                        mockturtle::aig_script aigopt;
                        opt_aig = aigopt.run(opt_aig);
                        mockturtle::depth_view part_aig_opt_depth{opt_aig};
                        int aig_opt_size = opt_aig.num_gates();
                        int aig_opt_depth = part_aig_opt_depth.depth();
                        // std::cout << "optimized aig part size = " << aig_opt_size << " and depth = " << aig_opt_depth << "\n";

                        auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( part_aig, resyn_mig );
                        mockturtle::depth_view part_mig_depth{opt_mig};
                        // std::cout << "mig part size = " << opt_mig.num_gates() << " and depth = " << part_mig_depth.depth() << "\n";
                        mockturtle::mig_script migopt;
                        opt_mig = migopt.run(opt_mig);
                        mockturtle::depth_view part_mig_opt_depth{opt_mig};
                        int mig_opt_size = opt_mig.num_gates();
                        int mig_opt_depth = part_mig_opt_depth.depth();
                        // std::cout << "optimized mig part size = " << mig_opt_size << " and depth = " << mig_opt_depth << "\n";

                        if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
                            // std::cout << "AIG wins\n";
                        }
                        else{
                            // std::cout << "MIG wins\n";
                            mig_parts.push_back(i);
                        }
                    }

                }
                else{
                    if(!cnn_model.empty()){
                        partitions_aig.run_classification(ntk_aig, cnn_model);

                        mig_parts = partitions_aig.get_mig_parts();
                    }
                    else{
                        std::cout << "Must include CNN model json file\n";
                    }

                }

                mockturtle::mig_network ntk_mig = aig_to_mig(ntk_aig, 0); //mockturtle::node_resynthesis<mockturtle::mig_network>( ntk_aig, resyn_mig );
                oracle::partition_manager<mockturtle::mig_network> partitions_mig(ntk_mig, partitions_aig.get_all_part_connections(), 
                        partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(), partitions_aig.get_part_num());
                
                // std::cout << "Scheduled optimization:\n";
                // std::cout << "MIG Optimization = {";
                // for(int i = 0; i < mig_parts.size(); i++){
                //     std::cout << mig_parts.at(i) << " ";
                // }
                // std::cout << "}\n";

                for(int i = 0; i < mig_parts.size(); i++){
                    // std::cout << "Optimize partition " << mig_parts.at(i) << "\n";
                    oracle::partition_view<mockturtle::mig_network> part = partitions_mig.create_part(ntk_mig, mig_parts.at(i));
                    mockturtle::depth_view part_depth{part};
                    // std::cout << "part size = " << part.num_gates() << " and depth = " << part_depth.depth() << "\n";

                    auto opt = mockturtle::node_resynthesis<mockturtle::mig_network>( part, resyn_mig );
                    // std::cout << "MIG before optimization\n";
                    // opt.foreach_node([&](auto node){
                    //     std::cout << "node = " << node << "\n";
                    //     opt.foreach_fanin(node, [&](auto conn, auto i){
                    //         std::cout << "child[" << i << "] = " << conn.index << "\n";
                    //     });
                    // });
                    mockturtle::mig_script migopt;
                    opt = migopt.run(opt);
                    // std::cout << "MIG after optimization\n";
                    // opt.foreach_node([&](auto node){
                    //     std::cout << "node = " << node << "\n";
                    //     opt.foreach_fanin(node, [&](auto conn, auto i){
                    //         std::cout << "child[" << i << "] = " << conn.index << "\n";
                    //     });
                    // });
                    mockturtle::depth_view part_opt_depth{opt};
                    // std::cout << "new part size = " << opt.num_gates() << " and depth = " << part_opt_depth.depth() << "\n";

                    partitions_mig.synchronize_part(part, opt, ntk_mig);
                }

                std::cout << "MIG partitions = {";
                for(int i = 0; i < mig_parts.size(); i++){
                    std::cout << mig_parts.at(i) << " ";
                }
                std::cout << "}\n";

                partitions_mig.connect_outputs(ntk_mig);
                
                ntk_mig = mockturtle::cleanup_dangling( ntk_mig );
                mockturtle::depth_view ntk_depth2{ntk_mig};
                std::cout << "new ntk size = " << ntk_mig.num_gates() << " and depth = " << ntk_depth2.depth() << "\n";
                std::cout << "Finished optimization\n";

                mockturtle::write_verilog(ntk_mig, out_file);
                
            
            }
            else{
                std::cout << "No AIG stored\n";
            }
        }
    
    private:
        int num_parts = 0;
        std::string cnn_model{};
        std::string out_file{};
    };

    ALICE_ADD_COMMAND(test_aig_then_part, "Testing");
}