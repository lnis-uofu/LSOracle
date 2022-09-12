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
#include <string>
#include <mockturtle/mockturtle.hpp>
#include <sta/Sta.hh>
#include <sta/ConcreteNetwork.hh>
#include <sta/Corner.hh>
#include <sta/Graph.hh>
#include <sta/Liberty.hh>
#include <sta/Network.hh>
#include <sta/PathRef.hh>
#include <sta/PortDirection.hh>
#include <sta/TimingArc.hh>
#include <sta/PatternMatch.hh>
#include <sta/VerilogReader.hh>
#include <sta/StaMain.hh>
#include <tcl.h>
// #include "algorithms/optimization/optimizers/strategy.hpp"
// #include "algorithms/optimization/optimizers/aig.hpp"
// #include "algorithms/optimization/optimizers/mig.hpp"
// #include "algorithms/optimization/optimizers/xag.hpp"
// #include "algorithms/optimization/optimizers/xmg.hpp"
// #include "algorithms/optimization/optimizers/abc.hpp"

namespace oracle {

enum optimization_strategy { size, balanced, depth };

struct node_depth {
    int nodes;
    int depth;
};

struct area_delay {
    double area;
    double delay;
};
    
template<typename network>
class optimizer
{
public:
    /**
     * human readable name
     */
    virtual const std::string optimizer_name() = 0;
    /**
     * Do any conversion necessary from original network type to the internal network type.
     */
    virtual void convert() = 0;
    /**
     * Perform optimization
     */
    virtual void optimize() = 0;
     /**
     * Calculate tech independent depth and nodes metrics.
     */
    virtual node_depth independent_metric() = 0;
    /**
     * List the type of optimization: area, delay, or balanced.
     */
    virtual optimization_strategy target() = 0;
    /**
     * Techmap, then return a path to a file containing the techmapped verilog.
     */
    virtual std::string techmap(const std::string &liberty_file, const std::string &temp_prefix) = 0;
    /**
     * convert the network to the superset.
     */
    virtual mockturtle::names_view<mockturtle::xmg_network> export_superset() = 0;
    /**
     * Reapply this optimization to a different network.
     */
    virtual optimizer<mockturtle::xmg_network> *reapply(int index, const mockturtle::window_view<mockturtle::names_view<mockturtle::xmg_network>> &part) = 0;
    virtual std::string get_network_name() = 0;
};

template <typename T>
class optimization_strategy_comparator {
public:
    // Comparator function
    virtual bool operator()(optimizer<T> &a, optimizer<T> &b) = 0;
    virtual const std::string name() = 0;
};

template <typename network>
optimizer<network> *optimize(optimization_strategy_comparator<network> &comparator,
                             optimization_strategy strategy,
                             const mockturtle::window_view<mockturtle::names_view<network>> &part,
                             int index,
                             const std::string &abc_exec,
                             const std::string &temp_prefix);

template <typename T>
class node_depth_strategy : public optimization_strategy_comparator<T>
{
    bool operator()(optimizer<T> &a, optimizer<T> &b)
    {
        node_depth x = a.independent_metric();
        node_depth y = b.independent_metric();

        return x.nodes * x.depth < y.nodes * y.depth;
    }
    const std::string name()
    {
        return "node-depth product";
    }
};

template <typename T>
class depth_strategy : public optimization_strategy_comparator<T>
{
    bool operator()(optimizer<T> &a, optimizer<T> &b)
    {
        node_depth x = a.independent_metric();
        node_depth y = b.independent_metric();

        return x.depth < y.depth;
    }
    const std::string name()
    {
        return "depth";
    }
};

 template <typename T>
 class node_strategy : public optimization_strategy_comparator<T>
 {
     bool operator()(optimizer<T> &a, optimizer<T> &b)
     {
         node_depth x = a.independent_metric();
         node_depth y = b.independent_metric();

         return x.nodes < y.nodes;
     }
     const std::string name()
     {
         return "node";
     }
};

template <typename T>
class tech_strategy : public optimization_strategy_comparator<T>
{
public:
    tech_strategy(const std::string &liberty_file, const std::string &sdc_file, const std::string &temp_prefix): liberty_file(liberty_file), sdc_file(sdc_file), temp_prefix(temp_prefix) {
    }

    bool operator()(optimizer<T> &a, optimizer<T> &b)
    {
        sta::Corner *corner = new sta::Corner("tt", 0);
        sta::MinMaxAll *minmax = sta::MinMaxAll::all();
        sta::LibertyLibrary *lib = sta::Sta::sta()->readLiberty(liberty_file.c_str(),
                                                                corner,
                                                                minmax,
                                                                true);

        const std::string at = a.techmap(liberty_file, temp_prefix);
        const std::string bt = b.techmap(liberty_file, temp_prefix);
        std::string model = a.get_network_name();
        area_delay x = run_timing(lib, at, "picorv32/combinational.sdc", model);
        area_delay y = run_timing(lib, bt, "picorv32/combinational.sdc", model);
        return compare(x,y);
    }
private:
    area_delay run_timing(sta::LibertyLibrary *lib,
                          const std::string &verilog_file, 
                          const std::string &sdc_file,
                          const std::string design) {
        std::cout << "Reading " << design << " from " << verilog_file << std::endl;
        bool read_ver = sta::readVerilogFile(verilog_file.c_str(),
                                             sta::Sta::sta()->networkReader());
        assert(read_ver); // << "failed to read verilog";
        bool linked = sta::Sta::sta()->linkDesign(design.c_str());
        assert(linked); // << "Failed to link";
        std::cout << "Attempting to read sdc " << sdc_file << std::endl;
        std::string read_sdc = "sta::read_sdc " + sdc_file;
        int a = Tcl_Eval(sta::Sta::sta()->tclInterp(), read_sdc.c_str());
        assert(a == 0);
        int b = Tcl_Eval(sta::Sta::sta()->tclInterp(), "sta::report_checks > /tmp/test.monkey");
        assert(b == 0);

        std::cout << "running timing" << std::endl;
        sta::Slack worst_slack;
        sta::Vertex *vertex;
        sta::Sta::sta()->worstSlack(sta::MinMax::max(), worst_slack, vertex);
        sta::PathRef worst_path_arrival;
        sta::PathRef worst_path_slack;

        sta::Sta::sta()->vertexWorstArrivalPath(vertex, sta::MinMax::max(), worst_path_arrival);
        sta::Sta::sta()->vertexWorstSlackPath(vertex, sta::MinMax::max(), worst_path_slack);

        sta::ConcreteNetwork *net = reinterpret_cast<sta::ConcreteNetwork*>(sta::Sta::sta()->networkReader());
        sta::ConcreteInstance *top = reinterpret_cast<sta::ConcreteInstance*>
            (net->topInstance());
        sta::CellSeq cells;
        sta::PatternMatch *pattern = new sta::PatternMatch("**", false, false, nullptr);
        net->findCellsMatching(reinterpret_cast<const sta::Library*>(lib), pattern, &cells);
        std::cout << "Number of cells " << cells.size() << std::endl;
        double area = 0;
        for (sta::Cell *cell: cells) {
            sta::ConcreteCell *ce = reinterpret_cast<sta::ConcreteCell*>(cell);
            sta::LibertyCell *c = ce->libertyCell();
            area += c->area();
        }
        std::cout << "Area " << area << std::endl;

        sta::Arrival arrival = worst_path_slack.arrival(sta::Sta::sta());
        // net->clear();
        // net->deleteTopInstance();
        std::cout << "Arrival " << arrival << std::endl;
        return {
            area, arrival
        };
    }    

    virtual bool compare(const area_delay &a, const area_delay &b) = 0;
    std::string liberty_file;
    std::string sdc_file;
    std::string temp_prefix;
};

template <typename T>
class area_delay_strategy : public tech_strategy<T>
{
public:
    area_delay_strategy(const std::string &liberty_file, const std::string &sdc_file, const std::string &temp_prefix): tech_strategy<T>(liberty_file, sdc_file, temp_prefix) {}
    const std::string name()
    {
        return "area-delay product";
    }
private:
    bool compare(const area_delay &a, const area_delay &b)
    {
        return a.delay * a.area < b.delay * b.area;
    }
};

template <typename T>
class delay_strategy : public tech_strategy<T>
{
public:
    delay_strategy(const std::string &liberty_file, const std::string &sdc_file, const std::string &temp_prefix): tech_strategy<T>(liberty_file, sdc_file, temp_prefix) {}
    const std::string name()
    {
        return "depth";
    }
private:
    bool compare(const area_delay &a, const area_delay &b)
    {
        return a.delay < b.delay;
    }

};

template <typename T>
class area_strategy : public tech_strategy<T>
{
public:
    area_strategy(const std::string &liberty_file, const std::string &sdc_file, const std::string &temp_prefix): tech_strategy<T>(liberty_file, sdc_file, temp_prefix) {}
    const std::string name()
    {
        return "area";
    }
private:
    bool compare(const area_delay &a, const area_delay &b)
    {
        return a.area < b.area;
    }
};

    
};
