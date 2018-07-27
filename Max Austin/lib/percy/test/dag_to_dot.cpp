#include <percy/percy.hpp>
#include <percy/io.hpp>
#include <cstdio>
#include <fstream>

using namespace percy;

/*******************************************************************************
    Verifies that the DAG to .dot conversion works properly.
*******************************************************************************/
int main(void)
{
    int ctr = 0;
    binary_dag g;
    unbounded_dag_generator ugen;

    ugen.reset(3);
    ctr = 0;
    while (ugen.next_dag(g)) {
        if (++ctr > 10) {
            break;
        }
        to_dot(g, std::cout);
    }

    ugen.reset(4);
    ctr = 0;
    while (ugen.next_dag(g)) {
        if (++ctr > 10) {
            break;
        }
        to_dot(g, std::cout);
    }

    ternary_dag h(5);
    h.add_vertex({ 0, 1, 2 });
    h.add_vertex({ 4, 3, 1 });
    h.add_vertex({ 5, 6, 0 });
    h.add_vertex({ 7, 6, 1 });
    to_dot(h, std::cout);

    return 0;
}

