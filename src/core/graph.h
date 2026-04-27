#ifndef OSFINALPROJECT_GRAPH_H
#define OSFINALPROJECT_GRAPH_H

#include "common.h"

struct graph_t {
    // TODO
    int temp;
};

BOOL load_graph_from_file(const char* filename, struct graph_t* out_graph);
BOOL load_graph_from_string(const char* graph_string, struct graph_t* out_graph);

#endif //OSFINALPROJECT_GRAPH_H
