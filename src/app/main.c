#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "anim.h"
#include "raylib.h"
#include "core/common.h"
#include "core/dijkstra.h"
#include "core/graph.h"
#include "render/draw.h"
#include "core/traveler.h"

static void display_diagnostic(const char* msg, const int screen_width, const int screen_height) {
    const int msg_font_size = 22;
    const int msg_w = MeasureText(msg, msg_font_size);
    const int msg_x = screen_width / 2 - msg_w / 2;
    const int msg_y = screen_height - 40;
    draw_text_background(msg, msg_x, msg_y, msg_font_size, (Color){255, 220, 80, 255},
                         (Color){30, 30, 30, 200});
}

int main(const int argc, char *argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    const char* graph_file_path;
    if (argc > 1) {
        graph_file_path = argv[1];
    } else {
        graph_file_path = "data/sample_graph.dat";
    }

    struct graph_t graph = {};
    memset(&graph, 0, sizeof(struct graph_t));

    if (!load_graph_from_file(graph_file_path, &graph)) {
        printf("Error: Could not load graph from file (data/sample_graph.dat); invalid CMake config?\n");
        return 1;
    }

    for (int i = 0; i < graph.traveler_count; ++i) {
        if (!dijkstra_compute(&graph, i)) {
            printf("Failed to compute dijkstra for traveler #%d", i);
        }

        traveler_create_subprocess(&graph.travelers[i]);
    }

    const int screen_width  = 900;
    const int screen_height = 550;
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screen_width, screen_height, "OS Final Project");
    SetTargetFPS(60);

    const Vector2 center = { (float)screen_width / 2, (float)screen_height / 2 };
    const float graph_radius = 220.f;

    Vector2 positions[MAX_VERTICES];
    compute_graph_positions(&graph, center, graph_radius, positions);

    struct anim_state anim = {0};
    anim.phase = ANIM_IDLE;

    BOOL is_animation_playing = FALSE;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();

        if (is_animation_playing) {
            for (int i = 0; i < graph.traveler_count; ++i) {
                anim_update(&graph.travelers[i], positions, dt);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_graph_circle(&graph, center, graph_radius);

        if (is_animation_playing) {
            // לולאה ראשונה: ציור של כל המסלולים בלבד (השכבה התחתונה)
            for (int i = 0; i < graph.traveler_count; ++i) {
                struct traveler_t* traveler = &graph.travelers[i];
                if (traveler->anim == NULL) {
                    continue;
                }

                if (traveler->anim->phase != ANIM_IDLE && traveler->dijkstra_result->path_len == 0) {
                    display_diagnostic("No path for some travelers", screen_width, screen_height);
                }

                if (traveler->dijkstra_result->path_len > 0) {
                    draw_path_edges(&graph, traveler->dijkstra_result->path, traveler->dijkstra_result->path_len,
                                    positions, NODE_DRAW_RADIUS);
                }
            }

            // second loop - drawing all entities at the top layer so there's no z-fighting
            for (int i = 0; i < graph.traveler_count; ++i) {
                struct traveler_t* traveler = &graph.travelers[i];
                if (traveler->anim == NULL) {
                    continue;
                }

                if (traveler->anim->phase != ANIM_IDLE) {
                    // decide who is the parent and who is a child, assume index 0 is the parent
                    // and the rest are children
                    BOOL is_child = (i != 0);
                    draw_entity(traveler->anim->entity_pos, is_child);
                }
            }
        }

        BOOL all_travelers_done = TRUE;
        for (int i = 0; i < graph.traveler_count; ++i) {
            if (graph.travelers[i].anim == NULL || graph.travelers[i].anim->phase != ANIM_DONE) {
                all_travelers_done = FALSE;
                break;
            }
        }
        if (all_travelers_done) {
            display_diagnostic("Arrived at destination", screen_width, screen_height);
        }

        const Vector2 button_start = { center.x - 50, 5 };
        const Vector2 button_end = { center.x + 50, 35 };
        const char* button_text = is_animation_playing ? "Stop" : "Start";
        const Color btn_col = is_animation_playing ? RED : DARKGREEN;
        const Color btn_hov = is_animation_playing ? VIOLET : GREEN;
        const Color btn_act = is_animation_playing ? PINK : LIME;
        const Color btn_txt = is_animation_playing ? WHITE : BLACK;
        if (draw_button(button_start, button_end, button_text, btn_col, btn_hov, btn_act, btn_txt)) {
            is_animation_playing = !is_animation_playing;

            if (is_animation_playing) {
                for (int i = 0; i < graph.traveler_count; ++i) {
                    anim_start(&graph.travelers[i], positions);
                }
            } else {
                for (int i = 0; i < graph.traveler_count; ++i) {
                    anim_stop(&graph.travelers[i]);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    free_graph(&graph);
    int status;
    waitpid(-1, &status, 0);
    return 0;
}
