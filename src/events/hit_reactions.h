#ifndef RE_EVENTS_HIT_REACTIONS_H
#define RE_EVENTS_HIT_REACTIONS_H

// Two action-triggered effects:
//
// DODGE (~10%): undoes the hurtCounter damage in the PlayerHit listener (fires after
//   hurtCounter is incremented in interaction.c), cancels knockback in PlayerKnockback
//   by overwriting *action, then teleports Mario behind the enemy and punches in the
//   next GameFrameUpdate tick.
//
// SUPER KNOCKBACK (~15%): sets a pending flag in PlayerKnockback, applies a 3-8x
//   forwardVel multiplier in tick after the KB action has been written to m->action.

static ListenerID sHitListenerID;
static ListenerID sKnockbackListenerID;

static int   sDodgePending = 0;
static f32   sDodgeSrcX, sDodgeSrcY, sDodgeSrcZ;

static int   sPendingKnockback     = 0;
static f32   sPendingKnockbackMult = 1.0f;

static void on_player_hit(IEvent *event) {
    PlayerHit *e = (PlayerHit *)event;
    if (!e->source) return;
    if (rng_next() % 10 != 0) return; // ~10% dodge

    // Undo hurtCounter that was just added (interaction.c line: m->hurtCounter += 4*damage).
    u8 added = (u8)(4 * e->damage);
    e->m->hurtCounter = (e->m->hurtCounter >= added) ? e->m->hurtCounter - added : 0;

    sDodgeSrcX = e->source->oPosX;
    sDodgeSrcY = e->source->oPosY;
    sDodgeSrcZ = e->source->oPosZ;
    sDodgePending = 1;
}

static void on_player_knockback(IEvent *event) {
    PlayerKnockback *e = (PlayerKnockback *)event;

    // Dodge takes priority: cancel the knockback action entirely.
    if (sDodgePending) {
        *e->action = ACT_IDLE;
        return;
    }

    if (rng_next() % 7 != 0) return; // ~15% super knockback
    sPendingKnockback     = 1;
    sPendingKnockbackMult = (f32)rng_range(3, 8);
}

static void tick_hit_reactions(struct MarioState *m) {
    if (sDodgePending) {
        // Teleport Mario behind the enemy using the cheap Chebyshev normaliser.
        f32 dx   = sDodgeSrcX - m->pos[0];
        f32 dz   = sDodgeSrcZ - m->pos[2];
        f32 adx  = dx < 0.0f ? -dx : dx;
        f32 adz  = dz < 0.0f ? -dz : dz;
        f32 norm = adx > adz ? adx : adz;
        if (norm > 0.0f) {
            f32 scale  = 150.0f / norm;
            m->pos[0]  = sDodgeSrcX + dx * scale;
            m->pos[2]  = sDodgeSrcZ + dz * scale;
        }
        m->pos[1]       = sDodgeSrcY;
        m->faceAngle[1] += 0x8000; // face back toward enemy
        set_mario_action(m, ACT_PUNCHING, 0);
        sDodgePending = 0;
    }

    if (sPendingKnockback) {
        u32 action = m->action;
        if (action == ACT_BACKWARD_AIR_KB   ||
            action == ACT_FORWARD_AIR_KB    ||
            action == ACT_HARD_BACKWARD_AIR_KB ||
            action == ACT_HARD_FORWARD_AIR_KB) {
            m->forwardVel   *= sPendingKnockbackMult;
            sPendingKnockback = 0;
        }
    }
}

static void register_hit_reactions(void) {
    sHitListenerID       = REGISTER_LISTENER(PlayerHit,       EVENT_PRIORITY_HIGH, on_player_hit);
    sKnockbackListenerID = REGISTER_LISTENER(PlayerKnockback, EVENT_PRIORITY_HIGH, on_player_knockback);
}

static void unregister_hit_reactions(void) {
    UNREGISTER_LISTENER(PlayerHit,       sHitListenerID);
    UNREGISTER_LISTENER(PlayerKnockback, sKnockbackListenerID);
}

#endif // RE_EVENTS_HIT_REACTIONS_H
