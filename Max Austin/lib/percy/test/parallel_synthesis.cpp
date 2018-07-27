#include <percy/percy.hpp>

#define MAX_TESTS 512

using namespace percy;


/*******************************************************************************
    Tests if parallel DAG-based synthesis works correctly.

void check_equivalence(int nr_in)
{
    dag<2> g;
    unbounded_dag_generator<sat_solver*> ugen;

    synth_stats stats;
    spec spec;

    auto synth1 = new_std_synth();
    auto synth2 = new_dag_synth();

    spec.verbosity = 4;

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    max_tests = std::min(max_tests, MAX_TESTS);
    static_truth_table<nr_in> tt;

    chain<2> c1;
    chain<2> c2;

    printf("Testing %d-input equivalence\n", nr_in);

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }
        spec.functions[0] = &tt;
        auto res1 = synth1->synthesize(spec, c1);
        assert(res1 == success);
        auto sim_tts1 = c1.template simulate(spec);
        auto c1_nr_steps = c1.get_nr_vertices();

        auto res2 = qpfence_synth(&stats, tt, g, nr_in, 0);
        assert(res2 == success);
        // Make sure that the found DAG is indeed valid for this function.
        auto verify_res = synth2->synthesize(spec, g, c2);
        assert(verify_res == success);
        auto sim_tts2 = c2.template simulate(spec);
        auto c2_nr_steps = c2.get_nr_vertices();

        assert(sim_tts1[0] == sim_tts2[0]);
        assert(c1_nr_steps == c2_nr_steps);
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    } 
    printf("\n");
}
*******************************************************************************/


int main(void)
{
//    check_equivalence<2>();
//   check_equivalence<3>();
//    check_equivalence<4>();

    return 0;
}

