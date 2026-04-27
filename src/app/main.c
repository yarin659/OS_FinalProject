#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "core/common.h"

// Helper function to free the dynamically allocated 2D graph array
void free_graph(int **graph, int n) {
    for (int i = 0; i < n; i++) {
        free(graph[i]);
    }
    free(graph);
}

int main(int argc, char *argv[]) {
    // Boilerplate parameter handling
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    FILE *file = fopen("data/sample_graph.dat", "r");
    if (!file) {
        printf("Error: Could not data file (data/sample_graph.dat); invalid CMake config?\n");
        return 1;
    }

    int N, M;
    if (fscanf(file, "%d %d", &N, &M) != 2) {
        fclose(file);
        return 1;
    }

    // Allocate Adjacency Matrix dynamically to prevent memory leaks
    int **graph = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        graph[i] = (int *)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            graph[i][j] = -1; // -1 represents no edge between nodes
        }
    }

    // Parse the edges
    for (int i = 0; i < M; i++) {
        int src, dst, weight;
        if (fscanf(file, "%d %d %d", &src, &dst, &weight) != 3) {
            free_graph(graph, N);
            fclose(file);
            return 1;
        }

        // Negative numbers are invalid input; print appropriate error
        if (weight < 0) {
            printf("Error: Negative edge weight detected.\n");
            free_graph(graph, N);
            fclose(file);
            return 1;
        }
        graph[src][dst] = weight;
    }

    // Parse the query (source and destination nodes)
    int start_node, end_node;
    if (fscanf(file, "%d %d", &start_node, &end_node) != 2) {
        free_graph(graph, N);
        fclose(file);
        return 1;
    }
    fclose(file);

    // Edge Case: Source is identical to Destination
    if (start_node == end_node) {
        printf("%d\n0\n", start_node);
        free_graph(graph, N);
        return 0;
    }

    // --- Dijkstra's Algorithm ---
    int *dist = (int *)malloc(N * sizeof(int));
    int *visited = (int *)malloc(N * sizeof(int));
    int *prev = (int *)malloc(N * sizeof(int));

    // Initialize algorithm arrays
    for (int i = 0; i < N; i++) {
        dist[i] = INF;
        visited[i] = 0;
        prev[i] = -1;
    }

    dist[start_node] = 0;

    for (int i = 0; i < N; i++) {
        int min_dist = INF;
        int u = -1;

        // Find the unvisited node with the smallest known distance
        for (int j = 0; j < N; j++) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        // Break if we've reached the target or remaining nodes are unreachable
        if (u == -1 || u == end_node) break;

        visited[u] = 1;

        // Update distances for adjacent nodes
        for (int v = 0; v < N; v++) {
            if (!visited[v] && graph[u][v] != -1 && dist[u] != INF) {
                int alt = dist[u] + graph[u][v];
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
    }

    // --- Print Outputs ---
    // Edge Case: Graph is disconnected (No path)
    if (dist[end_node] == INF) {
        printf("No path found\n");
    } else {
        // Reconstruct the path backwards
        int *path = (int *)malloc(N * sizeof(int));
        int curr = end_node;
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
        printf("\n%d\n", dist[end_node]);

        free(path);
    }

    // Cleanup memory
    free(dist);
    free(visited);
    free(prev);
    free_graph(graph, N);

    return 0;
}