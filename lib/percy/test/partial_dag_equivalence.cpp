#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void check_pd_equivalence(int nr_in)
{
    spec spec;

    bsat_wrapper solver;
    ssv_encoder encoder1(solver);
    partial_dag_encoder encoder2(solver);
    encoder2.reset_sim_tts(nr_in);

    spec.add_alonce_clauses = false;
    spec.add_nontriv_clauses = false;
    spec.add_lex_func_clauses = false;
    spec.add_colex_clauses = false;
    spec.add_noreapply_clauses = false;
    spec.add_symvar_clauses = false;

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    max_tests = std::min(max_tests, MAX_TESTS);
    dynamic_truth_table tt(nr_in);

    chain c1, c2, c2_cegar;

    auto dags = pd_generate_max(7);

    int64_t total_elapsed1 = 0;
    int64_t total_elapsed2 = 0;
    int64_t total_elapsed3 = 0;

    auto foo = get_encoder(solver);
    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec.add_lex_func_clauses = true;
        spec[0] = tt;
        auto start = std::chrono::steady_clock::now();
        auto res1 = synthesize(spec, c1, solver, *foo);
        const auto elapsed1 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res1 == success);
        auto sim_tts1 = c1.simulate();
        auto c1_nr_vertices = c1.get_nr_steps();
        assert(c1.satisfies_spec(spec));

        //spec.verbosity = 2;
        spec.add_lex_func_clauses = false;
        start = std::chrono::steady_clock::now();
        auto res2 = pd_synthesize(spec, c2, dags, solver, encoder2);
        const auto elapsed2 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res2 == success);
        assert(c2.satisfies_spec(spec));
        auto sim_tts2 = c2.simulate();
        auto c2_nr_vertices = c2.get_nr_steps();
        assert(c1_nr_vertices == c2_nr_vertices);
        assert(sim_tts1[0] == sim_tts2[0]);

        start = std::chrono::steady_clock::now();
        auto res3 = pd_synthesize(spec, c2_cegar, dags, solver, encoder2, SYNTH_STD_CEGAR);
        const auto elapsed3 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res3 == success);
        assert(c2_cegar.satisfies_spec(spec));
        const auto sim_tts2_cegar = c2_cegar.simulate();
        const auto c2_cegar_nr_vertices = c2_cegar.get_nr_steps();
        assert(c1_nr_vertices == c2_cegar_nr_vertices);
        assert(sim_tts1[0] == sim_tts2_cegar[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
        total_elapsed1 += elapsed1;
        total_elapsed2 += elapsed2;
        total_elapsed3 += elapsed3;
    }
    printf("\n");
    printf("Time elapsed (STD): %ldus\n", total_elapsed1);
    printf("Time elapsed (PD): %ldus\n", total_elapsed2);
    printf("Time elapsed (PD CEGAR): %ldus\n", total_elapsed3);
}

void check_pd_equivalence5()
{
    spec spec;

    bsat_wrapper solver;
    ssv_encoder encoder1(solver);
    partial_dag_encoder encoder2(solver);
    encoder2.reset_sim_tts(5);

    // don't run too many tests.
    auto max_tests = MAX_TESTS;
    dynamic_truth_table tt(5);

    chain c1, c2, c2_cegar;

    auto dags = pd_generate_max(9);

    int64_t total_elapsed1 = 0;
    int64_t total_elapsed2 = 0;
    int64_t total_elapsed3 = 0;
        
    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec.add_colex_clauses = true;
        spec.add_lex_func_clauses = true;
        spec[0] = tt;
        auto start = std::chrono::steady_clock::now();
        auto res1 = synthesize(spec, c1, solver, encoder1);
        const auto elapsed1 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        if (c1.get_nr_steps() > 9) {
            printf("Skipping!\n");
            printf("(%d/%d)\r", i + 1, max_tests);
            fflush(stdout);
            continue;
        }

        assert(res1 == success);
        auto sim_tts1 = c1.simulate();
        auto c1_nr_vertices = c1.get_nr_steps();
        assert(c1.satisfies_spec(spec));

        //spec.verbosity = 2;
        spec.add_lex_func_clauses = false;
        spec.add_colex_clauses = false;
        start = std::chrono::steady_clock::now();
        auto res2 = pd_synthesize(spec, c2, dags, solver, encoder2);
        const auto elapsed2 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res2 == success);
        assert(c2.satisfies_spec(spec));
        auto sim_tts2 = c2.simulate();
        auto c2_nr_vertices = c2.get_nr_steps();
        assert(c1_nr_vertices == c2_nr_vertices);
        assert(sim_tts1[0] == sim_tts2[0]);

        start = std::chrono::steady_clock::now();
        auto res3 = pd_synthesize(spec, c2_cegar, dags, solver, encoder2);
        const auto elapsed3 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res3 == success);
        assert(c2_cegar.satisfies_spec(spec));
        auto sim_tts3 = c2_cegar.simulate();
        auto c3_nr_vertices = c2_cegar.get_nr_steps();
        assert(c1_nr_vertices == c3_nr_vertices);
        assert(sim_tts1[0] == sim_tts3[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
        total_elapsed1 += elapsed1;
        total_elapsed2 += elapsed2;
        total_elapsed3 += elapsed3;
    }
    printf("\n");
    printf("Time elapsed (STD): %ldus\n", total_elapsed1);
    printf("Time elapsed (PD): %ldus\n", total_elapsed2);
    printf("Time elapsed (PD CEGAR): %ldus\n", total_elapsed3);
}

/// Tests synthesis based on partial DAGs by comparing it to conventional
/// synthesis.  By default, does not check for full equivalence of all n-input functions.
/// Users can specify a arbitrary runtime argument, which removes the limit on
/// the number of equivalence tests.
int main()
{
    {
        bsat_wrapper solver;
        partial_dag_encoder encoder(solver);
        kitty::static_truth_table<4> tt;
        kitty::create_from_hex_string(tt, "0357");
        spec spec;
        chain c1, c2;
        spec[0] = tt;
    
        const auto status1 = synthesize(spec, c1);
        assert(status1 == success);

        partial_dag g;
        g.reset(2, 3);
        g.set_vertex(0, 0, 0);
        g.set_vertex(1, 0, 0);
        g.set_vertex(2, 1, 2);
        const auto status = pd_synthesize(spec, c2, g, solver, encoder);
        assert(status == success);
        printf("found chain\n");
    }

    check_pd_equivalence(2);
    check_pd_equivalence(3);
    check_pd_equivalence(4);
#ifndef TRAVIS_BUILD
    check_pd_equivalence5();
#endif
    
    return 0;
}

