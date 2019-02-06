#include <cstdio>
#include <percy/percy.hpp>
#include <ctime>

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

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    max_tests = std::min(max_tests, MAX_TESTS);
    dynamic_truth_table tt(nr_in);

    chain c1, c2;

    auto dags = pd_generate_max(7);

    auto total_elapsed1 = 0;
    auto total_elapsed2 = 0;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec.add_colex_clauses = true;
        spec.add_lex_func_clauses = true;
        spec[0] = tt;
        auto start = std::clock();
        auto res1 = synthesize(spec, c1, solver, encoder1, SYNTH_STD_CEGAR);
        auto elapsed1 = std::clock() - start;
        assert(res1 == success);
        auto sim_tts1 = c1.simulate();
        auto c1_nr_vertices = c1.get_nr_steps();
        assert(c1.satisfies_spec(spec));

        //spec.verbosity = 2;
        spec.add_colex_clauses = false;
        spec.add_lex_func_clauses = false;
        start = std::clock();
        auto res2 = pd_synthesize_enum(spec, c2, dags);
        auto elapsed2 = std::clock() - start;
        assert(res2 == success);
        assert(c2.satisfies_spec(spec));
        auto sim_tts2 = c2.simulate();
        auto c2_nr_vertices = c2.get_nr_steps();
        assert(c1_nr_vertices == c2_nr_vertices);
        assert(sim_tts1[0] == sim_tts2[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
        total_elapsed1 += elapsed1;
        total_elapsed2 += elapsed2;
    }
    printf("\n");
    auto total_elapsed1_ms = 1000 * double(total_elapsed1) / CLOCKS_PER_SEC;
    auto total_elapsed2_ms = 1000 * double(total_elapsed2) / CLOCKS_PER_SEC;
    printf("Time elapsed (STD): %.2fms\n", total_elapsed1_ms);
    printf("Time elapsed (PD): %.2fms\n", total_elapsed2_ms);
}

void check_pd_equivalence5()
{
    spec spec;

    bsat_wrapper solver;
    ssv_encoder encoder1(solver);

    // don't run too many tests.
    auto max_tests = MAX_TESTS;
    dynamic_truth_table tt(5);

    chain c1, c2;

    auto dags = pd_generate_max(7);

    auto total_elapsed1 = 0;
    auto total_elapsed2 = 0;
        
    spec.conflict_limit = 1000;
    auto nr_instances = 0;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec.verbosity = 0;
        spec.add_colex_clauses = true;
        spec.add_lex_func_clauses = true;
        spec[0] = tt;
        auto start = std::clock();
        auto res1 = synthesize(spec, c1, solver, encoder1);
        auto elapsed1 = std::clock() - start;
        if (res1 == timeout) {
            printf("(%d/%d)\r", i + 1, max_tests);
            fflush(stdout);
            continue;
        }
        if (c1.get_nr_steps() > 7) {
            printf("(%d/%d)\r", i + 1, max_tests);
            fflush(stdout);
            continue;
        }
        nr_instances++;


        assert(res1 == success);
        auto sim_tts1 = c1.simulate();
        auto c1_nr_vertices = c1.get_nr_steps();
        assert(c1.satisfies_spec(spec));

        //spec.verbosity = 2;
        spec.add_colex_clauses = false;
        spec.add_lex_func_clauses = false;
        start = std::clock();
        auto res2 = pd_synthesize_enum(spec, c2, dags);
        auto elapsed2 = std::clock() - start;
        assert(res2 == success);
        assert(c2.satisfies_spec(spec));
        auto sim_tts2 = c2.simulate();
        auto c2_nr_vertices = c2.get_nr_steps();
        assert(c1_nr_vertices == c2_nr_vertices);
        assert(sim_tts1[0] == sim_tts2[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
        total_elapsed1 += elapsed1;
        total_elapsed2 += elapsed2;
    }
    printf("\n");
    auto total_elapsed1_ms = 1000 * double(total_elapsed1) / CLOCKS_PER_SEC;
    auto total_elapsed2_ms = 1000 * double(total_elapsed2) / CLOCKS_PER_SEC;
    printf("total instances synthesized: %d\n", nr_instances);
    printf("Time elapsed (STD): %.2fms\n", total_elapsed1_ms);
    printf("Time elapsed (PD): %.2fms\n", total_elapsed2_ms);
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
    }

    check_pd_equivalence(2);
    check_pd_equivalence(3);
    //check_pd_equivalence(4, 2, full_coverage);
#ifndef TRAVIS_BUILD
    check_pd_equivalence5();
#endif
    
    return 0;
}

