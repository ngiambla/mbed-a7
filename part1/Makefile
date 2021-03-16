
CC=gcc
CFLAGS=
OBJ = part1.o 

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

part1.exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)