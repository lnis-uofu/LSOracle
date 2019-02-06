#include <percy/percy.hpp>

using namespace percy;

// Verifies that the DITT MAJ encoder works properly with PD3s.
int main(void)
{
    chain c;

    {
        std::vector<partial_dag> non_iso_dags;
        const auto nr_steps = 6;
        const auto nr_in = 7;
        printf("Generating %d-step >=%d-PI PDs...\n", nr_steps, nr_in);
        {
            const auto dags = pd3_exact_generate_filtered(nr_steps, nr_in);
            printf("Generated %zu potentially isomorphic PDs...\n", dags.size());
            printf("Filtering out isomorphic PDs...\n");
            pd_filter_isomorphic_sfast(dags, non_iso_dags, true);
        }
        printf("Generated %zu non-isomorphic PDs...\n", non_iso_dags.size());


        const auto result = pd_ditt_maj_synthesize(nr_in, non_iso_dags, c, true);
        assert(result == failure);
    }

    std::vector<partial_dag> non_iso_dags;

    const auto nr_steps = 7;
    const auto nr_in = 7;
    printf("Generating %d-step >=%d-PI PDs...\n", nr_steps, nr_in);
    {
        const auto dags = pd3_exact_generate_filtered(nr_steps, nr_in);
        printf("Generated %zu potentially isomorphic PDs...\n", dags.size());
        printf("Filtering out isomorphic PDs...\n");
        pd_filter_isomorphic_sfast(dags, non_iso_dags, true);
    }
    printf("Generated %zu non-isomorphic PDs...\n", non_iso_dags.size());

    const auto result = pd_ditt_maj_synthesize(nr_in, non_iso_dags, c, true);
    assert(result == success);
    const auto chain_func = c.simulate()[0];
    kitty::dynamic_truth_table tt(7);
    kitty::create_majority(tt);
    assert(chain_func == tt);
    printf("Found chain:\n");
    c.to_mag_expression(std::cout);
    printf("\n");


    return 0;
}
        
