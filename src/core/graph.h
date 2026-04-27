#ifndef OSFINALPROJECT_GRAPH_H
#define OSFINALPROJECT_GRAPH_H

#include "common.h"

struct graph_t {
    int vertex_count;
    int edge_count;
    int** graph;

    int dijkstra_src;
    int dijkstra_dest;
};

BOOL load_graph_from_file(const char* filename, struct graph_t* out_graph);
BOOL load_graph_from_string(const char* graph_string, struct graph_t* out_graph);
void free_graph(const struct graph_t* graph);

#endif //OSFINALPROJECT_GRAPH_H
