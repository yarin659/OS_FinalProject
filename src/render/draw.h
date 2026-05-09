#ifndef OSFINALPROJECT_DRAW_H
#define OSFINALPROJECT_DRAW_H
#include "raylib.h"
#include "core/graph.h"

void draw_directed_edge(Vector2 start_pos, Vector2 end_pos, int weight, float node_radius, Color color);
void draw_graph_circle(const struct graph_t* graph, Vector2 center, float radius);
BOOL draw_button(Vector2 start_pos, Vector2 end_pos, const char* text, Color color, Color hover_color,
    Color active_color, Color text_color);

#endif //OSFINALPROJECT_DRAW_H
