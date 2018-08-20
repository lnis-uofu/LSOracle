#include <percy/percy.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace percy;

/*******************************************************************************
    Tests creation, manipulation, and generation of floating dags.
*******************************************************************************/
int main(void)
{
    
    {
        int fanins[2];
        floating_dag<2> dag(3);

        to_dot(dag, std::cout);

        fanins[0] = 0;
        fanins[1] = 1;
        dag.set_vertex(1, fanins);
        to_dot(dag, std::cout);
        
        fanins[0] = 1;
        fanins[1] = 2;
        dag.set_vertex(2, fanins);
        to_dot(dag, std::cout);

        fanins[0] = 0;
        fanins[1] = 0;
        dag.set_vertex(1, fanins);
        to_dot(dag, std::cout);
        
        // Tests floating dag isomorphism.
        floating_dag<2> dag2(3);
        fanins[0] = 2;
        fanins[1] = 1;
        dag2.set_vertex(2, fanins);

#ifndef DISABLE_NAUTY
        auto iso_vec1 = dag.get_iso_vector();
        auto iso_vec2 = dag2.get_iso_vector();
        assert(iso_vec1 == iso_vec2);
        assert(dag.is_isomorphic(dag2));
        
        fanins[0] = 0;
        fanins[1] = 0;
        dag2.set_vertex(2, fanins);
        iso_vec1 = dag.get_iso_vector();
        iso_vec2 = dag2.get_iso_vector();
        assert(iso_vec1 != iso_vec2);
        assert(!dag.is_isomorphic(dag2));
#endif
    }

          
    return 0;
}

