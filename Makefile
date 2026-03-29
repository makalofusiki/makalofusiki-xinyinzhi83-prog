# Makefile for Final Fantasy Style RPG Game

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
TARGET  = rpg_game
SRCS    = rpg_game.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
