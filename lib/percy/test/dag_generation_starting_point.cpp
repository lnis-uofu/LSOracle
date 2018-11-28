#include <percy/percy.hpp>
#include <cstdio>
#include <fstream>
#include <ctime>

using namespace percy;

/*******************************************************************************
    We want to be able to start the backtrack DAG generator at arbitrary points
    in the search tree. In order to do that we allow users to specify what
    the search stack currently looks like and to which level the search should
    backtrack.
*******************************************************************************/
int main(void)
{


    // Start the search by initializing the stack with the 3 possible options
    // for the first step: (0, 1), (0, 2), and (1,2). The total number of
    // solutions found should add up correctly.
    {
        rec_dag_generator rec_gen, rec_gen1, rec_gen2, rec_gen3, rec_gen4,
                          rec_gen5, rec_gen6;

        {
            rec_gen.reset(3, 8);
            rec_gen1.reset(3, 8);
            rec_gen2.reset(3, 8);
            rec_gen3.reset(3, 8);

            rec_gen1.add_selection(0, 1);
            rec_gen2.add_selection(0, 2);
            rec_gen3.add_selection(1, 2);

            auto nr_dags = rec_gen.count_dags();
            auto nr_dags1 = rec_gen1.count_dags();
            auto nr_dags2 = rec_gen2.count_dags();
            auto nr_dags3 = rec_gen3.count_dags();

            printf("nr_dags=%d\n", nr_dags);
            printf("nr_dags1=%d\n", nr_dags1);
            printf("nr_dags2=%d\n", nr_dags2);
            printf("nr_dags3=%d\n", nr_dags3);
            assert(nr_dags == (nr_dags1 + nr_dags2 + nr_dags3));
            printf("\n");
        }
        
        {
            rec_gen.reset(4, 7);
            rec_gen1.reset(4, 7);
            rec_gen2.reset(4, 7);
            rec_gen3.reset(4, 7);
            rec_gen4.reset(4, 7);
            rec_gen5.reset(4, 7);
            rec_gen6.reset(4, 7);

            rec_gen1.verbosity(0);
            rec_gen2.verbosity(0);
            rec_gen3.verbosity(0);
            rec_gen4.verbosity(0);
            rec_gen5.verbosity(0);
            rec_gen6.verbosity(0);
            
            rec_gen1.add_selection(0, 1);
            rec_gen2.add_selection(0, 2);
            rec_gen3.add_selection(0, 3);
            rec_gen4.add_selection(1, 2);
            rec_gen5.add_selection(1, 3);
            rec_gen6.add_selection(2, 3);

            auto nr_dags = rec_gen.count_dags();
            auto nr_dags1 = rec_gen1.count_dags();
            auto nr_dags2 = rec_gen2.count_dags();
            auto nr_dags3 = rec_gen3.count_dags();
            auto nr_dags4 = rec_gen4.count_dags();
            auto nr_dags5 = rec_gen5.count_dags();
            auto nr_dags6 = rec_gen6.count_dags();
            
            rec_gen.reset(4, 7);
            auto seq_dags = rec_gen.gen_dags();
            printf("seq_dags.size()=%lu\n", seq_dags.size());
            assert(seq_dags.size() == nr_dags);

            printf("nr_dags1=%d\n", nr_dags1);
            printf("nr_dags2=%d\n", nr_dags2);
            printf("nr_dags3=%d\n", nr_dags3);
            printf("nr_dags4=%d\n", nr_dags4);
            printf("nr_dags5=%d\n", nr_dags5);
            printf("nr_dags6=%d\n", nr_dags6);

            rec_gen.reset(4, 7);
            rec_gen1.reset(4, 7);
            rec_gen2.reset(4, 7);
            rec_gen3.reset(4, 7);
            rec_gen4.reset(4, 7);
            rec_gen5.reset(4, 7);
            rec_gen6.reset(4, 7);
            
            rec_gen1.add_selection(0, 1);
            rec_gen2.add_selection(0, 2);
            rec_gen3.add_selection(0, 3);
            rec_gen4.add_selection(1, 2);
            rec_gen5.add_selection(1, 3);
            rec_gen6.add_selection(2, 3);

            auto dags1 = rec_gen1.gen_dags();
            auto dags2 = rec_gen2.gen_dags();
            auto dags3 = rec_gen3.gen_dags();
            auto dags4 = rec_gen4.gen_dags();
            auto dags5 = rec_gen5.gen_dags();
            auto dags6 = rec_gen6.gen_dags();
            std::vector<dag<2>> par_dags;
            for (auto& dag : dags1) {
                par_dags.push_back(dag);
            }
            for (auto& dag : dags2) {
                par_dags.push_back(dag);
            }
            for (auto& dag : dags3) {
                par_dags.push_back(dag);
            }
            for (auto& dag : dags4) {
                par_dags.push_back(dag);
            }
            for (auto& dag : dags5) {
                par_dags.push_back(dag);
            }
            for (auto& dag : dags6) {
                par_dags.push_back(dag);
            }
            printf("par_dags.size()=%lu\n", par_dags.size());

            const auto total_par = nr_dags1 + nr_dags2 + nr_dags3 + nr_dags4 +
                nr_dags5 + nr_dags6;
            assert(total_par == par_dags.size());
            
            printf("nr_dags=%d (SEQ)\n", nr_dags);
            printf("nr_dags=%d (PAR)\n", total_par);
            assert(nr_dags == total_par);

        }

    }
}

