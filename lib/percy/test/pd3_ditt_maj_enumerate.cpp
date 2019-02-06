#include <percy/percy.hpp>

using namespace percy;

// Enumerates DITT MAJ solutions.
int main(void)
{
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

        const auto num_solutions = pd_ditt_maj_enumerate(nr_in, non_iso_dags, true);
        assert(num_solutions == 0);
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

    const auto num_solutions = pd_ditt_maj_enumerate(nr_in, non_iso_dags, true);
    assert(num_solutions > 0);

    return 0;
}
        
