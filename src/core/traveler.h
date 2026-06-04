#ifndef OSFINALPROJECT_TRAVELER_H
#define OSFINALPROJECT_TRAVELER_H

#include "common.h"
#include <sys/types.h>

struct dijkstra_result_t;
struct anim_state;

enum ipc_msg_type { 
    MSG_ARRIVED, 
    MSG_FINISHED 
};

struct ipc_msg_t {
    pid_t pid;
    int traveler_index;
    int type;          
    int current_node;
    int next_node;     
};

struct traveler_t {
    int dijkstra_src;
    int dijkstra_dest;
    struct dijkstra_result_t* dijkstra_result;

    struct graph_t* graph;
    struct anim_state* anim;

    BOOL has_subprocess;
    pid_t subprocess_pid;
    int traveler_index; 
};

pid_t traveler_create_subprocess(struct traveler_t* traveler, int index, int write_fd);
void traveler_destroy_subprocess(struct traveler_t* traveler);
void traveler_free(struct traveler_t* traveler);
int traveler_main(struct traveler_t* traveler, int write_fd);

#endif //OSFINALPROJECT_TRAVELER_H
