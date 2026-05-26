#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

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

    struct traveler_t {
        int id;
        int src;
        int dest;
        struct graph_t traveler_graph;
        struct anim_state anim;
        pid_t child_pid;
        Color color;
        BOOL is_active;
    };

    char line[256];
    int num_travelers = 0;


    const int screen_width  = 900;
    const int screen_height = 550;
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screen_width, screen_height, "OS Final Project");
    SetTargetFPS(60);

    const Vector2 center = { (float)screen_width / 2, (float)screen_height / 2 };
    const float graph_radius = 220.f;

    Vector2 positions[MAX_VERTICES];
    compute_graph_positions(&graph, center, graph_radius, positions);

    FILE *file = fopen(graph_file_path, "r");
    if (!file) {
        printf("Error: Could not open file for reading travelers\n");
        return 1;
    }
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' && strstr(line, "travelers")) {
            if (fscanf(file, "%d", &num_travelers) == 1) {
                break;
            }
        }
    }
    struct traveler_t *travelers = malloc(num_travelers * sizeof(struct traveler_t));
    Color colors[] = { RED, BLUE, ORANGE, PURPLE, LIME, GOLD, MAROON };

    for (int i = 0; i < num_travelers; i++) {
        travelers[i].id = i;
        fscanf(file, "%d %d", &travelers[i].src, &travelers[i].dest);
        travelers[i].color = colors[i % 7];
        travelers[i].is_active = TRUE;

        memset(&travelers[i].anim, 0, sizeof(struct anim_state));
        travelers[i].anim.phase = ANIM_IDLE;

        travelers[i].traveler_graph = graph;
        travelers[i].traveler_graph.dijkstra_src = travelers[i].src;
        travelers[i].traveler_graph.dijkstra_dest = travelers[i].dest;

        dijkstra_compute(&travelers[i].traveler_graph, &travelers[i].anim.result);

    }
    fclose(file);

    for (int i = 0; i < num_travelers; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            free(travelers);
            return 1;
        }
        else if (pid == 0) {
            // sun process
            printf("[%d] started\n", getpid());
            fflush(stdout);

            while (1) { //endless loop so the sun will sleep until the dad kill it
                sleep(1);
            }
            exit(0);
        }
        else {
            // parent process
            travelers[i].child_pid = pid;
        }
    }

    BOOL is_animation_playing = FALSE;

    // -------------------------------------------------------------
    // main GUI
    // -------------------------------------------------------------
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();


        if (is_animation_playing) {
            for (int i = 0; i < num_travelers; i++) {
                if (travelers[i].is_active && travelers[i].anim.phase != ANIM_DONE) {


                    graph.dijkstra_src = travelers[i].src;
                    graph.dijkstra_dest = travelers[i].dest;

                    anim_update(&travelers[i].anim, &travelers[i].traveler_graph, positions, dt);

                    if (travelers[i].anim.phase == ANIM_DONE) {
                        travelers[i].is_active = FALSE;
                        printf("Traveler %d finished. Killing child [PID: %d]\n", travelers[i].id, travelers[i].child_pid);
                        fflush(stdout);
                        kill(travelers[i].child_pid, SIGKILL); // שליחת סיגנל סיום לבן
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);


        draw_graph_circle(&graph, center, graph_radius);


        for (int i = 0; i < num_travelers; i++) {
            if (is_animation_playing && travelers[i].is_active) {


                if (travelers[i].anim.result.path_len > 0) {
                    draw_path_edges(&graph, travelers[i].anim.result.path, travelers[i].anim.result.path_len, positions, NODE_DRAW_RADIUS);
                }


                if (travelers[i].anim.phase != ANIM_IDLE) {
                    DrawCircleV(travelers[i].anim.entity_pos, 12.f, travelers[i].color);


                    Vector2 src_pos = positions[travelers[i].src];
                    DrawCircleLines((int) src_pos.x, (int) src_pos.y, NODE_DRAW_RADIUS + 5.f, travelers[i].color);
                }
            }
        }


        BOOL all_done = TRUE;
        for (int i = 0; i < num_travelers; i++) {
            if (travelers[i].is_active) {
                all_done = FALSE;
                break;
            }
        }
        if (is_animation_playing && all_done) {
            display_diagnostic("All travelers arrived at destinations", screen_width, screen_height);
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

                for (int i = 0; i < num_travelers; i++) {
                    anim_start(&travelers[i].anim, &travelers[i].traveler_graph, positions);
                }
            } else {

                for (int i = 0; i < num_travelers; i++) {
                    anim_stop(&travelers[i].anim);
                }
            }
        }

        EndDrawing();
    }

    // -------------------------------------------------------------
    // cleaning - avoid zombies
    // -------------------------------------------------------------
    CloseWindow();

    for (int i = 0; i < num_travelers; i++) {
        kill(travelers[i].child_pid, SIGKILL);
        waitpid(travelers[i].child_pid, NULL, 0);

        free_dijkstra_result(&travelers[i].anim.result);
    }

    free(travelers);

    free_graph(&graph);
    return 0;
}
