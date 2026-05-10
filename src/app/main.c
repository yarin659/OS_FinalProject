#include <stdio.h>
#include <memory.h>

#include "anim.h"
#include "raylib.h"
#include "core/common.h"
#include "core/dijkstra.h"
#include "core/graph.h"
#include "render/draw.h"

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

    struct graph_t graph = {};
    memset(&graph, 0, sizeof(struct graph_t));

    if (!load_graph_from_file("data/sample_graph.dat", &graph)) {
        printf("Error: Could not load graph from file (data/sample_graph.dat); invalid CMake config?\n");
        return 1;
    }

#ifdef CLI_ONLY
    if (graph.dijkstra_src == graph.dijkstra_dest) {
        printf("0\n0\n");
        return 0;
    }

    struct dijkstra_result_t result = {0};
    dijkstra_compute(&graph, &result);

    if (result.path_len == 0) {
        printf("No path found\n");
        return 0;
    }

    printf("%d", graph.dijkstra_src);
    for (int i = 1; i < result.path_len; i++) {
        printf(" -> %d", result.path[i]);
    }
    printf("\n%d\n", result.total_dist);

    free_dijkstra_result(&result);
#else
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

        if (is_animation_playing && anim.phase != ANIM_DONE) {
            anim_update(&anim, &graph, positions, dt);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_graph_circle(&graph, center, graph_radius);

        if (is_animation_playing && anim.result.path_len > 0) {
            draw_path_edges(&graph, anim.result.path, anim.result.path_len, positions, NODE_DRAW_RADIUS);
        }

        if (is_animation_playing && anim.phase != ANIM_IDLE) {
            draw_entity(anim.entity_pos);

            // source vertex marker
            const Vector2 src_pos = positions[graph.dijkstra_src];
            DrawCircleLines((int) src_pos.x, (int) src_pos.y, NODE_DRAW_RADIUS + 5.f, (Color){100, 200, 100, 200});
        }

        if (anim.phase == ANIM_DONE) {
            display_diagnostic("Arrived at destination", screen_width, screen_height);
        }

        if (is_animation_playing && anim.phase != ANIM_IDLE && anim.result.path_len == 0) {
            display_diagnostic("No path", screen_width, screen_height);
        }

#ifndef STATIC_GUI
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
                anim_start(&anim, &graph, positions);
            } else {
                anim_stop(&anim);
            }
        }
#endif // STATIC_GUI

        EndDrawing();
    }

    CloseWindow();
    free_dijkstra_result(&anim.result);
#endif // CLI_ONLY

    free_graph(&graph);
    return 0;
}
