#include "draw.h"

#include "raymath.h"

static void internal_draw_edge_arrow(const Vector2 start_pos, const Vector2 end_pos, const Vector2 dir,
                                         const Vector2 normal, const float arrow_size, const Color color) {
    // this is just the actual line connecting the two. we just draw it straight to the end pos and have the
    // arrowhead drawn over it.
    DrawLineEx(start_pos, end_pos, 2.f, color);

    // the rest here is the (directional) arrowhead itself
    const Vector2 scaled_dir = Vector2Scale(dir, arrow_size);
    const Vector2 scaled_normal = Vector2Scale(normal, arrow_size * 0.6f);

    const Vector2 arrow_p1 = end_pos;
    const Vector2 arrow_p2 = Vector2Subtract(end_pos, Vector2Add(scaled_dir, scaled_normal));
    const Vector2 arrow_p3 = Vector2Subtract(end_pos, Vector2Subtract(scaled_dir, scaled_normal));
    DrawTriangle(arrow_p1, arrow_p2, arrow_p3, color);
}

static void internal_draw_edge_text(const Vector2 start_pos, const Vector2 end_pos, const Vector2 normal,
                                    const int weight, const Color color) {
    // TODO: this is sort of drawn in the midpoint but because we're drawing it here, other lines draw over this
    // I need to check if there's a raylib this for z-ordering and then we can just give it a higher z-order.
    const Vector2 midpoint = Vector2Scale(Vector2Add(start_pos, end_pos), 0.5f);
    const Vector2 text_pos = Vector2Add(midpoint, Vector2Scale(normal, 12.0f));
    const char* weight_text = TextFormat("%d", weight);
    const int text_width = MeasureText(weight_text, 20);
    DrawText(weight_text, (int)text_pos.x - text_width/2, (int)text_pos.y - 10, 20, color);

}

void draw_directed_edge(const Vector2 start_pos, const Vector2 end_pos, const int weight, const float node_radius,
                        const Color color) {
    const Vector2 dir = Vector2Normalize(Vector2Subtract(end_pos, start_pos));
    const Vector2 normal = { -dir.y, dir.x };

    // we have to offset the start and end positions so that they connect to the edges of the circle
    // and not the center of it. Then we have to push it off to the side since I don't know
    // whether or a pair of vertices can have arrows pointing at each other,
    // but it's easier to see this way in case that they do.
    const float line_offset = 6.0f;
    Vector2 adjusted_start = Vector2Add(start_pos, Vector2Scale(dir, node_radius));
    Vector2 adjusted_end = Vector2Subtract(end_pos, Vector2Scale(dir, node_radius));
    adjusted_start = Vector2Add(adjusted_start, Vector2Scale(normal, line_offset));
    adjusted_end = Vector2Add(adjusted_end, Vector2Scale(normal, line_offset));

    internal_draw_edge_arrow(adjusted_start, adjusted_end, dir, normal, 10.0f, color);
    internal_draw_edge_text(adjusted_start, adjusted_end, normal, weight, color);
}

static void internal_draw_graph_edges(const struct graph_t* graph, Vector2 positions[], const float node_radius) {
    for (int i = 0; i < graph->vertex_count; i++) {
        for (int j = 0; j < graph->vertex_count; j++) {
            // because -1 is a disconnect we can just skip drawing it.
            // TODO: have to actually reference moodle to see if there are negative weights :|
            if (graph->graph[i][j] != -1) {
                draw_directed_edge(positions[i], positions[j], graph->graph[i][j], node_radius, GRAY);
            }
        }
    }
}

static void internal_draw_graph_vertices(const struct graph_t* graph, Vector2 positions[], const float node_radius) {
    for (int i = 0; i < graph->vertex_count; i++) {
        DrawCircleV(positions[i], node_radius, SKYBLUE);
        DrawCircleLines((int) positions[i].x, (int) positions[i].y, node_radius, DARKBLUE);

        const char* id_text = TextFormat("%d", i);
        const int text_width = MeasureText(id_text, 20);
        DrawText(id_text, (int) positions[i].x - text_width / 2, (int) positions[i].y - 10, 20, RAYWHITE);
    }
}

void draw_graph_circle(const struct graph_t* graph, const Vector2 center, const float radius) {
    // TODO: This is technically being calculated every frame. maybe we want to precalculate these.
    Vector2 positions[15];
    for (int i = 0; i < graph->vertex_count; i++) {
        const float angle = (2.0f * PI / (float)graph->vertex_count) * (float)i;
        positions[i].x = center.x + cosf(angle) * radius;
        positions[i].y = center.y + sinf(angle) * radius;
    }

    const float node_radius = 25.0f;
    internal_draw_graph_edges(graph, positions, node_radius);
    internal_draw_graph_vertices(graph, positions, node_radius);
}

static int find_bounded_font_size(const char* text, const Rectangle bounds, int padding, int min_size) {
    int font_size = (int)bounds.height - padding;
    if (font_size < min_size) font_size = min_size;

    int text_width = MeasureText(text, font_size);
    while (text_width > ((int)bounds.width - padding) && font_size > min_size) {
        font_size--;
        text_width = MeasureText(text, font_size);
    }

    return font_size;
}

enum button_state { BUTTON_NORMAL, BUTTON_HOVER, BUTTON_ACTIVE };
BOOL draw_button(const Vector2 start_pos, const Vector2 end_pos, const char* text, const Color color,
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
        default:
            break;
    }

    DrawRectangleRec(button_bounds, draw_color);

    const int font_size = find_bounded_font_size(text, button_bounds, 10, 12);
    const int text_width = MeasureText(text, font_size);
    const int text_x = (int)(button_bounds.x + (button_bounds.width - (float)text_width) / 2.f);
    const int text_y = (int)(button_bounds.y + (button_bounds.height - (float)font_size) / 2.f);
    DrawText(text, text_x, text_y, font_size, text_color);

    return button_pressed;
}
