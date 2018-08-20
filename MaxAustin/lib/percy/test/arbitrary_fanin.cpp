#include <cstdio>
#include <percy/percy.hpp>

using namespace percy;

/*******************************************************************************
    Tests support for exact synthesis of arbitrary fanin chains. Specifically,
    tests if the functions that are used to map selection variable indices to
    fanin assignments work correctly.
 *******************************************************************************/
int
main(void)
{
    std::vector<int> init_test = { 0, 0 };
    fanin_init(init_test, 1);
    assert(init_test[0] == 0);
    assert(init_test[1] == 1);

    std::vector<int> fanin2 = { 0, 1 };
    assert(fanin_inc(fanin2, 1) == false);

    assert(fanin_inc(fanin2, 2) == true);
    assert(fanin2[0] == 0);
    assert(fanin2[1] == 2);
    
    assert(fanin_inc(fanin2, 2) == true);
    assert(fanin2[0] == 1);
    assert(fanin2[1] == 2);

    assert(fanin_inc(fanin2, 2) == false);

    std::vector<int> fanin3 = { 0, 1, 2 };
    assert(fanin_inc(fanin3, 2) == false);
    
    assert(fanin_inc(fanin3, 3) == true);
    assert(fanin3[0] == 0);
    assert(fanin3[1] == 1);
    assert(fanin3[2] == 3);
    
    assert(fanin_inc(fanin3, 3) == true);
    assert(fanin3[0] == 0);
    assert(fanin3[1] == 2);
    assert(fanin3[2] == 3);

    assert(fanin_inc(fanin3, 3) == true);
    assert(fanin3[0] == 1);
    assert(fanin3[1] == 2);
    assert(fanin3[2] == 3);

    assert(fanin_inc(fanin3, 3) == false);

    return 0;
}

