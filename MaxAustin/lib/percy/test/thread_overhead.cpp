#include <percy/percy.hpp>

#define MAX_TESTS 512ul

using namespace percy;

/*******************************************************************************
    When parallelizing synthesis based on fence or DAG structures we have the
    following problem: the thread given a valid structure may finish much
    sooner than the threads that are given unsatisfiable structures. One way to
    mitigate this problem may be to have the threads periodically check if some
    other thread has already found a solution. In this test we attempt to
    quantify the runtime overhead caused by this problem and to test this
    potential solution.
*******************************************************************************/

/*
template<int nrin>
void check_overhead(int conflict_limit)
{
    dag<2> g;
    synth_stats stats;
    spec spec;
    spec.verbosity = 0;

    // don't run too many tests.
    auto max_tests = (1ul << (1ul << nrin));
    max_tests = std::min(max_tests, MAX_TESTS);
    static_truth_table<nrin> tt;

    chain chain;

    printf("Testing %d-input overhead\n", nrin);

    double total_overhead = 0;
    double total_synth_time = 0;
    double total_time_to_first_synth = 0;
    double nr_calls = 0;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }
        auto result = qpfence_synth(&stats, tt, g, nrin, conflict_limit);
        assert(result == success);

        total_overhead += stats.overhead;
        total_synth_time += stats.total_synth_time;
        total_time_to_first_synth += stats.time_to_first_synth;
        nr_calls += 1;

        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    } 
    printf("\n");

    printf("Total time to first synth: %.2fms\n", total_time_to_first_synth);
    printf("Total synth time: %.2fms\n", total_synth_time);
    printf("Total overhead: %.2fms\n", total_overhead);
    printf("\n");

    printf("Avg. time to first synth: %.2fms\n", 
            total_time_to_first_synth/nr_calls);
    printf("Avg. synth time: %.2fms\n", total_synth_time/nr_calls);
    printf("Avg. overhead: %.2fms\n", total_overhead/nr_calls);
    printf("\n");

}
*/

int main(void)
{
    /*
    vector<int> conflict_limits = { 0, 10, 50, 100, 250, 500, 1000, 2500 };
    for (auto conflict_limit : conflict_limits) {
        printf("Setting conflict limit to %d\n", conflict_limit);
        check_overhead<2>(conflict_limit);
        check_overhead<3>(conflict_limit);
        check_overhead<4>(conflict_limit);
        check_overhead<5>(conflict_limit);
    }
    */

    return 0;
}



