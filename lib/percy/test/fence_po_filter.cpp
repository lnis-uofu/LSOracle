#include <percy/fence.hpp>
#include <cassert>
#include <cstdio>

using namespace percy;

/*******************************************************************************
    Tests the PO and operator fanin count filter in various configurations.
*******************************************************************************/
int main(void)
{
    // Counts and prints all fences in F_5 that are relevant for 1-output
    // functions with 2-input operators.
    unsigned k = 5;
    unsigned nfences = 0;
    printf("F_%d\n", k);
    fence f;
    rec_fence_generator gen;
    auto rec_nfences = 0;
    for (unsigned l = 1; l <= k; l++) {
        printf("F(%d, %d)\n", k, l);
        po_filter<partition_generator> g(partition_generator(k, l), 1, 2);

        while (g.next_fence(f)) {
            nfences++;
            print_fence(f);
            printf("\n");
        }
        
        gen.reset(k, l);
        rec_nfences += gen.count_fences();
    }
    auto expected_fences = 6;
    assert(nfences == (unsigned)expected_fences);
    assert(rec_nfences == (unsigned)expected_fences);

    // Counts and prints all fences in F_5 that are relevant for 2-output
    // functions with 2-input operators.
    nfences = 0;
    rec_nfences = 0;
    printf("F_%d\n", k);
    for (unsigned l = 1; l <= k; l++) {
        printf("F(%d, %d)\n", k, l);
        po_filter<partition_generator> g(partition_generator(k, l), 2, 2);
        
        while (g.next_fence(f)) {
            nfences++;
            print_fence(f);
            printf("\n");
        }
        gen.reset(k, l, 2);
        rec_nfences += gen.count_fences();
    }
    expected_fences = 10;
    assert(nfences == (unsigned)expected_fences);
    assert(rec_nfences == (unsigned)expected_fences);

    // Counts and prints all fences in F_5 that are relevant for 1-output
    // functions with 3-input operators.
    nfences = 0;
    rec_nfences = 0;
    printf("F_%d\n", k);
    for (unsigned l = 1; l <= k; l++) {
        printf("F(%d, %d)\n", k, l);
        po_filter<partition_generator> g(partition_generator(k, l), 1, 3);

        while (g.next_fence(f)) {
            nfences++;
            print_fence(f);
            printf("\n");
        }
        gen.reset(k, l, 1, 3);
        rec_nfences += gen.count_fences();
    }
    expected_fences = 7;
    assert(nfences == (unsigned)expected_fences);
    assert(rec_nfences == (unsigned)expected_fences);

    return 0;
}

