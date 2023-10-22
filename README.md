# Not-Tetris-In-C

## Not-Tetris-In-C is a block puzzle piece fitting game made in C with PDCURSES. It has most of the features found in Tetris games, including levels, scoring, ghost pieces, next piece view, and held pieces.

### Installation
To compile the game, simply run make in the project directory.

Alternatively, you can use the following command: `gcc -Iinclude main.c -Llib -lpdcurses -o program.exe`

You can also run strip to reduce the size of the executable.


### Usage
To play the game on Windows, run `CTetris-Terminal.exe`

On Linux, run `./CTetris-Terminal.exe` (although you can't compile on linux maybe try linking ncurses instead and change the include in main.c)

The controls and other information are displayed on the main menu, and the gameplay is pretty self-explanatory.

### Features
- [x] Levels: The game has increasing gravity and more points as the player progresses through the levels.

- [x] Scoring: The player earns points for clearing lines.

- [x] Ghost pieces: The game displays a view of the piece at the bottom of the board in white.

- [x] Next piece view: The player can see the next piece that will appear on the board.

- [x] Held piece: The player can hold a piece for later use.
