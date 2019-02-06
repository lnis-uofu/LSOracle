#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void profile(int nr_in, bool full_coverage, const std::vector<partial_dag>& dags)
{
    spec spec;

    bsat_wrapper solver;
    partial_dag_encoder encoder(solver);
    encoder.reset_sim_tts(nr_in);

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c1, c2;

    int64_t total_elapsed1 = 0;
    int64_t total_elapsed2 = 0;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec[0] = tt;
        spec.add_colex_clauses = false;
        spec.add_lex_func_clauses = false;
        spec.add_symvar_clauses = false;
        spec.add_noreapply_clauses = false;
        
        auto start = std::chrono::steady_clock::now();
        const auto res1 = pd_synthesize(spec, c1, dags, solver, encoder);
        assert(res1 == success);
        auto elapsed1 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();

        start = std::chrono::steady_clock::now();
        const auto res2 = pd_synthesize(spec, c2, dags, solver, encoder, SYNTH_STD_CEGAR);
        assert(res2 == success);
        auto elapsed2 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();

        assert(c1.get_nr_steps() == c2.get_nr_steps());
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
        total_elapsed1 += elapsed1;
        total_elapsed2 += elapsed2;
    }
    printf("\n");
    printf("Time elapsed: %ldus\n", total_elapsed1);
    printf("Time elapsed (CEGAR): %ldus\n", total_elapsed2);
}

void profile5(const std::vector<partial_dag>& dags)
{
    spec spec;

    bsat_wrapper solver;
    ssv_encoder encoder(solver);
    partial_dag_encoder pd_encoder(solver);
    pd_encoder.reset_sim_tts(5);

    // don't run too many tests.
    dynamic_truth_table tt(5);

    chain c1, c2, c3;

    int64_t total_elapsed1 = 0;
    int64_t total_elapsed2 = 0;
    spec.conflict_limit = 100000;

    for (auto i = 1; i < MAX_TESTS; i++) {
        kitty::create_from_words(tt, &i, &i+1);
        spec[0] = tt;
        
        spec.add_colex_clauses = true;
        spec.add_lex_func_clauses = true;
        spec.add_symvar_clauses = true;
        spec.add_noreapply_clauses = true;
        const auto res3 = synthesize(spec, c3,solver, encoder);
        if (res3 == timeout) {
            continue;
        }

        spec.add_colex_clauses = false;
        spec.add_lex_func_clauses = false;
        spec.add_symvar_clauses = false;
        spec.add_noreapply_clauses = false;
        
        auto start = std::chrono::steady_clock::now();
        const auto res1 = pd_synthesize(spec, c1, dags, solver, pd_encoder);
        assert(res1 == success);
        auto elapsed1 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();

        start = std::chrono::steady_clock::now();
        const auto res2 = pd_synthesize(spec, c2, dags, solver, pd_encoder, SYNTH_STD_CEGAR);
        assert(res2 == success);
        auto elapsed2 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();

        assert(c1.get_nr_steps() == c2.get_nr_steps());
        assert(c3.get_nr_steps() == c2.get_nr_steps());
        
        printf("(%d/%d)\r", i+1, MAX_TESTS);
        fflush(stdout);
        total_elapsed1 += elapsed1;
        total_elapsed2 += elapsed2;
    }
    printf("\n");
    printf("Time elapsed: %ldus\n", total_elapsed1);
    printf("Time elapsed (CEGAR): %ldus\n", total_elapsed2);
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

    const auto dags = pd_generate_max(7);

    profile(2, full_coverage, dags);
    profile(3, full_coverage, dags);
    profile(4, full_coverage, dags);
    profile5(dags);
#endif
    
    return 0;
}

