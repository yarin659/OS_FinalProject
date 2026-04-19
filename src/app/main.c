#include <stdio.h>
#include <string.h>

#include "core/common.h"

int main(int argc, char *argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    if (argc > 0 && !strcmp(argv[0], "--test")) {
        // TODO run unit tests
        return 0;
    }

    return 0;
}
