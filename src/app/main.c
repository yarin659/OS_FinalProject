#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "anim.h"
#include "raylib.h"
#include "core/common.h"
#include "core/graph.h"
#include "core/vertex_lock.h"
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
        printf("Error: Could not load graph from file; invalid CMake config?\n");
        return 1;
    }

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    int flags = fcntl(pipe_fds[0], F_GETFL, 0);
    fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);

    const int screen_width  = 900;
    const int screen_height = 550;
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screen_width, screen_height, "OS Final Project - Milestone 6");
    SetTargetFPS(60);

    const Vector2 center = { (float)screen_width / 2, (float)screen_height / 2 };
    const float graph_radius = 220.f;

    Vector2 positions[MAX_VERTICES];
    compute_graph_positions(&graph, center, graph_radius, positions);

    for (int i = 0; i < graph.traveler_count; ++i) {
        anim_start(&graph.travelers[i], positions);
    }

    struct vertex_locks_t *vertex_locks = NULL;

    BOOL is_animation_playing = FALSE;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();

        struct ipc_msg_t msg;
        while (read(pipe_fds[0], &msg, sizeof(struct ipc_msg_t)) > 0) {
            const struct traveler_t *traveler = &graph.travelers[msg.traveler_index];
            struct anim_state *anim = traveler->anim;

            switch (msg.type) {
                case MSG_WAITING:
                    printf("[%d] WAITING outside node %d\n", msg.pid, msg.current_node);
                    fflush(stdout);
                    if (anim) {
                        anim->phase = ANIM_WAITING_OUTSIDE;
                        anim->waiting_node = msg.current_node;
                        const float angle = (float)msg.traveler_index * (2.f * 3.14159265f / 8.f);
                        const float orbit = NODE_DRAW_RADIUS + 20.f;
                        anim->entity_pos.x = positions[msg.current_node].x + cosf(angle) * orbit;
                        anim->entity_pos.y = positions[msg.current_node].y + sinf(angle) * orbit;
                    }
                    break;

                case MSG_ARRIVED:
                    printf("[%d] ENTERED node %d | next: %d\n", msg.pid, msg.current_node, msg.next_node);
                    fflush(stdout);
                    if (anim) {
                        anim->phase = ANIM_AT_VERTEX;
                        anim->entity_pos = positions[msg.current_node];
                    }
                    break;

                case MSG_LEAVING:
                    printf("[%d] LEAVING node %d -> node %d\n", msg.pid, msg.current_node, msg.next_node);
                    fflush(stdout);
                    if (anim) {
                        anim->phase = ANIM_ON_EDGE;
                        anim->current_u = msg.current_node;
                        anim->current_v = msg.next_node;
                        anim->phase_timer = 0.f;
                        const float weight = (float)graph.graph[msg.current_node][msg.next_node];
                        anim->total_edge_time = (weight * JUMP_DURATION_MS) / 1000.f;
                    }
                    break;

                case MSG_FINISHED:
                    printf("[%d] FINISHED\n", msg.pid);
                    fflush(stdout);
                    if (anim) {
                        anim->phase = ANIM_DONE;
                    }
                    break;

                default:
                    break;
            }
        }

        if (is_animation_playing) {
            for (int i = 0; i < graph.traveler_count; ++i) {
                anim_update(&graph.travelers[i], positions, dt);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        draw_graph_circle(&graph, center, graph_radius);

        for (int i = 0; i < graph.traveler_count; ++i) {
            const struct traveler_t *traveler = &graph.travelers[i];
            if (!traveler->anim) continue;

            if (traveler->anim->phase == ANIM_WAITING_OUTSIDE) {
                draw_entity_waiting(traveler->anim->entity_pos, i);
            } else {
                draw_entity(traveler->anim->entity_pos, i);
            }
        }

        BOOL all_travelers_done = TRUE;
        for (int i = 0; i < graph.traveler_count; ++i) {
            if (!graph.travelers[i].anim || graph.travelers[i].anim->phase != ANIM_DONE) {
                all_travelers_done = FALSE;
                break;
            }
        }
        if (all_travelers_done && is_animation_playing) {
            display_diagnostic("Arrived at destination", screen_width, screen_height);
        }

        const Vector2 button_start = { center.x - 50, 5 };
        const Vector2 button_end = { center.x + 50, 35 };
        const char *button_text = is_animation_playing ? "Stop" : "Start";
        const Color btn_col = is_animation_playing ? RED : DARKGREEN;
        const Color btn_hov = is_animation_playing ? VIOLET : GREEN;
        const Color btn_act = is_animation_playing ? PINK : LIME;
        const Color btn_txt = is_animation_playing ? WHITE : BLACK;

        if (draw_button(button_start, button_end, button_text, btn_col, btn_hov, btn_act, btn_txt)) {
            is_animation_playing = !is_animation_playing;

            if (is_animation_playing) {
                vertex_locks = vertex_locks_create(graph.vertex_count);

                for (int i = 0; i < graph.traveler_count; ++i) {
                    anim_start(&graph.travelers[i], positions);
                    graph.travelers[i].vertex_locks = vertex_locks;
                    traveler_create_subprocess(&graph.travelers[i], i, pipe_fds[1]);
                }
            } else {
                // kill subprocesses and destroy our semaphores
                for (int i = 0; i < graph.traveler_count; ++i) {
                    traveler_destroy_subprocess(&graph.travelers[i]);
                    anim_stop(&graph.travelers[i]);
                }
                vertex_locks_destroy(vertex_locks);
                vertex_locks = NULL;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    close(pipe_fds[0]);
    close(pipe_fds[1]);

    if (vertex_locks) {
        vertex_locks_destroy(vertex_locks);
    }

    free_graph(&graph);
    while (wait(NULL) > 0) {}

    return 0;
}
