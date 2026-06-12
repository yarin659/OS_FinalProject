#ifndef OSFINALPROJECT_ANIM_H
#define OSFINALPROJECT_ANIM_H

#include "raylib.h"
#include "core/config.h"
#include "core/graph.h"

enum anim_phase {
    ANIM_IDLE,
    ANIM_WAITING_OUTSIDE,
    ANIM_AT_VERTEX,
    ANIM_ON_EDGE,
    ANIM_DONE
};

struct anim_state {
    enum anim_phase phase;
    int     current_u;
    int     current_v;
    int     waiting_node; // vertex we're waiting for
    float   phase_timer;
    float   total_edge_time;
    Vector2 entity_pos;
};

void anim_start(struct traveler_t *traveler, Vector2 positions[MAX_VERTICES]);
void anim_stop(struct traveler_t *traveler);
void anim_update(struct traveler_t *traveler, Vector2 positions[MAX_VERTICES], float dt);

#endif /* OSFINALPROJECT_ANIM_H */
