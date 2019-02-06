#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
template<int nr_in>
void check_pd_equivalence()
{
    spec spec;
    using truth_table = kitty::static_truth_table<nr_in>;
    kitty::dynamic_truth_table map(truth_table::NumBits);
    std::transform(map.cbegin(), map.cend(), map.begin(), [](auto word) { return ~word; });
    std::unordered_set<truth_table, kitty::hash<truth_table>> classes;
    int64_t index = 0;
    truth_table tt;

    while (index != -1) {
        kitty::create_from_words(tt, &index, &index + 1);
        const auto res = kitty::exact_npn_canonization(tt, [&map](const auto& tt) { kitty::clear_bit(map, *tt.cbegin()); });
        classes.insert(std::get<0>(res));
        index = find_first_one_bit(map);
    }

    std::cout << "[i] enumerated "
        << map.num_bits() << " functions into "
        << classes.size() << " classes." << std::endl;


    bsat_wrapper solver;
    ssv_encoder encoder1(solver);
    partial_dag_encoder encoder2(solver);
    encoder2.reset_sim_tts(nr_in);

    chain c1, c2, c3;

    auto dags = pd_generate_max(7);

    int64_t total_elapsed1 = 0;
    int64_t total_elapsed2 = 0;
    int64_t total_elapsed3 = 0;

    int ctr = 0;
    for (const auto& tt : classes) {
        spec.verbosity = 0;
        spec.add_lex_func_clauses = true;
        spec.add_colex_clauses = true;
        spec[0] = tt;
        auto start = std::chrono::steady_clock::now();
        auto res1 = synthesize(spec, c1, solver, encoder1, SYNTH_STD_CEGAR);
        const auto elapsed1 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
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
        auto res3 = pd_ser_synthesize(spec, c3, solver, encoder2, "../../test/");
        const auto elapsed3 = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res3 == success);
        assert(c3.satisfies_spec(spec));
        const auto sim_tts3 = c3.simulate();
        const auto c3_nr_vertices = c3.get_nr_steps();
        assert(c1_nr_vertices == c3_nr_vertices);
        assert(sim_tts1[0] == sim_tts3[0]);
        
        printf("(%d/%zu)\r", ++ctr, classes.size());
        fflush(stdout);
        total_elapsed1 += elapsed1;
        total_elapsed2 += elapsed2;
        total_elapsed3 += elapsed3;
    }
    printf("\n");
    printf("Time elapsed (STD): %ldus\n", total_elapsed1);
    printf("Time elapsed (PD): %ldus\n", total_elapsed2);
    printf("Time elapsed (PD SER): %ldus\n", total_elapsed3);
}

/// Tests synthesis based on partial DAGs by comparing it to conventional
/// synthesis.  By default, does not check for full equivalence of all n-input functions.
/// Users can specify a arbitrary runtime argument, which removes the limit on
/// the number of equivalence tests.
int main()
{
#ifndef TRAVIS_BUILD
    check_pd_equivalence<3>();
    check_pd_equivalence<4>();
#endif
    
    return 0;
}

