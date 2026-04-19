#ifndef OSFINALPROJECT_GRAPH_H
#define OSFINALPROJECT_GRAPH_H

#include "common.h"

typedef struct {
    // TODO
} Graph;

BOOL load_graph_from_file(const char* filename, Graph* out_graph);
BOOL load_graph_from_string(const char* graph_string, Graph* out_graph);

#endif //OSFINALPROJECT_GRAPH_H
