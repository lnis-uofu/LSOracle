#include <percy/percy.hpp>
#include <cstdio>
#include <fstream>
#include <percy/io.hpp>

using namespace percy;

int main(void)
{
    dag<2> g;
    sat_dag_generator gen;
    unbounded_dag_generator ugen;

    int nr_vars = 2;
    int nr_vertices = 1;
    int nr_dags = 0;

    // Ensure that there is exactly 1 dag structure with 2 variables and 1
    // vertex.
    gen.reset(nr_vars, nr_vertices);
    nr_dags = 0;
    while (gen.next_dag(g)) {
        nr_dags++;
    }
    assert(nr_dags == 1);


    // Ensure that there are exactly 3 dag structures with 3 variables and 2
    // vertices.
    nr_vars = 3;
    nr_vertices = 2;
    gen.reset(nr_vars, nr_vertices);
    nr_dags = 0;
    while (gen.next_dag(g)) {
        nr_dags++;
    }
    assert(nr_dags == 3);

    // Ensure that there are exactly 12 dag structures with 3 variables and 3
    // vertices.
    nr_vars = 3;
    nr_vertices = 3;
    gen.reset(nr_vars, nr_vertices);
    nr_dags = 0;
    char buf[64];
    while (gen.next_dag(g)) {
        nr_dags++;
    }
    assert(nr_dags == 12);

    // Ensure that the total number of valid dag structures with 3 variables
    // and up to 3 vertices is 18.
    ugen.reset(3);
    nr_dags = 0;
    while (ugen.next_dag(g)) {
        if (g.get_nr_vertices() > 3) {
            break;
        }
        nr_dags++;
    }
    assert(nr_dags == 18);

    return 0;
}

