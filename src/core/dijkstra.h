#ifndef OSFINALPROJECT_DIJKSTRA_H
#define OSFINALPROJECT_DIJKSTRA_H

#include "common.h"
#include "graph.h"

struct dijkstra_result_t {
    int* path; // array of vertex indices from src to dest (inclusive)
    int path_len; // number of vertices in the path or 0 if no path
    int total_dist;
};

BOOL dijkstra_compute(const struct graph_t* graph, int traveler_id);
void free_dijkstra_result(struct dijkstra_result_t* result);

#endif //OSFINALPROJECT_DIJKSTRA_H
