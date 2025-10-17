CC      := cc
CFLAGS  := -std=c11 -Wall -Wextra -O2
SRC     := 4.c
TARGET  := 4

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)