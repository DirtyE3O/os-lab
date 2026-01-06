CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

OBJDIR = obj
BINDIR = .
BIN = rle

SRC = src/main.c src/rle.c
OBJ = $(OBJDIR)/main.o $(OBJDIR)/rle.o

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BIN)

.PHONY: all clean