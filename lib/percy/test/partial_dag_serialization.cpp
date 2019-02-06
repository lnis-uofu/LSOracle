#include <percy/percy.hpp>
#include <vector>

using namespace percy;

int main(void)
{
    partial_dag g1, g2, g3;

    g1.reset(2, 4);
    g1.set_vertex(0, 0, 0);
    g1.set_vertex(1, 0, 0);
    g1.set_vertex(2, 0, 1);
    g1.set_vertex(3, 2, 3);

    g2.reset(2, 4);
    g2.set_vertex(0, 0, 0);
    g2.set_vertex(1, 0, 0);
    g2.set_vertex(2, 0, 2);
    g2.set_vertex(3, 1, 3);

    g3.reset(2, 4);
    g3.set_vertex(0, 0, 0);
    g3.set_vertex(1, 0, 0);
    g3.set_vertex(2, 1, 2);
    g3.set_vertex(3, 1, 3);

    std::vector<partial_dag> dags;
    dags.push_back(g1);
    dags.push_back(g2);
    dags.push_back(g3);

    write_partial_dags(dags, "test.bin");
    auto read_dags = read_partial_dags("test.bin");
    for (const auto& dag : read_dags) {
        for (int i = 0; i < dag.nr_vertices(); i++) {
            auto v = dag.get_vertex(i);
            printf("(%d,%d) - ", v[0], v[1]);
        }
        printf("\n");
    }


    printf("Writing single DAG\n");
    auto fhandle = fopen("test.bin", "wb");
    write_partial_dag(g1, fhandle);
    fclose(fhandle);

    printf("Reading single DAG\n");
    read_dags = read_partial_dags("test.bin");
    for (const auto& dag : read_dags) {
        for (int i = 0; i < dag.nr_vertices(); i++) {
            auto v = dag.get_vertex(i);
            printf("(%d,%d) - ", v[0], v[1]);
        }
        printf("\n");
    }

    return 0;
}
