# Makefile for PointerPro Animate
#
# Default target: compile animate.o with Position Independent Code (PIC)

CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -fPIC

# Source files
SRC_DIR = src
SRCS = $(SRC_DIR)/canvas.c $(SRC_DIR)/sprite.c $(SRC_DIR)/placement.c $(SRC_DIR)/frame.c

# Header directories
INC_DIR = include

.PHONY: all clean test demo

# Default target: compile animate.o for submission
all: animate.o

# Compile all source files into individual .o files, then archive into animate.a
# The marking system will compile this into a shared object
animate.o: $(SRCS) $(INC_DIR)/animate.h $(SRC_DIR)/animate_internal.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/canvas.c -o canvas.o
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/sprite.c -o sprite.o
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/placement.c -o placement.o
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $(SRC_DIR)/frame.c -o frame.o
	ar rcs animate.a canvas.o sprite.o placement.o frame.o
	cp animate.a animate.o
	rm -f canvas.o sprite.o placement.o frame.o

# Clean build artifacts
clean:
	rm -f *.o *.a

# Run tests (requires ASAN build)
test: $(SRCS) $(INC_DIR)/animate.h $(SRC_DIR)/animate_internal.h
	$(CC) $(CFLAGS) -fsanitize=address -I$(INC_DIR) -c $(SRC_DIR)/canvas.c -o canvas.o
	$(CC) $(CFLAGS) -fsanitize=address -I$(INC_DIR) -c $(SRC_DIR)/sprite.c -o sprite.o
	$(CC) $(CFLAGS) -fsanitize=address -I$(INC_DIR) -c $(SRC_DIR)/placement.c -o placement.o
	$(CC) $(CFLAGS) -fsanitize=address -I$(INC_DIR) -c $(SRC_DIR)/frame.c -o frame.o
	$(CC) $(CFLAGS) -fsanitize=address -I$(INC_DIR) demo.c canvas.o sprite.o placement.o frame.o -o demo -lm
	./demo
	rm -f canvas.o sprite.o placement.o frame.o
