# Not-Tetris-In-C
Block Puzzle Piece Fitting game made in C with PDCURSES

To compile:

Just run make that's it 

or 
gcc -Iinclude main.c -Llib -lpdcurses -o program.exe(or some other name)
(you can also run strip to save a couple of KB for fun)

To play:

CTetris-Terminal.exe On windows
./CTetris-Terminal.exe on linux

Controls and everything is shown on the main menu everything else is pretty self explanatory

Has most tetris features:
Levels (Increasing gravity and more points)
Scoring (Points for lines cleared)
Ghost pieces (View of piece at bottom of board in white)
Next piece view (Can see the first very next piece you'll get)
Held Piece (Holding pieces for later)
