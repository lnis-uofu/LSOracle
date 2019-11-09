#include <ot/timer/timer.hpp>

/*
 * Interfaces the command line with OpenTimer API
 */

class STA {
private:
    ot::Timer timer;

public:

    void set_lib_path(const std::string &lib){
        this->timer.read_celllib(lib,std::nullopt);
    }
    void set_netlist_path(const std::string &netlist){
        this->timer.read_verilog(netlist);
    }
    void set_sdc_path(const std::string &sdc){
        this->timer.read_sdc(sdc);
    }

    void run_slack(){
        this->timer.update_timing();                          // update timing (O(1) builder)

        if(auto tns = this->timer.report_tns(); tns) std::cout << "TNS: " << *tns << '\n';  // (O(N) action)
        if(auto wns = this->timer.report_wns(); wns) std::cout << "WNS: " << *wns << '\n';  // (O(N) action)

        this->timer.dump_timer(std::cout);                    // dump the timer details (O(1) accessor)
    }

    void run_report_timing(){
        this->timer.update_timing();                          // update timing (O(1) builder)

        std::vector<ot::Path> critical = this->timer.report_timing(1);

        //if(auto rpt = this->timer.report_timing(1); rpt) std::cout << "Critical Path: " << *rpt << '\n';  // (O(N) action)
        std::cout << "Critical Path: " << critical[0] << std::endl;

        this->timer.dump_timer(std::cout);                    // dump the timer details (O(1) accessor)
    }
};