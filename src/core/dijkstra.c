#include "dijkstra.h"

#include <stdlib.h>

BOOL dijkstra_compute(const struct graph_t *graph, struct dijkstra_result_t *out_result) {
    out_result->path = NULL;
    out_result->path_len = 0;
    out_result->total_dist = 0;

    const int n = graph->vertex_count;
    const int src = graph->dijkstra_src;
    const int dest = graph->dijkstra_dest;

    if (src == dest) {
        out_result->path = malloc(sizeof(int));
        if (!out_result->path) {
            return FALSE;
        }
        out_result->path[0] = src;
        out_result->path_len = 1;
        return TRUE;
    }

    int *dist = malloc(n * sizeof(int));
    int *visited = malloc(n * sizeof(int));
    int *prev = malloc(n * sizeof(int));

    if (!dist || !visited || !prev) {
        free(dist);
        free(visited);
        free(prev);
        return FALSE;
    }

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        visited[i] = 0;
        prev[i] = -1;
    }

    dist[src] = 0;

    for (int i = 0; i < n; i++) {
        int min_dist = INF, u = -1;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        if (u == -1 || u == dest) break;
        visited[u] = 1;

        for (int v = 0; v < n; v++) {
            if (!visited[v] && graph->graph[u][v] != -1 && dist[u] != INF) {
                const int alt = dist[u] + graph->graph[u][v];
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
    }

    // no path found
    if (dist[dest] == INF) {
        free(dist);
        free(visited);
        free(prev);
        return TRUE;
    }

    out_result->total_dist = dist[dest];

    int *tmp = malloc(n * sizeof(int));
    if (!tmp) {
        free(dist);
        free(visited);
        free(prev);
        return FALSE;
    }

    int curr = dest, len = 0;
    while (curr != -1) {
        tmp[len++] = curr;
        curr = prev[curr];
    }

    out_result->path = malloc(len * sizeof(int));
    if (!out_result->path) {
        free(tmp);
        free(dist);
        free(visited);
        free(prev);
        return FALSE;
    }

    for (int i = 0; i < len; i++) {
        out_result->path[i] = tmp[len - 1 - i];
    }
    out_result->path_len = len;

    free(tmp);
    free(dist);
    free(visited);
    free(prev);
    return TRUE;
}

void free_dijkstra_result(struct dijkstra_result_t *result) {
    free(result->path);
    result->path = NULL;
    result->path_len = 0;
}

