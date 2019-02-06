#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

void profile(int nr_in)
{
    spec spec;

    // don't run too many tests.
    dynamic_truth_table tt(nr_in);

    chain c;

    for (auto i = 1; i < std::min(MAX_TESTS, (1 << 1 << nr_in)); i++) {
        kitty::create_from_words(tt, &i, &i+1);
        spec[0] = tt;
        auto res = pf_fence_synthesize(spec, c);
        assert(res == success);
        
        printf("(%d/%d)\r", i+1, MAX_TESTS);
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

    for (auto i = 1; i < MAX_TESTS; i++) {
        kitty::create_from_words(tt, &i, &i+1);
        spec[0] = tt;
        auto res = pf_fence_synthesize(spec, c);
        assert(res == success);
        printf("(%d/%d)\r", i+1, MAX_TESTS);
        fflush(stdout);
    }
    printf("\n");
}

int main()
{
    profile(2);
    profile(3);
    profile(4);
#ifndef TRAVIS_BUILD
    profile5();
#endif
    
    return 0;
}

