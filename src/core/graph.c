#include "graph.h"
#include <stdio.h>

BOOL load_graph_from_file(const char *filename, Graph *out_graph) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return FALSE;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > 1024) {
        fclose(file);
        return FALSE;
    }

    char buffer[1024];
    if (fgets(buffer, 1024, file) == NULL) {
        fclose(file);
        return FALSE;
    }

    fclose(file);
    return load_graph_from_string(buffer, out_graph);
}

BOOL load_graph_from_string(const char *graph_string, Graph *out_graph) {
    // TODO
    return FALSE;
}
