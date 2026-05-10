# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc/core -Isrc/app -Isrc/render
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Executable names
TARGET_M1 = dijkstra
TARGET_SIM = sim

# Common Core files (Needed for EVERY milestone)
CORE_SRCS = src/core/graph.c src/core/dijkstra.c

# Render files (Needed for M2 and M3)
RENDER_SRCS = src/render/draw.c

# Animation files (Needed only for M3)
ANIM_SRCS = src/app/anim.c

# --- Targets ---

all: milestone1 milestone3

# Milestone 1: Only Logic + Dijkstra (Terminal Output)
# Needs: Core logic + main
milestone1:
	$(CC) $(CFLAGS) src/app/main.c $(CORE_SRCS) -o $(TARGET_M1) $(LDFLAGS)

# Milestone 2: Static GUI
# Needs: Core logic + Rendering + main (Raylib)
milestone2:
	$(CC) $(CFLAGS) -DSTATIC_GUI src/app/main.c $(CORE_SRCS) $(RENDER_SRCS) -o $(TARGET_SIM) $(LDFLAGS)

# Milestone 3: Full Animation
# Needs: Core logic + Rendering + Animation + main
milestone3:
	$(CC) $(CFLAGS) src/app/main.c $(CORE_SRCS) $(RENDER_SRCS) $(ANIM_SRCS) -o $(TARGET_SIM) $(LDFLAGS)

clean:
	rm -f $(TARGET_M1) $(TARGET_SIM) *.o

.PHONY: all milestone1 milestone2 milestone3 clean