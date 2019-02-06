#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;


void check_maj_equivalence(int nr_in)
{
    spec spec;

    bmcg_wrapper solver;
    maj_encoder encoder(solver);

    dynamic_truth_table tt(nr_in);
    kitty::create_majority(tt);
    spec.verbosity = 0;
    spec[0] = tt;

    mig m1, m2, m3, m4;
    auto expected_size = 0;
    switch (nr_in) {
    case 3:
        expected_size = 1;
        break;
    case 5:
        expected_size = 4;
        break;
    default:
        expected_size = 7;
        break;
    }

    auto dags = pd3_generate_max(7, nr_in);
    for (int i = 1; i <= 7; i++) {
        const auto filename = "pd" + std::to_string(i) + ".bin";
        pd3_write_nonisomorphic(i, filename.c_str());
    }

    /*
    auto start = std::chrono::steady_clock::now();
    const auto res1 = maj_pd_synthesize(spec, m1, dags, solver, encoder);
    const auto elapsed1 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start
        ).count();

    assert(res1 == success);
    assert(m1.satisfies_spec(spec));

    start = std::chrono::steady_clock::now();
    const auto res2 = maj_ser_synthesize(spec, m2, solver, encoder);
    const auto elapsed2 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start
        ).count();
    assert(res2 == success);
    assert(m2.satisfies_spec(spec));
    */

    auto start = std::chrono::steady_clock::now();
    const auto res3 = parallel_maj_synthesize(spec, m3);
    const auto elapsed3 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start
        ).count();
    assert(res3 == success);
    assert(m3.satisfies_spec(spec));

    start = std::chrono::steady_clock::now();
    const auto res4 = maj_ser_synthesize_parallel(spec, m4);
    const auto elapsed4 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start
        ).count();
    assert(res4 == success);
    assert(m4.satisfies_spec(spec));
    //assert(m1.get_nr_steps() == expected_size);
    //assert(m2.get_nr_steps() == expected_size);
    assert(m3.get_nr_steps() ==  expected_size);
    assert(m4.get_nr_steps() == expected_size);
        
    //printf("Time elapsed (PD): %ldus\n", elapsed1);
    //printf("Time elapsed (PD SER): %ldus\n", elapsed2);
    printf("Time elapsed (PARR): %ldus\n", elapsed3);
    printf("Time elapsed (PD PARR): %ldus\n", elapsed4);
}

/// Tests synthesis based on partial DAGs by comparing it to conventional
/// synthesis.  By default, does not check for full equivalence of all n-input functions.
/// Users can specify a arbitrary runtime argument, which removes the limit on
/// the number of equivalence tests.
int main()
{
#ifndef TRAVIS_BUILD
    check_maj_equivalence(3);
    check_maj_equivalence(5);
    check_maj_equivalence(7);
#endif
    
    return 0;
}

