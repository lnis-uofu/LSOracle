#include <cstdio>
#include <percy/percy.hpp>

using namespace percy;

/*******************************************************************************
    Tests bitset manipulations. These is used to encode variable assignments
    for arbitrary-fanin synthesis.
 *******************************************************************************/
int
main(void)
{
    std::vector<int> asgn{ 0, 0 };

    // Verify that bitset assignment is initialized to zero.
    assert(is_zero(asgn));

    next_assignment(asgn);
    assert(!is_zero(asgn));
    assert(asgn[0] == true);
    assert(asgn[1] == false);
    
    next_assignment(asgn);
    assert(asgn[0] == false);
    assert(asgn[1] == true);

    next_assignment(asgn);
    assert(asgn[0] == true);
    assert(asgn[1] == true);
    
    next_assignment(asgn);
    assert(is_zero(asgn));
    assert(asgn[0] == false);
    assert(asgn[1] == false);

    return 0;
}

