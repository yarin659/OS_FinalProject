#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <memory.h>

#include "raylib.h"
#include "core/common.h"
#include "core/graph.h"
#include "render/draw.h"

void dijkstra(const struct graph_t *graph);

int main(int argc, char *argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    struct graph_t graph = {};
    memset(&graph, 0, sizeof(struct graph_t));

    if (!load_graph_from_file("data/sample_graph.dat", &graph)) {
        printf("Error: Could not load graph from file (data/sample_graph.dat); invalid CMake config?\n");
        return 1;
    }

    const int screen_width = 900;
    const int screen_height = 550;
    InitWindow(screen_width, screen_height, "OS Final Project");
    SetTargetFPS(60);

    const Vector2 center = { (float)screen_width / 2, (float)screen_height / 2 };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_graph_circle(&graph, center, 250.f);

        EndDrawing();
    }
    CloseWindow();

    //dijkstra(&graph);
    free_graph(&graph);
    return 0;
}

void dijkstra(const struct graph_t *graph) {
    // Edge Case: Source is identical to Destination
    if (graph->dijkstra_src == graph->dijkstra_dest) {
        printf("%d\n0\n", graph->dijkstra_src);
        return;
    }

    // --- Dijkstra's Algorithm ---
    int *dist = malloc(graph->vertex_count * sizeof(int));
    int *visited = malloc(graph->vertex_count * sizeof(int));
    int *prev = malloc(graph->vertex_count * sizeof(int));

    // Initialize algorithm arrays
    for (int i = 0; i < graph->vertex_count; i++) {
        dist[i] = INF;
        visited[i] = 0;
        prev[i] = -1;
    }

    dist[graph->dijkstra_src] = 0;

    for (int i = 0; i < graph->vertex_count; i++) {
        int min_dist = INF;
        int u = -1;

        // Find the unvisited node with the smallest known distance
        for (int j = 0; j < graph->vertex_count; j++) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        // Break if we've reached the target or remaining nodes are unreachable
        if (u == -1 || u == graph->dijkstra_dest) break;

        visited[u] = 1;

        // Update distances for adjacent nodes
        for (int v = 0; v < graph->vertex_count; v++) {
            if (!visited[v] && graph->graph[u][v] != -1 && dist[u] != INF) {
                const int alt = dist[u] + graph->graph[u][v];
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
    }

    // --- Print Outputs ---
    // Edge Case: Graph is disconnected (No path)
    if (dist[graph->dijkstra_dest] == INF) {
        printf("No path found\n");
    } else {
        // Reconstruct the path backwards
        int *path = malloc(graph->vertex_count * sizeof(int));
        int curr = graph->dijkstra_dest;
        int path_len = 0;

        while (curr != -1) {
            path[path_len++] = curr;
            curr = prev[curr];
        }

        // Print path forwards
        for (int i = path_len - 1; i >= 0; i--) {
            printf("%d", path[i]);
            if (i > 0) printf("->");
        }
        // Print total weight
        printf("\n%d\n", dist[graph->dijkstra_dest]);

        free(path);
    }

    // Cleanup memory
    free(dist);
    free(visited);
    free(prev);
}
