#ifndef OSFINALPROJECT_DRAW_H
#define OSFINALPROJECT_DRAW_H
#include "raylib.h"
#include "core/config.h"
#include "core/graph.h"

void draw_directed_edge(Vector2 start_pos, Vector2 end_pos, int weight, float node_radius, Color color);

void draw_graph_edges(const struct graph_t *graph, Vector2 positions[], float node_radius);
void draw_graph_vertices(const struct graph_t *graph, Vector2 positions[], float node_radius);

void draw_path_edges(const struct graph_t *graph, const int *path, int path_len, Vector2 positions[MAX_VERTICES],float node_radius);


void draw_graph_circle(const struct graph_t* graph, Vector2 center, float radius);
void draw_entity(Vector2 pos, int index);

void draw_text_background(const char* text, int x, int y, int font_size, Color color, Color background_color);
BOOL draw_button(Vector2 start_pos, Vector2 end_pos, const char* text, Color color, Color hover_color,
    Color active_color, Color text_color);

#endif //OSFINALPROJECT_DRAW_H