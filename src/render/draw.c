#include "draw.h"
#include <unistd.h>
#include "raymath.h"
#include "core/config.h"

static void internal_draw_edge_arrow(const Vector2 start_pos, const Vector2 end_pos, const Vector2 dir,
                                     const Vector2 normal, const float arrow_size, const Color color) {
    DrawLineEx(start_pos, end_pos, 2.f, color);

    const Vector2 scaled_dir = Vector2Scale(dir, arrow_size);
    const Vector2 scaled_normal = Vector2Scale(normal, arrow_size * 0.6f);

    const Vector2 arrow_p1 = end_pos;
    const Vector2 arrow_p2 = Vector2Subtract(end_pos, Vector2Add(scaled_dir, scaled_normal));
    const Vector2 arrow_p3 = Vector2Subtract(end_pos, Vector2Subtract(scaled_dir, scaled_normal));
    DrawTriangle(arrow_p1, arrow_p2, arrow_p3, color);
}

static void internal_draw_edge_text(const Vector2 start_pos, const Vector2 end_pos, const Vector2 normal,
                                    const int weight, const Color color) {
    const Vector2 midpoint = Vector2Scale(Vector2Add(start_pos, end_pos), 0.5f);
    const Vector2 text_pos = Vector2Add(midpoint, Vector2Scale(normal, 12.0f));
    const char *weight_text = TextFormat("%d", weight);
    const int text_width = MeasureText(weight_text, 20);
    DrawText(weight_text, (int) text_pos.x - text_width / 2, (int) text_pos.y - 10, 20, color);
}

void draw_directed_edge(const Vector2 start_pos, const Vector2 end_pos, const int weight, const float node_radius,
                        const Color color) {
    const Vector2 dir = Vector2Normalize(Vector2Subtract(end_pos, start_pos));
    const Vector2 normal = {-dir.y, dir.x};

    const float line_offset = 6.0f;
    Vector2 adjusted_start = Vector2Add(start_pos, Vector2Scale(dir, node_radius));
    Vector2 adjusted_end = Vector2Subtract(end_pos, Vector2Scale(dir, node_radius));
    adjusted_start = Vector2Add(adjusted_start, Vector2Scale(normal, line_offset));
    adjusted_end = Vector2Add(adjusted_end, Vector2Scale(normal, line_offset));

    internal_draw_edge_arrow(adjusted_start, adjusted_end, dir, normal, 10.0f, color);
    internal_draw_edge_text(adjusted_start, adjusted_end, normal, weight, color);
}

static void internal_draw_graph_edges(const struct graph_t *graph, Vector2 positions[], const float node_radius) {
    for (int i = 0; i < graph->vertex_count; i++) {
        for (int j = 0; j < graph->vertex_count; j++) {
            if (graph->graph[i][j] != -1) {
                draw_directed_edge(positions[i], positions[j], graph->graph[i][j], node_radius, GRAY);
            }
        }
    }
}

static void internal_draw_graph_vertices(const struct graph_t *graph, Vector2 positions[], const float node_radius) {
    for (int i = 0; i < graph->vertex_count; i++) {
        DrawCircleV(positions[i], node_radius, SKYBLUE);
        DrawCircleLines((int) positions[i].x, (int) positions[i].y, node_radius, DARKBLUE);

        const char *id_text = TextFormat("%d", i);
        const int text_width = MeasureText(id_text, 20);
        DrawText(id_text, (int) positions[i].x - text_width / 2, (int) positions[i].y - 10, 20, RAYWHITE);
    }
}

void draw_graph_circle(const struct graph_t *graph, const Vector2 center, const float radius) {
    Vector2 positions[MAX_VERTICES];
    compute_graph_positions(graph, center, radius, positions);

    internal_draw_graph_edges(graph, positions, NODE_DRAW_RADIUS);
    internal_draw_graph_vertices(graph, positions, NODE_DRAW_RADIUS);
}

void draw_path_edges(const struct graph_t *graph, const int *path, const int path_len,
                     Vector2 positions[MAX_VERTICES], const float node_radius) {
    const Color PATH_COLOR = {255, 140, 0, 255};

    for (int i = 0; i < path_len; i++) {
        const int v = path[i];
        DrawCircleV(positions[v], node_radius, (Color){255, 200, 60, 255});
        DrawCircleLines((int) positions[v].x, (int) positions[v].y, node_radius, PATH_COLOR);

        const char *id_text = TextFormat("%d", v);
        const int text_width = MeasureText(id_text, 20);
        DrawText(id_text, (int) positions[v].x - text_width / 2, (int) positions[v].y - 10, 20, DARKBROWN);
    }

    for (int i = 0; i < path_len - 1; i++) {
        const int u = path[i];
        const int v = path[i + 1];
        draw_directed_edge(positions[u], positions[v], graph->graph[u][v], node_radius, PATH_COLOR);
    }
}

void draw_entity(const Vector2 pos, const int index) {
    static const struct { Color halo; Color main; Color shine; } PALETTE[] = {
        { {255, 80,  80,  80}, {220, 50,  50,  255}, {255, 200, 200, 255} }, // red
        { {80,  80,  255, 80}, {50,  50,  220, 255}, {200, 200, 255, 255} }, // blue
        { {80,  200, 80,  80}, {40,  170, 40,  255}, {180, 255, 180, 255} }, // green
        { {220, 140, 40,  80}, {190, 110, 20,  255}, {255, 220, 160, 255} }, // orange
        { {180, 60,  220, 80}, {140, 30,  190, 255}, {230, 180, 255, 255} }, // purple
        { {40,  200, 210, 80}, {20,  170, 180, 255}, {170, 240, 245, 255} }, // cyan
        { {210, 190, 30,  80}, {180, 160, 10,  255}, {255, 245, 160, 255} }, // yellow
        { {220, 80,  160, 80}, {190, 50,  130, 255}, {255, 190, 220, 255} }, // pink
    };
    static const int PALETTE_SIZE = (int)(sizeof(PALETTE) / sizeof(PALETTE[0]));

    const int i = ((index % PALETTE_SIZE) + PALETTE_SIZE) % PALETTE_SIZE;
    DrawCircleV(pos, 16.f, PALETTE[i].halo);
    DrawCircleV(pos, 11.f, PALETTE[i].main);
    DrawCircleV(pos, 5.f,  PALETTE[i].shine);
}

void draw_text_background(const char *text, const int x, const int y, const int font_size, const Color color,
                          const Color background_color) {
    const int text_width = MeasureText(text, font_size);
    DrawRectangle(x - 10, y - 6, text_width + 20, font_size + 12, background_color);
    DrawText(text, x, y, font_size, color);
}

static int find_bounded_font_size(const char *text, const Rectangle bounds, const int padding, const int min_size) {
    int font_size = (int) bounds.height - padding;
    if (font_size < min_size) font_size = min_size;

    int text_width = MeasureText(text, font_size);
    while (text_width > ((int) bounds.width - padding) && font_size > min_size) {
        font_size--;
        text_width = MeasureText(text, font_size);
    }
    return font_size;
}

enum button_state { BUTTON_NORMAL, BUTTON_HOVER, BUTTON_ACTIVE };

BOOL draw_button(const Vector2 start_pos, const Vector2 end_pos, const char *text, const Color color,
                 const Color hover_color, const Color active_color, const Color text_color) {
    const Rectangle button_bounds = {
        start_pos.x,
        start_pos.y,
        end_pos.x - start_pos.x,
        end_pos.y - start_pos.y
    };
    enum button_state state = BUTTON_NORMAL;
    BOOL button_pressed = FALSE;

    if (CheckCollisionPointRec(GetMousePosition(), button_bounds)) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            state = BUTTON_ACTIVE;
        } else {
            state = BUTTON_HOVER;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            button_pressed = TRUE;
        }
    }

    Color draw_color = color;
    switch (state) {
        case BUTTON_ACTIVE:
            draw_color = active_color;
            break;

        case BUTTON_HOVER:
            draw_color = hover_color;
            break;

        default: break;
    }

    DrawRectangleRec(button_bounds, draw_color);

    const int font_size = find_bounded_font_size(text, button_bounds, 10, 12);
    const int text_width = MeasureText(text, font_size);
    const int text_x = (int) (button_bounds.x + (button_bounds.width - (float) text_width) / 2.f);
    const int text_y = (int) (button_bounds.y + (button_bounds.height - (float) font_size) / 2.f);
    DrawText(text, text_x, text_y, font_size, text_color);

    return button_pressed;
}
