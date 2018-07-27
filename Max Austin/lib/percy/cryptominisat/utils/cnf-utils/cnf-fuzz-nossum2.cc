#include <random>
#include <iostream>

static const unsigned int nr_groups = 16;
static const unsigned int nr_variables = 16;

/* XXX: Seed */
std::mt19937 my_random;

static void group(unsigned int group)
{
	uint64_t i = 0;
	do {
		if (my_random() % (1 << (nr_variables - 4)) == 0)
			continue;

		for (unsigned int j = 0; j < nr_variables; ++j) {
			if (my_random() % nr_variables == 0)
				continue;

			std::cout
			<<  (i & (1U << j) ? "-" : "")
            << (1 + nr_variables * group + j)
            << " ";
		}

		std::cout << "0" << std::endl;
	} while (++i & ((1UL << nr_variables) - 1));
}

int main(int argc, char *argv[])
{
	for (unsigned int i = 0; i < nr_groups; ++i)
		group(i);

	for (unsigned int i = 0; i < (nr_groups * nr_variables) >> 4; ++i) {
		for (unsigned int j = 0; j < nr_groups << 2; ++j) {
			std::cout
			<< (my_random() % 2 ? "-" : "")
            << (1 + (my_random() % (nr_groups * nr_variables)))
            << " ";
		}

		std::cout << "0" << std::endl;
	}

	return 0;
}
