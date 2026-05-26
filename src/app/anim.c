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

    traveler->anim = anim;

    // No path
    if (traveler->dijkstra_result->path_len == 0) {
        anim->phase = ANIM_DONE;
        return;
    }

    // place entity at source vertex
    anim->entity_pos  = positions[traveler->dijkstra_result->path[0]];
    anim->seg_index   = 0;
    anim->jump_index  = 0;
    anim->phase_timer = 0.f;

    if (traveler->dijkstra_result->path_len == 1) {
        anim->phase = ANIM_DONE;
    } else {
        const int u = traveler->dijkstra_result->path[0];
        const int v = traveler->dijkstra_result->path[1];
        anim->jump_total = traveler->graph->graph[u][v];
        anim->jump_index = 0;
        anim->phase = ANIM_ON_EDGE;
        anim->phase_timer = 0.f;
    }
}

void anim_stop(struct traveler_t* traveler) {
    free(traveler->anim);
    traveler->anim = NULL;
}

void anim_update(const struct traveler_t *traveler, Vector2 positions[MAX_VERTICES], const float dt) {
    struct anim_state* anim = traveler->anim;

    if (anim->phase == ANIM_IDLE || anim->phase == ANIM_DONE) {
        return;
    }

    anim->phase_timer += dt;

    if (anim->phase == ANIM_ON_EDGE) {
        const float jump_sec = JUMP_DURATION_MS / 1000.f;

        float t = anim->phase_timer / jump_sec;
        if (t > 1.f) t = 1.f;

        const int u = traveler->dijkstra_result->path[anim->seg_index];
        const int v = traveler->dijkstra_result->path[anim->seg_index + 1];
        const int W = anim->jump_total;

        // linear interpolation :D
        const float jump_start_t = (float) anim->jump_index / (float)W;
        const float jump_end_t = (float)(anim->jump_index + 1) / (float)W;
        const float lerp_t = jump_start_t + (jump_end_t - jump_start_t) * t;
        anim->entity_pos = Vector2Lerp(positions[u], positions[v], lerp_t);

        // Jump completed?
        if (anim->phase_timer >= jump_sec) {
            anim->jump_index++;
            anim->phase_timer = 0.f;

            // Finished the entire edge so snap to destination
            if (anim->jump_index >= W) {
                anim->entity_pos = positions[v];
                anim->seg_index++;

                // Final destination = done, otherwise, reset timer so we wait 1s
                if (anim->seg_index >= traveler->dijkstra_result->path_len - 1) {
                    anim->phase = ANIM_DONE;
                } else {
                    anim->phase = ANIM_AT_VERTEX;
                    anim->phase_timer = 0.f;
                }
            }
        }
    } else if (anim->phase == ANIM_AT_VERTEX) {
        // wait the whole 1s...
        if (anim->phase_timer >= VERTEX_WAIT_MS / 1000.f) {
            const int u = traveler->dijkstra_result->path[anim->seg_index];
            const int v = traveler->dijkstra_result->path[anim->seg_index + 1];
            anim->jump_total = traveler->graph->graph[u][v];
            anim->jump_index = 0;
            anim->phase = ANIM_ON_EDGE;
            anim->phase_timer = 0.f;
        }
    }
}
