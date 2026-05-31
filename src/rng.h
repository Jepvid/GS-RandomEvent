#ifndef RE_RNG_H
#define RE_RNG_H

static unsigned int sRandState = 0xA5CF9B3E;

static unsigned int rng_next(void) {
    sRandState ^= sRandState << 13;
    sRandState ^= sRandState >> 17;
    sRandState ^= sRandState << 5;
    return sRandState;
}

static int rng_range(int lo, int hi) {
    return lo + (int)(rng_next() % (unsigned int)(hi - lo + 1));
}

#endif // RE_RNG_H
