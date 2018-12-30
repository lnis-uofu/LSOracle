#include <percy/percy.hpp>
#include <cassert>
#include <cstdio>
#include <vector>

using namespace percy;
using std::vector;


/*******************************************************************************
    Counts and prints all fences up to and including F_5 and ensures that the
    number is correct.
*******************************************************************************/
int main(void)
{
    fence f;
    auto total_expected_fences = 0u;
    for (unsigned k = 1; k <= 5; k++) {
        printf("F_%d\n", k);
        for (unsigned l = 1; l <= k; l++) {
            printf("F(%d, %d)\n", k, l);
            partition_generator g(k, l);

            auto nfences = 0u;
            while (g.next_fence(f)) {
                nfences++;
                print_fence(f);
                printf("\n");
            }
            const auto expected_fences = binomial_coeff(k-1, l-1);
            assert(nfences == expected_fences);
            total_expected_fences += nfences;
        }
        auto nfences = 0u;
        family_generator g(k);
        auto expected_fences = 0u;
        for (int l = 1; l <= k; l++) {
            expected_fences += binomial_coeff(k-1, l-1);
        }
        while (g.next_fence(f)) {
            nfences++;
        }
        assert(nfences == (unsigned)expected_fences);
    }
    unbounded_generator g;
    auto nfences = 0u;
    while (true) {
        g.next_fence(f);
        if (g.get_nnodes() >= 6) {
            break;
        }
        nfences++;
    }
    assert(nfences == total_expected_fences);

    rec_fence_generator recgen;
    recgen.set_po_filter(false);
    for (unsigned k = 1; k <= 5; k++) {
        printf("F_%d\n", k);
        auto total_nr_fences = 0;
        for (unsigned l = 1; l <= k; l++) {
            printf("F(%u, %u)\n", k, l);
            recgen.reset(k, l);

            vector<fence> fences;
            recgen.generate_fences(fences);
            const auto nfences = fences.size();
            for (auto& f : fences) {
                print_fence(f);
                printf("\n");
            }

            const auto expected_fences = binomial_coeff(k-1, l-1);
            assert(nfences == expected_fences);
            total_nr_fences += nfences;
        }
        auto fences = generate_fences(k, false);
        assert(fences.size() == total_nr_fences);
    }

    // Count the maximum number of fences needed to synthesize all 5-input
    // functions.
    auto global_total = 0;
    recgen.set_po_filter(true);
    vector<fence> po_fences;
    for (unsigned k = 1; k <= 12; k++) {
        auto total_nr_fences = 0;
        for (unsigned l = 1; l <= k; l++) {
            recgen.reset(k, l);
            total_nr_fences += recgen.count_fences();
        }
        generate_fences(po_fences, k);
        global_total += total_nr_fences;
        printf("Number of fences in F_%d = %d\n", k, total_nr_fences);
    }
    printf("Nr. of fences relevant to 5-input single-output synthesis is %d\n",
            global_total);
    assert(po_fences.size() == global_total);

    return 0;
}

