# Makefile for compiling the Tetris game

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror

# Libraries
LIBS = -lSDL2 -lSDL2_ttf -lpthread

# Source files
SRCS = main.c tetris_game.c client_utils.c ultis.c

# Object files
OBJS = $(SRCS:.c=.o)

# Output executable
TARGET = game.o

# Default target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean