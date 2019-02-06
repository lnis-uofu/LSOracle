#include <percy/percy.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace percy;
using kitty::dynamic_truth_table;

int main(void)
{
    {
        chain c;
        spec spec;
        spec.set_primitive(AIG);

        dynamic_truth_table tt1(2);
        dynamic_truth_table tt2(2);
        kitty::create_nth_var(tt1, 0);
        kitty::create_nth_var(tt2, 1);

        spec[0] = tt1 & tt2;
        auto result = synthesize(spec, c);
        assert(result == success);
        assert(c.get_nr_steps() == 1);
        assert(c.simulate()[0] == spec[0]);
        assert(c.is_aig());
        
        spec[0] = tt1 | tt2;
        result = synthesize(spec, c);
        assert(result == success);
        assert(c.simulate()[0] == spec[0]);
        assert(c.is_aig());

        spec[0] = tt1 ^ tt2;
        result = synthesize(spec, c);
        assert(result == success);
        assert(c.simulate()[0] == spec[0]);
        assert(c.is_aig());
    }
    {
        // Synthesize the function x_1 XOR x_2 XOR x_3
        chain c;
        spec spec;
        spec.set_primitive(AIG);

        dynamic_truth_table tt1(3);
        dynamic_truth_table tt2(3);
        dynamic_truth_table tt3(3);

        kitty::create_nth_var(tt1, 0);
        kitty::create_nth_var(tt2, 1);
        kitty::create_nth_var(tt3, 2);

        spec[0] = tt1 ^ tt2 ^ tt3;
        const auto result = synthesize(spec, c);
        assert(result == success);
        assert(c.is_aig());
    }

    {
        chain c;
        spec spec;
        spec.set_primitive(AIG);
        kitty::dynamic_truth_table tt(3);
        for (int i = 0; i < 256; i++) {
            kitty::create_from_words(tt, &i, &i + 1);
            spec[0] = tt;
            const auto result = synthesize(spec, c);
            assert(result == success);
            assert(c.is_aig());
            assert(c.simulate()[0] == tt);
        }
    }

    return 0;
}

