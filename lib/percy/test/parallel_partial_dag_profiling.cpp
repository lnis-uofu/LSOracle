#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void profile(int nr_in)
{
    spec spec;

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    max_tests = std::min(max_tests, MAX_TESTS);
    dynamic_truth_table tt(nr_in);

    chain c;

    auto dags = pd_generate_max(7);
    spec.add_lex_func_clauses = false;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);
        spec[0] = tt;
        const auto res = pd_synthesize_parallel(spec, c, dags);
        assert(res == success);
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    }
    printf("\n");
}

void profile5()
{
    spec spec;

    // don't run too many tests.
    dynamic_truth_table tt(5);

    chain c;

    auto dags = pd_generate_max(9);
    spec.add_lex_func_clauses = false;

    for (auto i = 1; i < MAX_TESTS; i++) {
        kitty::create_from_words(tt, &i, &i+1);
        spec[0] = tt;
        const auto res = pd_synthesize_parallel(spec, c, dags);
        assert(res == success);
        printf("(%d/%d)\r", i+1, MAX_TESTS);
        fflush(stdout);
    }
    printf("\n");
}

/// Tests synthesis based on partial DAGs by comparing it to conventional
/// synthesis.  By default, does not check for full equivalence of all n-input functions.
/// Users can specify a arbitrary runtime argument, which removes the limit on
/// the number of equivalence tests.
int main()
{
#ifndef TRAVIS_BUILD
    profile(2);
    profile(3);
    profile(4);
    profile5();
#endif
    
    return 0;
}

