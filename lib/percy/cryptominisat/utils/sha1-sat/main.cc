/*
 * sha1-sat -- SAT instance generator for SHA-1
 * Copyright (C) 2011-2012  Vegard Nossum <vegard.nossum@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <boost/program_options.hpp>
#include "format.hh"

#if defined (_MSC_VER)
#define my_popcnt(x) __popcnt(x)
#else
#define my_popcnt(x) __builtin_popcount(x)
#endif


/* Instance options */
static std::string config_attack = "preimage";
static unsigned int config_nr_rounds = 80;
static unsigned int config_nr_message_bits = 0;
static unsigned int config_nr_hash_bits = 160;
static bool all_zero_output = false;

/* CNF options */
static bool config_use_xor_clauses = false;
static bool config_use_tseitin_adders = true;
static bool config_restrict_branching = false;
static bool nocomment = false;

static std::ostringstream cnf;

static void comment(std::string str)
{
    if (!nocomment) {
        cnf << format("c $\n", str);
    }
}

static int nr_variables = 0;
static unsigned int nr_clauses = 0;
static unsigned int nr_xor_clauses = 0;
static unsigned int nr_constraints = 0;

static void new_vars(std::string label, int x[], unsigned int n, bool decision_var = true)
{
	for (unsigned int i = 0; i < n; ++i)
		x[i] = ++nr_variables;

	comment(format("var $/$ $", x[0], n, label));

	if (config_restrict_branching) {
		if (decision_var) {
			for (unsigned int i = 0; i < n; ++i)
				cnf << format("d $ 0\n", x[i]);
		} else {
			for (unsigned int i = 0; i < n; ++i)
				cnf << format("d -$ 0\n", x[i]);
		}
	}
}

static void constant(int r, bool value)
{
	cnf << format("$$ 0\n", value ? "" : "-", r);

	nr_clauses += 1;
	nr_constraints += 1;
}

static void constant32(int r[], uint32_t value)
{
	comment(format("constant32 ($)", value));

	for (unsigned int i = 0; i < 32; ++i) {
		cnf << format("$$ 0\n", (value >> i) & 1 ? "" : "-", r[i]);

		nr_clauses += 1;
		nr_constraints += 1;
	}
}

static void new_constant(std::string label, int r[32], uint32_t value)
{
	new_vars(label, r, 32);
	constant32(r, value);
}

template<typename T>
static void args_to_vector(std::vector<T> &/*v*/)
{
}

template<typename T, typename... Args>
static void args_to_vector(std::vector<T> &v, T x, Args... args)
{
	v.push_back(x);
	return args_to_vector(v, args...);
}

static void clause(const std::vector<int> &v)
{
	for (int x: v) {
		cnf << format("$$ ", x < 0 ? "-" : "", abs(x));
	}

	cnf << format("0\n");

	nr_clauses += 1;
	nr_constraints += 1;
}

template<typename... Args>
static void clause(Args... args)
{
	std::vector<int> v;
	args_to_vector(v, args...);
	clause(v);
}

static void xor_clause(const std::vector<int> &v)
{
	cnf << format("x ");

	for (int x: v)
		cnf << format("$$ ", x < 0 ? "-" : "", abs(x));

	cnf << format("0\n");

	nr_xor_clauses += 1;
}

template<typename... Args>
static void xor_clause(Args... args)
{
	std::vector<int> v;
	args_to_vector(v, args...);
	xor_clause(v);
}

static void xor2(int r[], int a[], int b[], unsigned int n)
{
	comment("xor2");

	if (config_use_xor_clauses) {
		for (unsigned int i = 0; i < n; ++i)
			xor_clause(-r[i], a[i], b[i]);
	} else {
		for (unsigned int i = 0; i < n; ++i) {
			for (unsigned int j = 0; j < 8; ++j) {
				if (my_popcnt(j ^ 1) % 2 == 1)
					continue;

				clause((j & 1) ? -r[i] : r[i],
					(j & 2) ? a[i] : -a[i],
					(j & 4) ? b[i] : -b[i]);
			}
		}
	}
}

static void xor3(int r[], int a[], int b[], int c[], unsigned int n = 32)
{
	comment("xor3");

	if (config_use_xor_clauses) {
		for (unsigned int i = 0; i < n; ++i)
			xor_clause(-r[i], a[i], b[i], c[i]);
	} else {
		for (unsigned int i = 0; i < n; ++i) {
			for (unsigned int j = 0; j < 16; ++j) {
				if (my_popcnt(j ^ 1) % 2 == 0)
					continue;

				clause((j & 1) ? -r[i] : r[i],
					(j & 2) ? a[i] : -a[i],
					(j & 4) ? b[i] : -b[i],
					(j & 8) ? c[i] : -c[i]);
			}
		}
	}
}

static void xor4(int r[32], int a[32], int b[32], int c[32], int d[32])
{
	comment("xor4");

	if (config_use_xor_clauses) {
		for (unsigned int i = 0; i < 32; ++i)
			xor_clause(-r[i], a[i], b[i], c[i], d[i]);
	} else {
		for (unsigned int i = 0; i < 32; ++i) {
			for (unsigned int j = 0; j < 32; ++j) {
				if (my_popcnt(j ^ 1) % 2 == 1)
					continue;

				clause((j & 1) ? -r[i] : r[i],
					(j & 2) ? a[i] : -a[i],
					(j & 4) ? b[i] : -b[i],
					(j & 8) ? c[i] : -c[i],
					(j & 16) ? d[i] : -d[i]);
			}
		}
	}
}

static void eq(int a[], int b[], unsigned int n = 32)
{
	if (config_use_xor_clauses) {
		for (unsigned int i = 0; i < n; ++i)
			xor_clause(-a[i], b[i]);
	} else {
		for (unsigned int i = 0; i < n; ++i) {
			clause(-a[i], b[i]);
			clause(a[i], -b[i]);
		}
	}
}

static void neq(int a[], int b[], unsigned int n = 32)
{
	if (config_use_xor_clauses) {
		for (unsigned int i = 0; i < n; ++i)
			xor_clause(a[i], b[i]);
	} else {
		for (unsigned int i = 0; i < n; ++i) {
			clause(a[i], b[i]);
			clause(-a[i], -b[i]);
		}
	}
}

static void and2(int r[], int a[], int b[], unsigned int n)
{
	for (unsigned int i = 0; i < n; ++i) {
		clause(r[i], -a[i], -b[i]);
		clause(-r[i], a[i]);
		clause(-r[i], b[i]);
	}
}

static void or2(int r[], int a[], int b[], unsigned int n)
{
	for (unsigned int i = 0; i < n; ++i) {
		clause(-r[i], a[i], b[i]);
		clause(r[i], -a[i]);
		clause(r[i], -b[i]);
	}
}

static void add2(std::string /*label*/, int r[32], int a[32], int b[32])
{
	comment("add2");

	if (config_use_tseitin_adders) {
		int c[31];
		new_vars("carry", c, 31);

		int t0[31];
		new_vars("t0", t0, 31);

		int t1[31];
		new_vars("t1", t1, 31);

		int t2[31];
		new_vars("t2", t2, 31);

		and2(c, a, b, 1);
		xor2(r, a, b, 1);

		xor2(t0, &a[1], &b[1], 31);
		and2(t1, &a[1], &b[1], 31);
		and2(t2, t0, c, 31);
		or2(&c[1], t1, t2, 30);
		xor2(&r[1], t0, c, 31);
	}
}

static void add5(std::string label, int r[32], int a[32], int b[32], int c[32], int d[32], int e[32])
{
	comment("add5");

	if (config_use_tseitin_adders) {
		int t0[32];
		new_vars("t0", t0, 32);

		int t1[32];
		new_vars("t1", t1, 32);

		int t2[32];
		new_vars("t2", t2, 32);

		add2(label, t0, a, b);
		add2(label, t1, c, d);
		add2(label, t2, t0, t1);
		add2(label, r, t2, e);
	} else {
		assert(false);
	}
}

static void rotl(int r[32], int x[32], unsigned int n)
{
	for (unsigned int i = 0; i < 32; ++i)
		r[i] = x[(i + 32 - n) % 32];
}

class sha1 {
public:
	int w[80][32];
	int h_in[5][32];
	int h_out[5][32];

	int a[85][32];

	sha1(unsigned int nr_rounds, std::string name)
	{
		comment("sha1");
		comment(format("parameter nr_rounds = $", nr_rounds));

		for (unsigned int i = 0; i < 16; ++i)
			new_vars(format("w$[$]", name, i), w[i], 32, !config_restrict_branching);

		/* XXX: Fix this later by writing directly to w[i] */
		int wt[80][32];
		for (unsigned int i = 16; i < nr_rounds; ++i)
			new_vars(format("w$[$]", name, i), wt[i], 32);

		new_vars(format("h$_in0", name), h_in[0], 32);
		new_vars(format("h$_in1", name), h_in[1], 32);
		new_vars(format("h$_in2", name), h_in[2], 32);
		new_vars(format("h$_in3", name), h_in[3], 32);
		new_vars(format("h$_in4", name), h_in[4], 32);

		new_vars(format("h$_out0", name), h_out[0], 32);
		new_vars(format("h$_out1", name), h_out[1], 32);
		new_vars(format("h$_out2", name), h_out[2], 32);
		new_vars(format("h$_out3", name), h_out[3], 32);
		new_vars(format("h$_out4", name), h_out[4], 32);

		for (unsigned int i = 0; i < nr_rounds; ++i)
			new_vars(format("a[$]", i + 5), a[i + 5], 32);

		for (unsigned int i = 16; i < nr_rounds; ++i) {
			xor4(wt[i], w[i - 3], w[i - 8], w[i - 14], w[i - 16]);
			rotl(w[i], wt[i], 1);
		}

		/* Fix constants */
		int k[4][32];
		new_constant("k[0]", k[0], 0x5a827999);
		new_constant("k[1]", k[1], 0x6ed9eba1);
		new_constant("k[2]", k[2], 0x8f1bbcdc);
		new_constant("k[3]", k[3], 0xca62c1d6);

		constant32(h_in[0], 0x67452301);
		constant32(h_in[1], 0xefcdab89);
		constant32(h_in[2], 0x98badcfe);
		constant32(h_in[3], 0x10325476);
		constant32(h_in[4], 0xc3d2e1f0);

		rotl(a[4], h_in[0], 32 - 0);
		rotl(a[3], h_in[1], 32 - 0);
		rotl(a[2], h_in[2], 32 - 30);
		rotl(a[1], h_in[3], 32 - 30);
		rotl(a[0], h_in[4], 32 - 30);

		for (unsigned int i = 0; i < nr_rounds; ++i) {
			int prev_a[32];
			rotl(prev_a, a[i + 4], 5);

			int b[32];
			rotl(b, a[i + 3], 0);

			int c[32];
			rotl(c, a[i + 2], 30);

			int d[32];
			rotl(d, a[i + 1], 30);

			int e[32];
			rotl(e, a[i + 0], 30);

			int f[32];
			new_vars(format("f[$]", i), f, 32);

			if (i < 20) {
				for (unsigned int j = 0; j < 32; ++j) {
					clause(-f[j], -b[j], c[j]);
					clause(-f[j], b[j], d[j]);
					clause(-f[j], c[j], d[j]);

					clause(f[j], -b[j], -c[j]);
					clause(f[j], b[j], -d[j]);
					clause(f[j], -c[j], -d[j]);
				}
			} else if (i >= 20 && i < 40) {
				xor3(f, b, c, d);
			} else if (i >= 40 && i < 60) {
				for (unsigned int j = 0; j < 32; ++j) {
					clause(-f[j], b[j], c[j]);
					clause(-f[j], b[j], d[j]);
					clause(-f[j], c[j], d[j]);

					clause(f[j], -b[j], -c[j]);
					clause(f[j], -b[j], -d[j]);
					clause(f[j], -c[j], -d[j]);
					//clause(f[j], -b[j], -c[j], -d[j]);
				}
			} else if (i >= 60 && i < 80) {
				xor3(f, b, c, d);
			}

			add5(format("a[$]", i + 5), a[i + 5], prev_a, f, e, k[i / 20], w[i]);
		}

		/* Rotate back */
		int c[32];
		rotl(c, a[nr_rounds + 2], 30);

		int d[32];
		rotl(d, a[nr_rounds + 1], 30);

		int e[32];
		rotl(e, a[nr_rounds + 0], 30);

		add2("h_out", h_out[0], h_in[0], a[nr_rounds + 4]);
		add2("h_out", h_out[1], h_in[1], a[nr_rounds + 3]);
		add2("h_out", h_out[2], h_in[2], c);
		add2("h_out", h_out[3], h_in[3], d);
		add2("h_out", h_out[4], h_in[4], e);
	}

};

static uint32_t rotl(uint32_t x, unsigned int n)
{
	return (x << n) | (x >> (32 - n));
}

static void sha1_forward(unsigned int nr_rounds, uint32_t w[80], uint32_t h_out[5])
{
	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xEFCDAB89;
	uint32_t h2 = 0x98BADCFE;
	uint32_t h3 = 0x10325476;
	uint32_t h4 = 0xC3D2E1F0;

	for (unsigned int i = 16; i < nr_rounds; ++i)
		w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

	uint32_t a = h0;
	uint32_t b = h1;
	uint32_t c = h2;
	uint32_t d = h3;
	uint32_t e = h4;

	for (unsigned int i = 0; i < nr_rounds; ++i) {
		uint32_t f, k;

		if (i < 20) {
			f = (b & c) | (~b & d);
			k = 0x5A827999;
		} else if (i >= 20 && i < 40) {
			f = b ^ c ^ d;
			k = 0x6ED9EBA1;
		} else if (i >= 40 && i < 60) {
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
		} else if (i >= 60 && i < 80) {
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
		} else {
            assert(false);
		}

		uint32_t t = rotl(a, 5) + f + e + k + w[i];
		e = d;
		d = c;
		c = rotl(b, 30);
		b = a;
		a = t;
	}

	h_out[0] = h0 + a;
	h_out[1] = h1 + b;
	h_out[2] = h2 + c;
	h_out[3] = h3 + d;
	h_out[4] = h4 + e;
}

static void preimage()
{
	sha1 f(config_nr_rounds, "");

	/* Generate a known-valid (messsage, hash)-pair */
	uint32_t w[80];
	for (unsigned int i = 0; i < 16; ++i) {
		w[i] = ((uint32_t)(rand() & 0xffff));
		w[i] |= ((uint32_t)(rand() & 0xffff)) << 16;
    }

	uint32_t h[5];
	sha1_forward(config_nr_rounds, w, h);

	/* Fix message bits */
	comment(format("Fix $ message bits", config_nr_message_bits));

	std::vector<unsigned int> message_bits(512);
	for (unsigned int i = 0; i < 512; ++i)
		message_bits[i] = i;

	std::random_shuffle(message_bits.begin(), message_bits.end());
	for (unsigned int i = 0; i < config_nr_message_bits; ++i) {
		unsigned int r = message_bits[i] / 32;
		unsigned int s = message_bits[i] % 32;

		constant(f.w[r][s], (w[r] >> s) & 1);
	}

	/* Fix hash bits */
	comment(format("Fix $ hash bits", config_nr_hash_bits));

	if (all_zero_output) {
		for (unsigned int i = 0; i < config_nr_hash_bits; ++i) {
			constant(f.h_out[i/32][i%32], 0);
		}
	} else {
		std::vector<unsigned int> hash_bits(160);
		for (unsigned int i = 0; i < 160; ++i)
			hash_bits[i] = i;

		std::random_shuffle(hash_bits.begin(), hash_bits.end());
		for (unsigned int i = 0; i < config_nr_hash_bits; ++i) {
			unsigned int r = hash_bits[i] / 32;
			unsigned int s = hash_bits[i] % 32;

			constant(f.h_out[r][s], (h[r] >> s) & 1);
		}
	}
}

/* The second preimage differs from the first preimage by flipping one of
 * the message bits. */
static void second_preimage()
{
	sha1 f(config_nr_rounds, "");

	/* Generate a known-valid (messsage, hash)-pair */
	uint32_t w[80];
	for (unsigned int i = 0; i < 16; ++i) {
		w[i] = ((uint32_t)(rand() & 0xffff));
		w[i] |= ((uint32_t)(rand() & 0xffff)) << 16;
	}

	uint32_t h[5];
	sha1_forward(config_nr_rounds, w, h);

	/* Fix message bits */
	comment(format("Fix $ message bits", config_nr_message_bits));

	std::vector<unsigned int> message_bits(512);
	for (unsigned int i = 0; i < 512; ++i)
		message_bits[i] = i;

	std::random_shuffle(message_bits.begin(), message_bits.end());

	/* Flip the first bit */
	if (config_nr_message_bits > 0) {
		unsigned int r = message_bits[0] / 32;
		unsigned int s = message_bits[0] % 32;

		constant(f.w[r][s], !((w[r] >> s) & 1));
	}

	for (unsigned int i = 1; i < config_nr_message_bits; ++i) {
		unsigned int r = message_bits[i] / 32;
		unsigned int s = message_bits[i] % 32;

		constant(f.w[r][s], (w[r] >> s) & 1);
	}

	/* Fix hash bits */
	comment(format("Fix $ hash bits", config_nr_hash_bits));

	std::vector<unsigned int> hash_bits(160);
	for (unsigned int i = 0; i < 160; ++i)
		hash_bits[i] = i;

	std::random_shuffle(hash_bits.begin(), hash_bits.end());
	for (unsigned int i = 0; i < config_nr_hash_bits; ++i) {
		unsigned int r = hash_bits[i] / 32;
		unsigned int s = hash_bits[i] % 32;

		constant(f.h_out[r][s], (h[r] >> s) & 1);
	}
}

static void collision()
{
	sha1 f(config_nr_rounds, "0");
	sha1 g(config_nr_rounds, "1");

	if (config_nr_message_bits > 0)
		std::cerr << "warning: collision attacks do not use fixed message bits\n";

	/* Fix message bits (set m != m') */
	comment(format("Fix $ message bits", config_nr_message_bits));

	std::vector<unsigned int> message_bits(512);
	for (unsigned int i = 0; i < 512; ++i)
		message_bits[i] = i;

	std::random_shuffle(message_bits.begin(), message_bits.end());

	/* Flip some random bit */
	{
		unsigned int r = message_bits[0] / 32;
		unsigned int s = message_bits[0] % 32;

		neq(&f.w[r][s], &g.w[r][s], 1);
	}

	/* Fix hash bits (set H = H') */
	comment(format("Fix $ hash bits", config_nr_hash_bits));

	std::vector<unsigned int> hash_bits(160);
	for (unsigned int i = 0; i < 160; ++i)
		hash_bits[i] = i;

	std::random_shuffle(hash_bits.begin(), hash_bits.end());
	for (unsigned int i = 0; i < config_nr_hash_bits; ++i) {
		unsigned int r = hash_bits[i] / 32;
		unsigned int s = hash_bits[i] % 32;

		eq(&f.h_out[r][s], &g.h_out[r][s], 1);
	}
}

int main(int argc, char *argv[])
{
	unsigned long seed = time(0);

	/* Process command line */
	{
		using namespace boost::program_options;

		options_description options("Options");
		options.add_options()
			("help,h", "Display this information")
		;

		options_description instance_options("Instance options");
		instance_options.add_options()
			("seed", value<unsigned long>(&seed), "Random number seed")
			("attack", value<std::string>(), "Attack type (preimage, second-preimage, collision)")
			("rounds", value<unsigned int>(&config_nr_rounds), "Number of rounds (16-80)")
			("message-bits", value<unsigned int>(&config_nr_message_bits), "Number of fixed message bits (0-512)")
			("hash-bits", value<unsigned int>(&config_nr_hash_bits), "Number of fixed hash bits (0-160)")
			("zero", bool_switch(&all_zero_output), "When doing preimage attack, hash output should be zero")
		;

		options_description format_options("Format options");
		format_options.add_options()
			("tseitin-adders", "Use Tseitin encoding of the circuit representation of adders")
			("nocomment", "Don't add comments");
		;

		options_description cnf_options("CNF-specific options");
		cnf_options.add_options()
			("xor", "Use XOR clauses")
			("restrict-branching", "Restrict branching variables to message bits")
		;

		options_description all_options;
		all_options.add(options);
		all_options.add(instance_options);
		all_options.add(format_options);
		all_options.add(cnf_options);

		positional_options_description p;
		p.add("input", -1);

		variables_map map;
		store(command_line_parser(argc, argv)
			.options(all_options)
			.positional(p)
			.run(), map);
		notify(map);

		if (map.count("help")) {
			std::cout << all_options;
			return 0;
		}

		if (map.count("nocomment")) {
			nocomment = true;
		}

		if (map.count("attack") == 1) {
			config_attack = map["attack"].as<std::string>();
		} else if (map.count("attack") > 1) {
			std::cerr << "Can only specify --attack once\n";
			return EXIT_FAILURE;
		}

		if (config_attack != "preimage" && config_attack != "second-preimage" && config_attack != "collision") {
			std::cerr << "Invalid --attack\n";
			return EXIT_FAILURE;
		}

		if (config_attack != "preimage" && all_zero_output) {
			std::cerr << "You can only zero out the output for preimage attacks" << std::endl;
			return EXIT_FAILURE;
		}

		if (map.count("xor"))
			config_use_xor_clauses = true;

		if (map.count("restrict-branching"))
			config_restrict_branching = true;
	}

	comment("");
	comment("Instance generated by sha1-sat");
	comment("Written by Vegard Nossum <vegard.nossum@gmail.com>");
	comment("<https://github.com/vegard/sha1-sat>");
	comment("");

	/* Include command line in instance */
	{
		std::ostringstream ss;

		ss << argv[0];

		for (int i = 1; i < argc; ++i) {
			ss << " ";
			ss << argv[i];
		}

		comment(format("command line: $", ss.str()));
	}

	comment(format("parameter seed = $", seed));
	srand(seed);

	if (config_attack == "preimage") {
		preimage();
	} else if (config_attack == "second-preimage") {
		second_preimage();
	} else if (config_attack == "collision") {
		collision();
	}

	std::cout
	<< format("p cnf $ $\n", nr_variables, nr_clauses)
	<< cnf.str();

	return 0;
}
