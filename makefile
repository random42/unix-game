CC = gcc -std=c89 -pedantic -w -D_POSIX_C_SOURCE=199309L
INCLUDES = src/*.h
COMMON_DEPS = $(INCLUDES) makefile

MASTER = build/master.o \
	build/random.o \
	build/msg.o \
	build/sem.o \
	build/shm.o \
	build/process.o \
	build/timer.o \
	build/game.o \
	build/debug.o

PLAYER = build/player.o \
	build/random.o \
	build/msg.o \
	build/sem.o \
	build/shm.o \
	build/process.o \
	build/game.o \
	build/debug.o

PAWN = build/pawn.o \
	build/random.o \
	build/msg.o \
	build/sem.o \
	build/shm.o \
	build/process.o \
	build/game.o \
	build/debug.o

all: dir master player pawn

master: $(MASTER)
	$(CC) -o bin/$@ $^

player: $(PLAYER)
	$(CC) -o bin/$@ $^

pawn: $(PAWN)
	$(CC) -o bin/$@ $^

build/%.o: src/%.c $(COMMON_DEPS)
	$(CC) -c $< -o $@

dir:
	mkdir -p bin build

clean:
	rm -rf bin build
