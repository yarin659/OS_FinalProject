#ifndef OSFINALPROJECT_GRAPH_H
#define OSFINALPROJECT_GRAPH_H

#include "common.h"
#include "config.h"
#include "raylib.h"

struct traveler_t;

struct graph_t {
    int vertex_count;
    int edge_count;
    int** graph;

    int traveler_count;
    struct traveler_t* travelers;
};

BOOL load_graph_from_file(const char* filename, struct graph_t* out_graph);
BOOL load_graph_from_string(const char* graph_string, struct graph_t* out_graph);
void free_graph(const struct graph_t* graph);

void compute_graph_positions(const struct graph_t *graph, Vector2 center, float radius,
                             Vector2 positions[MAX_VERTICES]);

#endif //OSFINALPROJECT_GRAPH_H
