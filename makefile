# Makefile for the game
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lSDL -lSDL_image -lSDL_ttf  # Add -lSDL_ttf to link against SDL_ttf
OBJECTS = main.o perso.o
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

main.o: main.c perso.h
	$(CC) $(CFLAGS) -c main.c -o main.o

perso.o: perso.c perso.h
	$(CC) $(CFLAGS) -c perso.c -o perso.o

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
