#include <percy/percy.hpp>

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

#ifndef DISABLE_NAUTY
    // Should be self-isomorphic
    assert(g1.is_isomorphic(g1));
    assert(g2.is_isomorphic(g2));
    assert(g3.is_isomorphic(g3));

    // Check between DAGs
    assert(g1.is_isomorphic(g2));
    assert(!g1.is_isomorphic(g3));

    pd_iso_checker checker(7);
    assert(checker.isomorphic(g1, g1));
    assert(checker.isomorphic(g2, g2));
    assert(checker.isomorphic(g3, g3));
    assert(checker.isomorphic(g1, g2));
    assert(!checker.isomorphic(g1, g3));
#endif

    return 0;
}
