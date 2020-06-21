CC = gcc -std=c89 -pedantic -w
INCLUDES = src/*.h
COMMON_DEPS = $(INCLUDES) makefile

TEST = build/test.o \
	build/random.o \
	build/msg.o \
	build/sem.o \
	build/shm.o \
	build/sig.o \
	build/timer.o \
	build/game.o \
	build/debug.o

MASTER = build/master.o \
	build/random.o \
	build/msg.o \
	build/sem.o \
	build/shm.o \
	build/sig.o \
	build/timer.o \
	build/game.o \
	build/debug.o

PLAYER = build/player.o \
	build/random.o \
	build/msg.o \
	build/sem.o \
	build/shm.o \
	build/sig.o \
	build/game.o \
	build/debug.o

PAWN = build/pawn.o \
	build/random.o \
	build/msg.o \
	build/sem.o \
	build/shm.o \
	build/sig.o \
	build/game.o \
	build/debug.o

all: dir test master player pawn

master: $(MASTER)
	$(CC) -o bin/$@ $^

player: $(PLAYER)
	$(CC) -o bin/$@ $^

pawn: $(PAWN)
	$(CC) -o bin/$@ $^

test: $(TEST)
	$(CC) -o bin/$@ $^

build/%.o:	src/%.c $(COMMON_DEPS)
	$(CC) -c $< -o $@

dir:
	mkdir -p bin build

clean:
	rm -rf bin build
