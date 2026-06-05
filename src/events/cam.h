#ifndef RE_EVENTS_CAM_H
#define RE_EVENTS_CAM_H

#include "game/camera.h"
#include "engine/math_util.h"

// CameraUpdate fires after update_lakitu — overriding gLakituState here is authoritative.

#define CAM_DURATION 600

static RE_Timer sTopDownFrames = {0};
static RE_Timer sDjiFrames     = {0};
static RE_Timer sFpsFrames     = {0};
static u16      sDjiAngle      = 0;

static ListenerID sCamListenerID;

static void do_topdown(struct MarioState *m) {
    (void)m;
    re_timer_set(&sTopDownFrames, CAM_DURATION);
    re_timer_set(&sDjiFrames,     0);
    re_timer_set(&sFpsFrames,     0);
}

static void do_djicam(struct MarioState *m) {
    (void)m;
    re_timer_set(&sTopDownFrames, 0);
    re_timer_set(&sDjiFrames,     CAM_DURATION);
    re_timer_set(&sFpsFrames,     0);
    sDjiAngle = 0;
}

static void do_fpsmario(struct MarioState *m) {
    (void)m;
    re_timer_set(&sTopDownFrames, 0);
    re_timer_set(&sDjiFrames,     0);
    re_timer_set(&sFpsFrames,     CAM_DURATION);
}

// Sets all four lakitu fields so the smooth-approach system doesn't fight our override.
static void cam_set(f32 px, f32 py, f32 pz, f32 fx, f32 fy, f32 fz) {
    gLakituState.pos[0]      = px; gLakituState.pos[1]      = py; gLakituState.pos[2]      = pz;
    gLakituState.curPos[0]   = px; gLakituState.curPos[1]   = py; gLakituState.curPos[2]   = pz;
    gLakituState.focus[0]    = fx; gLakituState.focus[1]    = fy; gLakituState.focus[2]    = fz;
    gLakituState.curFocus[0] = fx; gLakituState.curFocus[1] = fy; gLakituState.curFocus[2] = fz;
}

static void on_camera_update(IEvent *event) {
    (void)event;
    if (!gMarioState) return;
    struct MarioState *m = gMarioState;

    if (re_timer_active(&sTopDownFrames)) {
        cam_set(
            m->pos[0], m->pos[1] + 1200.0f, m->pos[2],
            m->pos[0], m->pos[1],            m->pos[2]
        );
        re_timer_tick(&sTopDownFrames);

    } else if (re_timer_active(&sDjiFrames)) {
        sDjiAngle += 200;
        cam_set(
            m->pos[0] + sins(sDjiAngle) * 700.0f, m->pos[1] + 300.0f, m->pos[2] + coss(sDjiAngle) * 700.0f,
            m->pos[0],                             m->pos[1] + 80.0f,  m->pos[2]
        );
        re_timer_tick(&sDjiFrames);

    } else if (re_timer_active(&sFpsFrames)) {
        f32 headY = m->pos[1] + 120.0f;
        f32 ld    = 400.0f;
        cam_set(
            m->pos[0],
            headY,
            m->pos[2],
            m->pos[0] + sins(m->faceAngle[1]) * ld,
            headY,
            m->pos[2] + coss(m->faceAngle[1]) * ld
        );
        re_timer_tick(&sFpsFrames);
    }
}

static void register_cam(void) {
    sCamListenerID = REGISTER_LISTENER(CameraUpdate, EVENT_PRIORITY_NORMAL, on_camera_update);
}

static void unregister_cam(void) {
    UNREGISTER_LISTENER(CameraUpdate, sCamListenerID);
}

#endif // RE_EVENTS_CAM_H
