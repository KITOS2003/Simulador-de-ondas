CC = gcc

CFLAGS = -g -ansi -pedantic -Wall -Wextra -std=c11 -O3
LINK = -lm

BUILD=build
SOURCE=src

OBJ= $(BUILD)/main.o $(BUILD)/lodepng.o $(BUILD)/png_encoder.o $(BUILD)/wave_solver.o $(BUILD)/simulation_maker.o

$(BUILD)/%.o: $(SOURCE)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

wave_sim: $(OBJ)
	$(CC) -o $@ $^ $(LINK)

all: wave_sim
