#include <percy/percy.hpp>

int main(void)
{
    const auto classes = percy::generate_npn_classes<4>();
    assert(classes.size() == 222u);
    printf("Found 222 4-input NPN classes\n");

    return 0;
}
