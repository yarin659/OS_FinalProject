#include <stdio.h>
#include <string.h>

#include "core/common.h"
#include "core/graph.h"

int main(int argc, char *argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    if (argc > 1 && !strcmp(argv[1], "--test")) {
        // TODO run unit tests
        return 0;
    }

    Graph graph = {};
    load_graph_from_file("data/sample_graph.dat", &graph);

    return 0;
}
