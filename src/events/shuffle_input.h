#ifndef RE_EVENTS_SHUFFLE_INPUT_H
#define RE_EVENTS_SHUFFLE_INPUT_H

// Shuffle A, B, Z buttons to random positions for 5-30 seconds.

#define SHUFFLE_MIN 150
#define SHUFFLE_MAX 900

static const u16 kShuffleBtns[3]  = { A_BUTTON,   B_BUTTON,   Z_TRIG      };
static const u16 kShuffleCBtns[4] = { U_CBUTTONS, D_CBUTTONS, L_CBUTTONS, R_CBUTTONS };

static int sShuffleFrames      = 0;
static u16 sShuffleBtnMap[3];  // A/B/Z remapping
static u16 sShuffleCMap[4];    // C-button remapping
static int sShuffleStickFlipX  = 0; // invert X axis
static int sShuffleStickFlipY  = 0; // invert Y axis
static int sShuffleStickSwap   = 0; // swap X and Y

static void fisher_yates(int *arr, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = (int)(rng_next() % (unsigned int)(i + 1));
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

static void do_shuffle_input(struct MarioState *m) {
    (void)m;
    sShuffleFrames = rng_range(SHUFFLE_MIN, SHUFFLE_MAX);

    int order3[3] = { 0, 1, 2 };
    fisher_yates(order3, 3);
    for (int i = 0; i < 3; i++) sShuffleBtnMap[i] = kShuffleBtns[order3[i]];

    int order4[4] = { 0, 1, 2, 3 };
    fisher_yates(order4, 4);
    for (int i = 0; i < 4; i++) sShuffleCMap[i] = kShuffleCBtns[order4[i]];

    // Randomly pick a stick transformation (any of 8 combos).
    u32 r = rng_next() % 8;
    sShuffleStickSwap  = (r >> 2) & 1;
    sShuffleStickFlipX = (r >> 1) & 1;
    sShuffleStickFlipY = (r >> 0) & 1;
}

static void apply_shuffle_input(struct Controller *c) {
    if (sShuffleFrames <= 0) return;

    // --- Buttons: A/B/Z ---
    u16 orig_down    = c->buttonDown;
    u16 orig_pressed = c->buttonPressed;
    u16 btn_mask     = A_BUTTON | B_BUTTON | Z_TRIG;
    u16 base_down    = orig_down    & ~btn_mask;
    u16 base_pressed = orig_pressed & ~btn_mask;
    for (int i = 0; i < 3; i++) {
        if (orig_down    & kShuffleBtns[i]) base_down    |= sShuffleBtnMap[i];
        if (orig_pressed & kShuffleBtns[i]) base_pressed |= sShuffleBtnMap[i];
    }

    // --- C-buttons ---
    u16 c_mask = U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS;
    u16 cbtn_orig_down    = orig_down    & c_mask;
    u16 cbtn_orig_pressed = orig_pressed & c_mask;
    base_down    &= ~c_mask;
    base_pressed &= ~c_mask;
    for (int i = 0; i < 4; i++) {
        if (cbtn_orig_down    & kShuffleCBtns[i]) base_down    |= sShuffleCMap[i];
        if (cbtn_orig_pressed & kShuffleCBtns[i]) base_pressed |= sShuffleCMap[i];
    }

    c->buttonDown    = base_down;
    c->buttonPressed = base_pressed;

    // --- Stick ---
    f32 sx = c->stickX;
    f32 sy = c->stickY;
    if (sShuffleStickSwap)  { f32 t = sx; sx = sy; sy = t; }
    if (sShuffleStickFlipX) sx = -sx;
    if (sShuffleStickFlipY) sy = -sy;
    c->stickX = sx;
    c->stickY = sy;

    sShuffleFrames--;
}

#endif // RE_EVENTS_SHUFFLE_INPUT_H
