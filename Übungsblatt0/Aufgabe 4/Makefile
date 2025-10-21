CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -I.

OBJ  = 4.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

4: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

