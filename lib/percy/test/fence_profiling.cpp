#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void profile(int nr_in, bool full_coverage)
{
    spec spec;

    bsat_wrapper solver;
    ssv_fence2_encoder encoder(solver);
    encoder.reset_sim_tts(nr_in);

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c;

    int64_t total_elapsed = 0;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec[0] = tt;
        //spec.verbosity = 2;
        spec.add_colex_clauses = false;
        spec.add_lex_func_clauses = false;
        spec.add_symvar_clauses = false;
        spec.add_noreapply_clauses = false;
        const auto start = std::chrono::steady_clock::now();
        const auto res2 = fence_cegar_synthesize(spec, c, solver, encoder);
        assert(res2 == success);
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
        total_elapsed += elapsed;
    }
    printf("\n");
    printf("Time elapsed: %ldus\n", total_elapsed);
}

int main()
{
#ifndef TRAVIS_BUILD
    bool full_coverage = false;
    if (full_coverage) {
        printf("Doing full equivalence check\n");
    } else {
        printf("Doing partial equivalence check\n");
    }

    profile(2, full_coverage);
    profile(3, full_coverage);
    profile(4, full_coverage);
#endif
    
    return 0;
}

