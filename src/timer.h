#ifndef RE_TIMER_H
#define RE_TIMER_H

// Countdown timer in frames. Zero means inactive.
typedef struct { int remaining; } RE_Timer;

static inline void re_timer_set(RE_Timer *t, int frames) { t->remaining = frames; }
// Decrement and return remaining frames. Safe to call when already 0.
static inline int  re_timer_tick(RE_Timer *t)            { return t->remaining > 0 ? --t->remaining : 0; }
static inline int  re_timer_active(const RE_Timer *t)    { return t->remaining > 0; }
// Fraction of total remaining: 1.0 = just started, 0.0 = expired.
static inline float re_timer_frac(const RE_Timer *t, int total) {
    return total > 0 ? (float)t->remaining / (float)total : 0.0f;
}

#endif // RE_TIMER_H
