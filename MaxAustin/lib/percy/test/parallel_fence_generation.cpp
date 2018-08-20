#include <percy/percy.hpp>
#include <cassert>
#include <cstdio>

using namespace percy;
using std::vector;

/*******************************************************************************
    Verifies that fence generation using a concurrent queue works the same as
    sequential generation.
*******************************************************************************/
int main(void)
{
    fence f;
    vector<fence> v;
    moodycamel::ConcurrentQueue<fence> q(2048);
    spec spec;

    for (int k = 1; k <= 12; k++) {
        spec.nr_steps = k;
        auto v = generate_fences(k);
        generate_fences(spec, q);
        auto qfences = 0;
        while (q.try_dequeue(f)) {
            ++qfences;
        }
        assert(qfences == v.size());
        printf("# fences = %d\n", qfences);
    }
}

