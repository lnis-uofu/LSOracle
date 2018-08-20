#include <cstdio>
#include <vector>
#include <percy/percy.hpp>
#include <kitty/kitty.hpp>
#include <ctime>

#define NR_IN 5
#define NR_FUNCS 2

using namespace percy;
using kitty::dynamic_truth_table;
using std::vector;

/*******************************************************************************
    Verifies that our timeouts work correctly.
*******************************************************************************/
void check_timeout(int nr_in, dynamic_truth_table& tt, int conflict_limit, vector<double>& times)
{
    spec spec;
    spec.conflict_limit = conflict_limit;
    spec[0] = tt;
    spec.verbosity = 0;
    chain chain;
    
    auto start = std::clock();
    auto res = synthesize(spec, chain, SLV_BSAT2, ENC_FENCE, SYNTH_FENCE);
    auto elapsed = std::clock()-start;
    times.push_back(elapsed / (double) CLOCKS_PER_SEC);
    assert(res == timeout);
    //assert(solver_nr_conflicts(spec.solver()) >= conflict_limit);
}

int main(void)
{
    dynamic_truth_table tt(5);
    vector<double> times;
    for (int i = 0; i < NR_FUNCS; i++) {
        kitty::create_random(tt);
        check_timeout(NR_IN, tt, 100000, times);
    }

    for (int i = 0; i < NR_FUNCS; i++) {
        printf("time[%d] = %fs\n", i, times[i]);
    }
    
    auto tot = std::accumulate(times.rbegin(), times.rend(), 0.0);
    printf("Total elapsed time = %fs\n", tot);
    auto mean = tot / NR_FUNCS;
    printf("Mean time per function = %fs\n", mean);
    double sq_sum = std::inner_product(times.begin(), times.end(), times.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / NR_FUNCS - mean * mean);
    printf("Standard deviation = %fs\n", stdev);

    return 0;
}

