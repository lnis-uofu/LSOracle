#include <cstdio>
#include <percy/percy.hpp>
#include <ctime>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void bench(int nr_in, bool full_coverage)
{
    spec spec;

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c2;

    auto dags = pd_generate_max(7);

    auto total_elapsed2 = 0;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec[0] = tt;
     
        const auto start = std::clock();
        auto res2 = pd_synthesize_enum(spec, c2, dags);
        auto elapsed2 = std::clock() - start;
        assert(res2 == success);
        auto sim_tts2 = c2.simulate();
        assert(sim_tts2[0] == spec[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
        total_elapsed2 += elapsed2;
    }
    printf("\n");
    auto total_elapsed2_ms = 1000 * double(total_elapsed2) / CLOCKS_PER_SEC;
    printf("Time elapsed (PD): %.2fms\n", total_elapsed2_ms);
}

/// Tests synthesis based on partial DAGs by comparing it to conventional
/// synthesis.  By default, does not check for full equivalence of all n-input functions.
/// Users can specify a arbitrary runtime argument, which removes the limit on
/// the number of equivalence tests.
int main()
{
    bool full_coverage = false;
    if (full_coverage) {
        printf("Doing full equivalence check\n");
    } else {
        printf("Doing partial equivalence check\n");
    }

#ifndef TRAVIS_BUILD
    bench(2, full_coverage);
    bench(3, full_coverage);
    bench(4, full_coverage);
#endif
    //check_pd_equivalence5();
    
    return 0;
}

