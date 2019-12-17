#include <percy/percy.hpp>

extern int test(void);

int main(void)
{
    const auto val = test();
    printf("hello world\n");
    printf("val=%d\n", val);

    return 0;
}
