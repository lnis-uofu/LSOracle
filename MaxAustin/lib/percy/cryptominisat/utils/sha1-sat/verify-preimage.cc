#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cassert>

uint32_t rotl(uint32_t x, unsigned int n)
{
    return (x << n) | (x >> (32 - n));
}

void sha1(unsigned int nr_rounds,
    uint32_t &h0,
    uint32_t &h1,
    uint32_t &h2,
    uint32_t &h3,
    uint32_t &h4,
    uint32_t w[80])
{
    uint32_t a = h0;
    uint32_t b = h1;
    uint32_t c = h2;
    uint32_t d = h3;
    uint32_t e = h4;

    for (unsigned int i = 16; i < 80; ++i) {
        w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    for (unsigned int i = 0; i < nr_rounds; ++i) {
        uint32_t f, k;

        if (i < 20) {
            f = (b & c) | (~b & d);
            k = uint32_t(0x5a827999U);
        } else if (i >= 20 && i < 40) {
            f = b ^ c ^ d;
            k = uint32_t(0x6ed9eba1U);
        } else if (i >= 40 && i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = uint32_t(0x8f1bbcdcU);
        } else if (i >= 60 && i < 80) {
            f = b ^ c ^ d;
            k = uint32_t(0xca62c1d6U);
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

    h0 = h0 + a;
    h1 = h1 + b;
    h2 = h2 + c;
    h3 = h3 + d;
    h4 = h4 + e;
}

int main()
{
    unsigned int nr_rounds;
    int ret;
    ret = scanf("%u", &nr_rounds);
    if (ret < 1 || ret == EOF) {
        exit(EXIT_FAILURE);
    }

    uint32_t w[80];
    for (unsigned int i = 0; i < 16; ++i) {
        ret = scanf("%08x", &w[i]);
        if (ret < 1 || ret == EOF) {
            exit(EXIT_FAILURE);
        }
    }

    uint32_t H[5];
    for (unsigned int i = 0; i < 5; ++i) {
        ret = scanf("%08x", &H[i]);
        if (ret < 1 || ret == EOF) {
            exit(EXIT_FAILURE);
        }
    }

    uint32_t h[5];
    for (unsigned int i = 0; i < 5; ++i) {
        ret = scanf("%08x", &h[i]);
        if (ret < 1 || ret == EOF) {
            exit(EXIT_FAILURE);
        }
    }

    sha1(nr_rounds, H[0], H[1], H[2], H[3], H[4], w);

    for (unsigned int i = 0; i < 5; ++i) {
        printf("%08x %08x %s\n", h[i], H[i], h[i] == H[i] ? "correct" : "incorrect");
    }

    for (unsigned int i = 0; i < 5; ++i) {
        if (h[i] != H[i]) {
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
