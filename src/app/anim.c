#include "anim.h"
#include <stdlib.h>
#include <string.h>
#include "raymath.h"
#include "core/traveler.h"

void anim_start(struct traveler_t* traveler, Vector2 positions[MAX_VERTICES]) {
    if (traveler->anim != NULL) {
        anim_stop(traveler);
    }

    struct anim_state* anim = malloc(sizeof(struct anim_state));
    memset(anim, 0, sizeof(struct anim_state));
    anim->phase = ANIM_IDLE;
    
    anim->entity_pos = positions[traveler->dijkstra_src];
    traveler->anim = anim;
}

void anim_stop(struct traveler_t* traveler) {
    if (traveler->anim) {
        free(traveler->anim);
        traveler->anim = NULL;
    }
}

void anim_update(struct traveler_t *traveler, Vector2 positions[MAX_VERTICES], const float dt) {
    struct anim_state* anim = traveler->anim;
    if (!anim || anim->phase != ANIM_ON_EDGE) {
        return;
    }

    anim->phase_timer += dt;
    float t = anim->phase_timer / anim->total_edge_time;
    if (t > 1.f) t = 1.f;

    anim->entity_pos = Vector2Lerp(positions[anim->current_u], positions[anim->current_v], t);
}
