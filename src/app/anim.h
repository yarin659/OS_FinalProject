#ifndef OSFINALPROJECT_ANIM_H
#define OSFINALPROJECT_ANIM_H

#include "raylib.h"
#include "core/config.h"
#include "core/dijkstra.h"

enum anim_phase {
    ANIM_IDLE,      // Not started yet
    ANIM_AT_VERTEX, // Waiting on a vertex
    ANIM_ON_EDGE,   // Moving along an edge
    ANIM_DONE       // Arrived at destination
};

struct anim_state {
    enum anim_phase phase;
    struct dijkstra_result_t result;

    // Progress markings
    int seg_index; // segment of path overall
    int jump_index; // jump id within the edge
    int jump_total; // basically cached current edge weight

    float phase_timer;
    Vector2 entity_pos;
};

void anim_start(struct anim_state *anim, const struct graph_t *graph, Vector2 positions[MAX_VERTICES]);
void anim_stop(struct anim_state *anim);
void anim_update(struct anim_state *anim, const struct graph_t *graph, Vector2 positions[MAX_VERTICES], float dt);

#endif //OSFINALPROJECT_ANIM_H
