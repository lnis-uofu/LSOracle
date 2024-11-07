template< typename network>
class abc_optimizer: public aig_optimizer<network>
{
public:
    abc_optimizer(partition_manager_junior<network> &partman, mockturtle::names_view<network> &ntk, int index, optimization_strategy target, const std::string &abc_exec): aig_optimizer<network>(partman, ntk, index, target, abc_exec) {}

    const std::string optimizer_name()
    {
        return "abc resyn2";
    }

    optimizer<mockturtle::xmg_network> *reapply(partition_manager_junior<mockturtle::xmg_network> &partman, xmg_names &ntk)
    {
        return new abc_optimizer<mockturtle::xmg_network>(partman, ntk, this->index, this->strategy, this->abc_exec);
    }
    void optimize()
    {
        char *blif_name_char = strdup("/tmp/lsoracle_XXXXXX.blif");
        if (mkstemps(blif_name_char, 5) == -1) {
            throw std::exception();
        }
        std::string blif_name = std::string(blif_name_char);
        std::cout << "writing blif to " << blif_name  << std::endl;

        char *blif_output_name_char = strdup("/tmp/lsoracle_XXXXXX_optimized.blif");
        if (mkstemps(blif_output_name_char, 15) == -1) {
            throw std::exception();
        }
        std::string blif_output_name = std::string(blif_output_name_char);
        std::cout << "writing abc output to " << blif_output_name  << std::endl;

        mockturtle::write_blif_params ps;
        ps.skip_feedthrough = 1u;
        mockturtle::write_blif(this->converted, blif_name, ps);
        std::string script = "abc -c \"read_blif " + blif_name + "; resyn2; write_blif " + blif_output_name + " \"";
        int code = system((script).c_str());
        assert(code == 0);
        std::cout << "optimized with abc" << std::endl;

        mockturtle::names_view<mockturtle::klut_network> klut;
        auto const result = lorina::read_blif(blif_name,
                                              mockturtle::blif_reader(klut));

        lorina::return_code read_blif_return_code = lorina::read_blif(blif_output_name, mockturtle::blif_reader(klut));
        assert(read_blif_return_code == lorina::return_code::success);
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
        mockturtle::node_resynthesis(this->optimal, klut, resyn);
    }
};
