#include <percy/percy.hpp>
#include <cstdio>
#include <fstream>

using namespace percy;

int main(void)
{
#ifndef DISABLE_NAUTY
    dag<2> g1, g2;
    sat_dag_generator gen;
    unbounded_dag_generator ugen;

    int nr_vars = 3;
    int nr_vertices = 4;
    bool isomorphic = false;

    // First check a simple symmetry break where we simply swap two nodes.
    g1.reset(nr_vars, nr_vertices);
    g2.reset(nr_vars, nr_vertices);

    g1.set_vertex(0, 1, 2);
    g1.set_vertex(1, 1, 2);
    g1.set_vertex(2, 0, 3);
    g1.set_vertex(3, 4, 5);

    g2.set_vertex(0, 1, 2);
    g2.set_vertex(1, 1, 2);
    g2.set_vertex(2, 0, 4);
    g2.set_vertex(3, 3, 5);

    isomorphic = g1.is_isomorphic(g2, 1);
    assert(isomorphic);
    
    // Verify that permutation isomorphism does not hold up in this case.
    isomorphic = g1.is_perm_isomorphic(g2);
    assert(!isomorphic);

    // A slighly more complex isomorphism check where the DAGs are mirror
    // images of each other.
    g2.set_vertex(0, 0, 1);
    g2.set_vertex(1, 0, 1);
    g2.set_vertex(2, 2, 4);
    g2.set_vertex(3, 3, 5);

    isomorphic = g1.is_isomorphic(g2, 1);
    assert(isomorphic);

    // Verify that permutation isomorphism does not hold up in this case.
    isomorphic = g1.is_perm_isomorphic(g2);
    assert(!isomorphic);

    // Ensure that non-isomorphic graphs do not result in false positives.
    g2.set_vertex(0, 0, 2);
    g2.set_vertex(1, 1, 3);
    g2.set_vertex(2, 1, 3);
    g2.set_vertex(3, 4, 5);

    isomorphic = g1.is_isomorphic(g2, 1);
    assert(!isomorphic);
#endif
    return 0;
}

