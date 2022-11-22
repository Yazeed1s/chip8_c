
OBJS = src/chip8.c src/keyboard.c src/memory.c src/screen.c src/stack.c src/main.c
CMP = chip8.o main.o keyboard.o memory.o screen.o stack.o
CC = gcc
L_FLAGS = -lSDL2
OBJ_NAME = chip8

all: $(OBJS)
	$(CC) -c $(OBJS)
	$(CC) $(CMP) $(L_FLAGS) -o $(OBJ_NAME)
	
.PHONY: clean
clean:
	rm -f $(OBJ_NAME) $(CMP)
