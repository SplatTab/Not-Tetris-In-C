CC = gcc
CFLAGS = -Os -Iinclude
LDFLAGS = -Llib -lpdcurses

all: CTetris-Terminal.exe

CTetris-Terminal.exe: main.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
	strip $@

clean:
	rm main.exe