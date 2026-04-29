#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <memory.h>
#include <math.h>

#include "raylib.h"
#include "core/common.h"
#include "core/graph.h"
#include "render/draw.h"

// --- הגדרות עבור אבן דרך 3 ---
typedef enum { STATE_IDLE, STATE_WAITING, STATE_MOVING, STATE_ARRIVED } AnimState;

struct {
    AnimState state;
    int path[MAX_VERTICES]; 
    int path_len;
    int current_idx;        // האינדקס של הצומת הנוכחי במערך ה-path
    int edge_step;          // הקפיצה הנוכחית על הקשת (1 עד W) 
    float timer;            // טיימר פנימי לשניות
    bool is_playing;        // שליטה על Play/Stop 
} anim_ctrl = { STATE_IDLE, {0}, 0, 0, 0, 0.0f, false };

// פונקציות עזר
void dijkstra(struct graph_t *graph);
Vector2 get_node_position(int node_idx, int total_nodes, Vector2 center, float radius);

int main(int argc, char *argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    struct graph_t graph = {};
    memset(&graph, 0, sizeof(struct graph_t));

    if (!load_graph_from_file("data/sample_graph.dat", &graph)) {
        printf("Error: Could not load graph from file\n");
        return 1;
    }

    // הפעלת דייקסטרה פעם אחת כדי למצוא את המסלול לפני תחילת האנימציה [cite: 24, 26]
    dijkstra(&graph);

    const int screen_width = 900;
    const int screen_height = 550;
    const float graph_radius = 250.0f;
    InitWindow(screen_width, screen_height, "OS Final Project - Milestone 3");
    SetTargetFPS(60);

    const Vector2 center = { (float)screen_width / 2, (float)screen_height / 2 };

    while (!WindowShouldClose()) {
        
        // --- 1. עדכון לוגיקה (Update) ---
        if (anim_ctrl.is_playing && anim_ctrl.state != STATE_ARRIVED && anim_ctrl.state != STATE_IDLE) {
            anim_ctrl.timer += GetFrameTime();

            // התנהגות בצומת: המתנה של שנייה (לא כולל יעד) 
            if (anim_ctrl.state == STATE_WAITING) {
                if (anim_ctrl.timer >= 1.0f) {
                    anim_ctrl.state = STATE_MOVING;
                    anim_ctrl.timer = 0;
                    anim_ctrl.edge_step = 0;
                }
            } 
            // התנהגות על קשת: קפיצות לפי משקל 
            else if (anim_ctrl.state == STATE_MOVING) {
                if (anim_ctrl.timer >= 0.3f) { // כל קפיצה לוקחת 300 מילישניות 
                    anim_ctrl.edge_step++;
                    anim_ctrl.timer = 0;

                    int u = anim_ctrl.path[anim_ctrl.current_idx];
                    int v = anim_ctrl.path[anim_ctrl.current_idx + 1];
                    int weight = graph.graph[u][v];

                    if (anim_ctrl.edge_step >= weight) {
                        anim_ctrl.current_idx++;
                        // בדיקה אם הגענו לצומת האחרון (היעד) [cite: 27]
                        if (anim_ctrl.current_idx >= anim_ctrl.path_len - 1) {
                            anim_ctrl.state = STATE_ARRIVED;
                        } else {
                            anim_ctrl.state = STATE_WAITING;
                        }
                    }
                }
            }
        }

        // --- 2. ציור (Drawing) ---
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // ציור הגרף הסטטי ברקע [cite: 27]
            draw_graph_circle(&graph, center, graph_radius);

            // כפתור Play/Stop 
            Rectangle btnRec = { 20, 20, 100, 40 };
            DrawRectangleRec(btnRec, anim_ctrl.is_playing ? RED : DARKGREEN);
            DrawText(anim_ctrl.is_playing ? "STOP" : "PLAY", 35, 30, 20, WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btnRec)) {
                anim_ctrl.is_playing = !anim_ctrl.is_playing;
            }

            // ציור הישות הנעה 
            if (anim_ctrl.state != STATE_IDLE) {
                int u_idx = anim_ctrl.path[anim_ctrl.current_idx];
                Vector2 posA = get_node_position(u_idx, graph.vertex_count, center, graph_radius);

                if (anim_ctrl.state == STATE_MOVING) {
                    int v_idx = anim_ctrl.path[anim_ctrl.current_idx + 1];
                    Vector2 posB = get_node_position(v_idx, graph.vertex_count, center, graph_radius);
                    int weight = graph.graph[u_idx][v_idx];
                    
                    float t = (float)anim_ctrl.edge_step / weight;
                    Vector2 entityPos = {
                        posA.x + (posB.x - posA.x) * t,
                        posA.y + (posB.y - posA.y) * t
                    };
                    DrawCircleV(entityPos, 15, BLUE);
                } else {
                    DrawCircleV(posA, 15, BLUE);
                }
            }

            // הצגת הודעה לאחר הגעה ליעד [cite: 27]
            if (anim_ctrl.state == STATE_ARRIVED) {
                DrawRectangle(0, 0, screen_width, screen_height, Fade(RAYWHITE, 0.7f));
                DrawText("DESTINATION REACHED!", screen_width/2 - 170, screen_height/2 - 15, 30, DARKGREEN);
            }

        EndDrawing();
    }

    CloseWindow();
    free_graph(&graph);
    return 0;
}

// פונקציה לחישוב מיקום צומת על המעגל (תואם ל-draw_graph_circle)
Vector2 get_node_position(int node_idx, int total_nodes, Vector2 center, float radius) {
    float angle = (2.0f * PI * node_idx / total_nodes) - (PI / 2.0f);
    return (Vector2){ 
        center.x + radius * cosf(angle), 
        center.y + radius * sinf(angle) 
    };
}

void dijkstra(struct graph_t *graph) {
    if (graph->dijkstra_src == graph->dijkstra_dest) {
        anim_ctrl.state = STATE_ARRIVED;
        return;
    }

    int *dist = malloc(graph->vertex_count * sizeof(int));
    int *visited = malloc(graph->vertex_count * sizeof(int));
    int *prev = malloc(graph->vertex_count * sizeof(int));

    for (int i = 0; i < graph->vertex_count; i++) {
        dist[i] = INF;
        visited[i] = 0;
        prev[i] = -1;
    }

    dist[graph->dijkstra_src] = 0;

    for (int i = 0; i < graph->vertex_count; i++) {
        int min_dist = INF;
        int u = -1;

        for (int j = 0; j < graph->vertex_count; j++) {
            if (!visited[j] && dist[j] < min_dist) {
                min_dist = dist[j];
                u = j;
            }
        }

        if (u == -1 || u == graph->dijkstra_dest) break;
        visited[u] = 1;

        for (int v = 0; v < graph->vertex_count; v++) {
            if (!visited[v] && graph->graph[u][v] != -1 && dist[u] != INF) {
                int alt = dist[u] + graph->graph[u][v];
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
    }

    if (dist[graph->dijkstra_dest] != INF) {
        // שחזור המסלול ושמירה לאנימציה [cite: 24]
        int temp_path[MAX_VERTICES];
        int curr = graph->dijkstra_dest;
        int count = 0;

        while (curr != -1) {
            temp_path[count++] = curr;
            curr = prev[curr];
        }

        anim_ctrl.path_len = count;
        for (int i = 0; i < count; i++) {
            anim_ctrl.path[i] = temp_path[count - 1 - i];
        }

        anim_ctrl.state = STATE_WAITING; 
        anim_ctrl.current_idx = 0;
    }

    free(dist);
    free(visited);
    free(prev);
}
