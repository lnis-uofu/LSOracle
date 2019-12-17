#include <cstdio>
#include <percy/percy.hpp>
#include <cstdlib>

using namespace percy;

bool verify_simulated_dc_tt(
    const kitty::dynamic_truth_table& tt,
    const kitty::dynamic_truth_table& dc_tt,
    const kitty::dynamic_truth_table& dc_mask)
{
    for (auto i = 0u; i < tt.num_bits(); i++) {
        if (kitty::get_bit(dc_mask, i)) {
            continue;
        }
        assert(kitty::get_bit(dc_tt, i) == kitty::get_bit(tt, i));
    }

    return true;
}

template<class Encoder>
void verify_dc(const kitty::dynamic_truth_table& tt, const kitty::dynamic_truth_table& dc_mask)
{
    assert(tt.num_vars() == dc_mask.num_vars());

    //printf("got truth table\t %s\n", kitty::to_binary(tt).c_str());
    //printf("got DC mask\t %s\n", kitty::to_binary(dc_mask).c_str());

    chain chain;
    spec spec;
    bmcg_wrapper solver;
    Encoder encoder(solver);
    spec[0] = tt;

    const auto res = synthesize(spec, chain, solver, encoder);
    assert(res == success);
    //printf("found a %d-step chain\n", chain.get_nr_steps());
    const auto full_tt = chain.simulate()[0];
    assert(full_tt == tt);

    spec.set_dont_care(0, dc_mask);
    const auto dc_res = synthesize(spec, chain, solver, encoder);
    assert(dc_res == success);
    //printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto dc_tt = chain.simulate()[0];
    //printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, dc_tt, dc_mask));

    /*
    const auto cegar_dc_res = synthesize(spec, chain, solver, encoder, SYNTH_STD_CEGAR);
    assert(cegar_dc_res == success);
    //printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto cegar_dc_tt = chain.simulate()[0];
    //printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, cegar_dc_tt, dc_mask));
    */
}

template<class Encoder>
void verify_dc3(const kitty::dynamic_truth_table& tt, const kitty::dynamic_truth_table& dc_mask)
{
    assert(tt.num_vars() == dc_mask.num_vars());

    printf("got truth table\t %s\n", kitty::to_binary(tt).c_str());
    printf("got DC mask\t %s\n", kitty::to_binary(dc_mask).c_str());

    chain chain;
    spec spec;
    spec.fanin = 3;
    bmcg_wrapper solver;
    Encoder encoder(solver);
    spec[0] = tt;

    const auto res = synthesize(spec, chain, solver, encoder);
    assert(res == success);
    printf("found a %d-step chain\n", chain.get_nr_steps());
    const auto full_tt = chain.simulate()[0];
    assert(full_tt == tt);

    spec.set_dont_care(0, dc_mask);
    const auto dc_res = synthesize(spec, chain, solver, encoder);
    assert(dc_res == success);
    printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto dc_tt = chain.simulate()[0];
    printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, dc_tt, dc_mask));

    /*
    const auto cegar_dc_res = synthesize(spec, chain, solver, encoder, SYNTH_STD_CEGAR);
    assert(cegar_dc_res == success);
    printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto cegar_dc_tt = chain.simulate()[0];
    printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, cegar_dc_tt, dc_mask));
    */
}

template<class Encoder>
void verify_fence_dc(const kitty::dynamic_truth_table& tt, const kitty::dynamic_truth_table& dc_mask)
{
    assert(tt.num_vars() == dc_mask.num_vars());

    //printf("got truth table\t %s\n", kitty::to_binary(tt).c_str());
    //printf("got DC mask\t %s\n", kitty::to_binary(dc_mask).c_str());

    chain chain;
    spec spec;
    bmcg_wrapper solver;
    Encoder encoder(solver);
    spec[0] = tt;

    const auto res = fence_synthesize(spec, chain, solver, encoder);
    assert(res == success);
    //printf("found a %d-step chain\n", chain.get_nr_steps());
    const auto full_tt = chain.simulate()[0];
    assert(full_tt == tt);

    spec.set_dont_care(0, dc_mask);
    const auto dc_res = fence_synthesize(spec, chain, solver, encoder);
    assert(dc_res == success);
    //printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto dc_tt = chain.simulate()[0];
    //printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, dc_tt, dc_mask));

    /*
    const auto cegar_dc_res = fence_cegar_synthesize(spec, chain, solver, encoder);
    assert(cegar_dc_res == success);
    //printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto cegar_dc_tt = chain.simulate()[0];
    //printf("DC simulation tt\t %s\n", kitty::to_binary(cegar_dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, cegar_dc_tt, dc_mask));
    */

    const auto pf_dc_res = pf_fence_synthesize(spec, chain);
    assert(pf_dc_res == success);
    //printf("found a %d-step DC chain (PF)\n", chain.get_nr_steps());
    const auto pf_dc_tt = chain.simulate()[0];
    //printf("DC simulation tt\t %s (PF)\n", kitty::to_binary(pf_dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, pf_dc_tt, dc_mask));
}

template<class Encoder>
void verify_encoder(void)
{
    {
        kitty::dynamic_truth_table tt(3), dc(3);
        for (int i = 0; i < 256; i++) {
            kitty::create_from_words(tt, &i, &i + 1);

            kitty::clear(dc);
            for (int i = 0; i < 8; i++) {
                const auto number = rand() % 100 + 1; // Random number between 1 and 100
                if (number <= 25) {
                    kitty::set_bit(dc, i);
                }
            }

            verify_dc<Encoder>(tt, dc);
        }
    }

    {
        kitty::dynamic_truth_table tt(4), dc(4);
        for (int i = 0; i < 256; i++) {
            kitty::create_random(tt);
            kitty::clear(dc);
            for (int i = 0; i < 16; i++) {
                const auto number = rand() % 100 + 1; // Random number between 1 and 100
                if (number <= 11) {
                    kitty::set_bit(dc, i);
                }
            }

            verify_dc3<Encoder>(tt, dc);
        }
    }

    {
        kitty::dynamic_truth_table a(4), b(4), c(4), x(4), y(4), dc;

        kitty::create_nth_var(a, 0);
        kitty::create_nth_var(b, 1);
        kitty::create_nth_var(c, 2);
        kitty::create_nth_var(x, 3);

        y = (a & b & x) | (~a & c & x);
        dc = (a & ~b & x) | (~a & ~x);
        verify_dc<Encoder>(y, dc);
    }

    {
        // Even a random 6-input function can be made trivial by
        // using a don't care mask
        spec spec;
        chain c;
        bsat_wrapper solver;
        Encoder encoder(solver);
        kitty::dynamic_truth_table tt(6), dc_mask(6);
        kitty::create_random(tt);
        dc_mask = ~dc_mask;
        spec[0] = tt;
        spec.set_dont_care(0, dc_mask);

        printf("got truth table\t %s\n", kitty::to_binary(tt).c_str());
        printf("got DC mask\t %s\n", kitty::to_binary(dc_mask).c_str());

        const auto result = synthesize(spec, c, solver, encoder);
        assert(result == success);
        const auto dc_tt = c.simulate()[0];
        printf("found a %d-step DC chain\n", c.get_nr_steps());
        printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
        verify_simulated_dc_tt(tt, dc_tt, dc_mask);


        /*
        const auto cegar_result = synthesize(spec, c, solver, encoder, SYNTH_STD_CEGAR);
        assert(cegar_result == success);
        const auto cegar_dc_tt = c.simulate()[0];
        printf("found a %d-step DC chain (CEGAR)\n", c.get_nr_steps());
        printf("DC simulation tt\t %s (CEGAR)\n", kitty::to_binary(cegar_dc_tt).c_str());
        verify_simulated_dc_tt(tt, cegar_dc_tt, dc_mask);
        */
    }   
}

template<class Encoder>
void verify_fence_encoder(void)
{
    {
        kitty::dynamic_truth_table tt(3), dc(3);
        for (int i = 0; i < 256; i++) {
            kitty::create_from_words(tt, &i, &i + 1);

            kitty::clear(dc);
            for (int i = 0; i < 8; i++) {
                const auto number = rand() % 100 + 1; // Random number between 1 and 100
                if (number <= 25) {
                    kitty::set_bit(dc, i);
                }
            }

            verify_fence_dc<Encoder>(tt, dc);
        }
    }
    {
        kitty::dynamic_truth_table a(4), b(4), c(4), x(4), y(4), dc;

        kitty::create_nth_var(a, 0);
        kitty::create_nth_var(b, 1);
        kitty::create_nth_var(c, 2);
        kitty::create_nth_var(x, 3);

        y = (a & b & x) | (~a & c & x);
        dc = (a & ~b & x) | (~a & ~x);
        verify_fence_dc<Encoder>(y, dc);
    }

    {
        // Even a random 6-input function can be made trivial by
        // using a don't care mask
        spec spec;
        chain c;
        bsat_wrapper solver;
        Encoder encoder(solver);
        kitty::dynamic_truth_table tt(6), dc_mask(6);
        kitty::create_random(tt);
        dc_mask = ~dc_mask;
        spec[0] = tt;
        spec.set_dont_care(0, dc_mask);

        printf("got truth table\t %s\n", kitty::to_binary(tt).c_str());
        printf("got DC mask\t %s\n", kitty::to_binary(dc_mask).c_str());

        const auto result = fence_synthesize(spec, c, solver, encoder);
        assert(result == success);
        const auto dc_tt = c.simulate()[0];
        printf("found a %d-step DC chain\n", c.get_nr_steps());
        printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
        verify_simulated_dc_tt(tt, dc_tt, dc_mask);

        /*
        const auto cegar_result = fence_cegar_synthesize(spec, c, solver, encoder);
        assert(cegar_result == success);
        const auto cegar_dc_tt = c.simulate()[0];
        printf("found a %d-step DC chain (CEGAR)\n", c.get_nr_steps());
        printf("DC simulation tt\t %s (CEGAR)\n", kitty::to_binary(cegar_dc_tt).c_str());
        verify_simulated_dc_tt(tt, cegar_dc_tt, dc_mask);
        */
    }   
}

template<class Encoder>
void verify_aig_dc(const kitty::dynamic_truth_table& tt, const kitty::dynamic_truth_table& dc_mask)
{
    assert(tt.num_vars() == dc_mask.num_vars());

    //printf("got truth table\t %s\n", kitty::to_binary(tt).c_str());
    //printf("got DC mask\t %s\n", kitty::to_binary(dc_mask).c_str());

    chain chain;
    spec spec;
    bmcg_wrapper solver;
    Encoder encoder(solver);
    spec[0] = tt;
    spec.set_primitive(AIG);

    const auto res = synthesize(spec, chain, solver, encoder);
    assert(res == success);
    //printf("found a %d-step chain\n", chain.get_nr_steps());
    const auto full_tt = chain.simulate()[0];
    assert(full_tt == tt);
    assert(chain.is_aig());

    spec.set_dont_care(0, dc_mask);
    const auto dc_res = synthesize(spec, chain, solver, encoder);
    assert(dc_res == success);
    //printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto dc_tt = chain.simulate()[0];
    //printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, dc_tt, dc_mask));
    assert(chain.is_aig());

    /*
    const auto cegar_dc_res = synthesize(spec, chain, solver, encoder, SYNTH_STD_CEGAR);
    assert(cegar_dc_res == success);
    //printf("found a %d-step DC chain\n", chain.get_nr_steps());
    const auto cegar_dc_tt = chain.simulate()[0];
    //printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
    assert(verify_simulated_dc_tt(tt, cegar_dc_tt, dc_mask));
    assert(chain.is_aig());
    */
}

template<class Encoder>
void verify_aig_encoder(void)
{
    {
        kitty::dynamic_truth_table tt(3), dc(3);
        for (int i = 0; i < 256; i++) {
            kitty::create_from_words(tt, &i, &i + 1);

            kitty::clear(dc);
            for (int i = 0; i < 8; i++) {
                const auto number = rand() % 100 + 1; // Random number between 1 and 100
                if (number <= 25) {
                    kitty::set_bit(dc, i);
                }
            }

            verify_aig_dc<Encoder>(tt, dc);
        }
    }
    {
        kitty::dynamic_truth_table a(4), b(4), c(4), x(4), y(4), dc;

        kitty::create_nth_var(a, 0);
        kitty::create_nth_var(b, 1);
        kitty::create_nth_var(c, 2);
        kitty::create_nth_var(x, 3);

        y = (a & b & x) | (~a & c & x);
        dc = (a & ~b & x) | (~a & ~x);
        verify_aig_dc<Encoder>(y, dc);
    }

    {
        // Even a random 6-input function can be made trivial by
        // using a don't care mask
        spec spec;
        spec.set_primitive(AIG);
        chain c;
        bsat_wrapper solver;
        Encoder encoder(solver);
        kitty::dynamic_truth_table tt(6), dc_mask(6);
        kitty::create_random(tt);
        dc_mask = ~dc_mask;
        spec[0] = tt;
        spec.set_dont_care(0, dc_mask);

        printf("got truth table\t %s\n", kitty::to_binary(tt).c_str());
        printf("got DC mask\t %s\n", kitty::to_binary(dc_mask).c_str());

        const auto result = synthesize(spec, c, solver, encoder);
        assert(result == success);
        assert(c.is_aig());
        const auto dc_tt = c.simulate()[0];
        printf("found a %d-step DC AIG\n", c.get_nr_steps());
        printf("DC simulation tt\t %s\n", kitty::to_binary(dc_tt).c_str());
        verify_simulated_dc_tt(tt, dc_tt, dc_mask);

        /*
        const auto cegar_result = synthesize(spec, c, solver, encoder, SYNTH_STD_CEGAR);
        assert(cegar_result == success);
        assert(c.is_aig());
        const auto cegar_dc_tt = c.simulate()[0];
        printf("found a %d-step DC AIG (CEGAR)\n", c.get_nr_steps());
        printf("DC simulation tt\t %s (CEGAR)\n", kitty::to_binary(cegar_dc_tt).c_str());
        verify_simulated_dc_tt(tt, cegar_dc_tt, dc_mask);
        */
    }   
}

int main(void)
{
    verify_encoder<ssv_encoder>();
    verify_encoder<msv_encoder>();
    verify_encoder<ditt_encoder>();
   // verify_fence_encoder<ssv_fence_encoder>();
    verify_fence_encoder<ssv_fence2_encoder>();
    verify_aig_encoder<ssv_encoder>();
    return 0;
}

