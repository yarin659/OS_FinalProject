CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET_M1 = dijkstra
TARGET_SIM = sim

CORE_SRCS = src/core/graph.c src/core/dijkstra.c src/core/traveler.c
RENDER_SRCS = src/render/draw.c
ANIM_SRCS = src/app/anim.c
SOURCES = src/app/main.c $(CORE_SRCS) $(RENDER_SRCS) $(ANIM_SRCS)

milestone1:
	$(CC) $(CFLAGS) -DCLI_ONLY $(SOURCES) -o $(TARGET_M1) $(LDFLAGS)

milestone2:
	$(CC) $(CFLAGS) -DSTATIC_GUI $(SOURCES) -o $(TARGET_SIM) $(LDFLAGS)

milestone3:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET_SIM) $(LDFLAGS)

milestone4:
	$(CC) $(CFLAGS) -O3 $(SOURCES) -o $(TARGET_SIM) $(LDFLAGS)

clean:
	rm -f $(TARGET_M1) $(TARGET_SIM) *.o

.PHONY: milestone1 milestone2 milestone3 milestone4 clean