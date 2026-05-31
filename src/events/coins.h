#ifndef RE_EVENTS_COINS_H
#define RE_EVENTS_COINS_H

static void do_coin_bonus(struct MarioState *m) {
    int n = rng_range(5, 20);
    m->numCoins += (s16)n;
    if (m->numCoins > 999) m->numCoins = 999;
}

static void do_coin_penalty(struct MarioState *m) {
    int n = rng_range(5, 15);
    m->numCoins -= (s16)n;
    if (m->numCoins < 0) m->numCoins = 0;
}

#endif // RE_EVENTS_COINS_H
