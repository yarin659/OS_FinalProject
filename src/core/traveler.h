#ifndef OSFINALPROJECT_TRAVELER_H
#define OSFINALPROJECT_TRAVELER_H

#include "common.h"
#include <sys/types.h>

struct dijkstra_result_t;
struct anim_state;

struct traveler_t {
    int dijkstra_src;
    int dijkstra_dest;
    struct dijkstra_result_t* dijkstra_result;

    struct graph_t* graph;
    struct anim_state* anim;

    BOOL has_subprocess;
    pid_t subprocess_pid;
};

pid_t traveler_create_subprocess(struct traveler_t* traveler);
void traveler_destroy_subprocess(struct traveler_t* traveler);
void traveler_free(struct traveler_t* traveler);
int traveler_main(pid_t pid);

#endif //OSFINALPROJECT_TRAVELER_H
