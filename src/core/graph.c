#include "graph.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "traveler.h"

BOOL load_graph_from_file(const char *filename, struct graph_t *out_graph) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("load_graph_from_file: failed to open file '%s'\n", filename);
        return FALSE;
    }

    fseek(file, 0, SEEK_END);
    const size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > 1024) {
        printf("load_graph_from_file: file size exceeds 1024 bytes\n");
        fclose(file);
        return FALSE;
    }

    char buffer[1025];
    const size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        printf("load_graph_from_file: failed to read from file '%s'\n", filename);
        fclose(file);
        return FALSE;
    }

    buffer[bytes_read] = '\0';

    fclose(file);
    return load_graph_from_string(buffer, out_graph);
}

BOOL load_graph_from_string(const char *graph_string, struct graph_t *out_graph) {
    // Make a copy so we don't destruct the original graph_string
    const size_t len = strlen(graph_string);
    char* input_buffer = malloc(len + 1);

    // just in case malloc fails
    if (input_buffer == NULL) {
        printf("load_graph_from_string: failed to allocate memory for input buffer\n");
        return FALSE;
    }

    strncpy(input_buffer, graph_string, len + 1);

    char* next_token = NULL;

    const char* line = strtok_r(input_buffer, "\r\n", &next_token);
    if (line == NULL) {
        printf("load_graph_from_string: missing first line\n");
        free(input_buffer);
        return FALSE;
    }

    // First line handling - read out the vertex and edge count
    if (sscanf(line, "%d %d", &out_graph->vertex_count, &out_graph->edge_count) != 2) {
        printf("load_graph_from_string: invalid vertex/edge count\n");
        free(input_buffer);
        return FALSE;
    }

    out_graph->graph = malloc(out_graph->vertex_count * sizeof(int *));
    if (out_graph->graph == NULL) {
        printf("load_graph_from_string: failed to allocate memory for graph structure (1)\n");
        free(input_buffer);
        return FALSE;
    }

    for (int i = 0; i < out_graph->vertex_count; ++i) {
        out_graph->graph[i] = malloc(out_graph->vertex_count * sizeof(int));
        if (out_graph->graph[i] == NULL) {
            printf("load_graph_from_string: failed to allocate memory for graph structure (2)\n");

            // it's OK for us to call free on null ptrs supposedly, so no issue here.
            // if needed, we can track how much we allocated so far and free based on that.
            free_graph(out_graph);
            free(input_buffer);
            return FALSE;
        }

        for (int j = 0; j < out_graph->vertex_count; ++j)
            out_graph->graph[i][j] = -1;
    }

    // src-dst-weight handling to build the graph structure
    for (int i = 0; i < out_graph->edge_count; ++i) {
        line = strtok_r(NULL, "\r\n", &next_token);
        if (line == NULL) {
            printf("load_graph_from_string: missing edge info line\n");
            free_graph(out_graph);
            free(input_buffer);
            return FALSE;
        }

        int src, dst, weight;
        if (sscanf(line, "%d %d %d", &src, &dst, &weight) != 3) {
            printf("load_graph_from_string: invalid edge info line\n");
            free_graph(out_graph);
            free(input_buffer);
            return FALSE;
        }

        if (src < 0 || src >= out_graph->vertex_count || dst < 0 || dst >= out_graph->vertex_count) {
            printf("load_graph_from_string: invalid vertex index\n");
            free_graph(out_graph);
            free(input_buffer);
            return FALSE;
        }

        out_graph->graph[src][dst] = weight;
    }

    line = strtok_r(NULL, "\r\n", &next_token);
    if (line == NULL) {
        printf("load_graph_from_string: expected traveler count line, got EOF.\n");
        free_graph(out_graph);
        free(input_buffer);
        return FALSE;
    }

    int traveler_count;
    if (sscanf(line, "%d", &traveler_count) != 1) {
        printf("load_graph_from_string: missing or invalid traveler count.\n");
        free_graph(out_graph);
        free(input_buffer);
        return FALSE;
    }
    out_graph->traveler_count = traveler_count;

    out_graph->travelers = malloc(traveler_count * sizeof(struct traveler_t));
    if (out_graph->travelers == NULL) {
        printf("load_graph_from_string: failed to allocate memory for traveler descriptors\n");
        free_graph(out_graph);
        free(input_buffer);
        return FALSE;
    }

    for (int i = 0; i < traveler_count; ++i) {
        line = strtok_r(NULL, "\r\n", &next_token);
        if (line == NULL) {
            printf("load_graph_from_string: expected traveler info line for %d, got EOF.\n", i);
            free_graph(out_graph);
            free(input_buffer);
            return FALSE;
        }

        struct traveler_t traveler_descriptor = {0};
        traveler_descriptor.graph = out_graph;
        if (sscanf(line, "%d %d", &traveler_descriptor.dijkstra_src, &traveler_descriptor.dijkstra_dest) != 2) {
            printf("load_graph_from_string: invalid traveler info line\n");
            free_graph(out_graph);
            free(input_buffer);
            return FALSE;
        }

        out_graph->travelers[i] = traveler_descriptor;
    }

    free(input_buffer);
    return TRUE;
}

void free_graph(const struct graph_t *graph) {
    for (int i = 0; i < graph->vertex_count; ++i) {
        free(graph->graph[i]);
    }

    for (int i = 0; i < graph->traveler_count; ++i) {
        traveler_free(&graph->travelers[i]);
    }
    free(graph->travelers);

    free(graph->graph);
}

void compute_graph_positions(const struct graph_t* graph, const Vector2 center, const float radius,
                              Vector2 positions[MAX_VERTICES]) {
    for (int i = 0; i < graph->vertex_count; i++) {
        const float angle = (2.0f * PI / (float)graph->vertex_count) * (float)i;
        positions[i].x = center.x + cosf(angle) * radius;
        positions[i].y = center.y + sinf(angle) * radius;
    }
}

