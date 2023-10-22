#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>

#define BOARD_HEIGHT 21 // Height of the board or play area
#define BOARD_WIDTH 10 // Width of the board or play area
#define BLOCK_SIZE 2 // Size of each block in characters
#define GRAVITY_INTERVAL 10 // Default ticks for a block to fall

#define WINDOW_HEIGHT BOARD_HEIGHT + 1 // Window Height 1 bigger than board
#define WINDOW_WIDTH BOARD_WIDTH * BLOCK_SIZE + 15 // Window width multiplied by size of blocks since they are 2 characters + 15 space for stats

typedef struct {
    int x, y;
    int shape[4][4];
} Tetromino; // Structure with a position and shape for a tetromino

int board[BOARD_HEIGHT][BOARD_WIDTH]; // 2D array representing the blocks on the board
int board_colors[BOARD_HEIGHT][BOARD_WIDTH]; // 2D array representing the colors of the blocks on the board
int gravity_interval = GRAVITY_INTERVAL; // How many ticks before a block falls

Tetromino ghost_tetromino = { 0 };
Tetromino next_tetromino = { 0 };
Tetromino tetromino = { 0 };
Tetromino held_tetromino = { 0 };
int held = 0; // Whether a tetromino is being held
int already_held = 0; // Whether the player has already used his hold

int lines_cleared = 0; // Total lines cleared during game
int level = 1; // Current level
int score = 0; // Total score during game
const int points[] = {0, 40, 100, 300, 1200}; // Points based off cleared lines

// Tetris logo for start menu
const char tetris_logo[5][23] = {
    "||| ||| ||| ||  | |||",
    " |  |    |  | |   |  ",
    " |  |||  |  ||| | |||",
    " |  |    |  | | |   |",
    " |  |||  |  | | | |||",
};

// The 7 tetromino shapes from tetris
int tetromino_shapes[7][4][4] = {
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 2, 2, 0},
        {0, 2, 2, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 3, 3},
        {0, 3, 3, 0},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 4, 4, 0},
        {0, 0, 4, 4},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 5, 0},
        {0, 5, 5, 5},
        {0, 0, 0, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 0, 6, 0},
        {0, 0, 6, 0},
        {0, 6, 6, 0}
    },
    {
        {0, 0, 0, 0},
        {0, 7, 0, 0},
        {0, 7, 0, 0},
        {0, 7, 7, 0}
    }
};

void draw_board() {
    // Draw top line seperating game from stats
    for (int j = 0; j < BOARD_WIDTH + 15; j++) {
        attron(COLOR_PAIR(8));
        mvprintw(1, j * BLOCK_SIZE, "__");
        attroff(COLOR_PAIR(8));
    }

    // Draw board and blocks
    for (int i = 1; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            // If spot in board is 0 draw space otherwise we draw block
            if (board[i][j] == 0) {
                mvprintw(i+1, j * BLOCK_SIZE, "  ");
            } else {
                attron(COLOR_PAIR(board_colors[i][j]));
                mvprintw(i+1, j * BLOCK_SIZE, "[]");
                attroff(COLOR_PAIR(board_colors[i][j]));
            }
        }

        // Draw right border
        attron(COLOR_PAIR(8));
        mvprintw(i+1, BOARD_WIDTH * BLOCK_SIZE, "||");
        attroff(COLOR_PAIR(8));
    }

    // Draw next shape on the right side of the board
    mvprintw(5, (BOARD_WIDTH + 3) * BLOCK_SIZE, "NEXT");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (next_tetromino.shape[i][j] != 0) {
                attron(COLOR_PAIR(next_tetromino.shape[i][j]));
                mvprintw(i+6, (j + BOARD_WIDTH + 2) * BLOCK_SIZE, "[]");
                attroff(COLOR_PAIR(next_tetromino.shape[i][j]));
            } else {
                mvprintw(i+6, (j + BOARD_WIDTH + 2) * BLOCK_SIZE, "  ");
            }
        }
    }

    // Draw held shape on the right side of the board
    mvprintw(11, (BOARD_WIDTH + 3) * BLOCK_SIZE, "HELD");
    if (held) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (held_tetromino.shape[i][j] != 0) {
                    attron(COLOR_PAIR(held_tetromino.shape[i][j]));
                    mvprintw(i+12, (j + BOARD_WIDTH + 2) * BLOCK_SIZE, "[]");
                    attroff(COLOR_PAIR(held_tetromino.shape[i][j]));
                } else {
                    mvprintw(i+12, (j + BOARD_WIDTH + 2) * BLOCK_SIZE, "  ");
                }
            }
        }
    } else {
        mvprintw(12, (BOARD_WIDTH + 2) * BLOCK_SIZE, "  ");
        mvprintw(13, (BOARD_WIDTH + 2) * BLOCK_SIZE, "  ");
        mvprintw(14, (BOARD_WIDTH + 2) * BLOCK_SIZE, "  ");
        mvprintw(15, (BOARD_WIDTH + 2) * BLOCK_SIZE, "  ");
    }
}

// Draw's a tetromino
void draw_tetromino(const Tetromino t, int ghost) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j] != 0) {
                int color = ghost ? 8 : t.shape[i][j];
                attron(COLOR_PAIR(color));
                mvprintw(t.y + i, (t.x + j) * BLOCK_SIZE, "[]");
                attroff(COLOR_PAIR(color));
            }
        }
    }
}

// Clears a tetromino
void clear_tetromino(const Tetromino t) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j] != 0) {
                mvprintw(t.y + i, (t.x + j) * BLOCK_SIZE, "  ");
            }
        }
    }
}

// Checks if a tetromino is colliding with the board
int check_collision(const Tetromino t) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j] != 0) {
                int x = t.x + j;
                int y = t.y + i;
                if (x < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT || board[y][x] != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Adds a tetromino to the board
void add_tetromino(const Tetromino t) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j] != 0) {
                board[t.y + i][t.x + j] = 1;
                board_colors[t.y + i][t.x + j] = t.shape[i][j];
            }
        }
    }
}

// Rotates a tetromino shape
void rotate_tetromino(Tetromino* t) {
    // Rotate shape 90 degrees clockwise
    int temp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = t->shape[i][j];
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            t->shape[i][j] = temp[3 - j][i];
        }
    }

    // Check if rotated shape hit's board if it does push one back
    if (t->x < 0) {
        t->x = 0;
    } else if (t->x + 4 > BOARD_WIDTH) {
        t->x = BOARD_WIDTH - 4;
    }
    if (t->y + 4 > BOARD_HEIGHT) {
        t->y = BOARD_HEIGHT - 4;
    } else if (t->y <= 0) { // Ensure tetromino doesn't hit top of board
        t->y = 1;
    }
}

void place_tetrimino_board(const Tetromino t)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (t.shape[i][j] != 0) {
                int x = t.x + j;
                int y = t.y + i;
                board[y][x] = t.shape[i][j];
            }
        }
    }
}

// Clear's tetris rows if they are full and displays/updates stats
void clear_rows() {
    // Check for full rows and clear them
    int cleared = 0;
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        int row_full = 1;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] == 0) {
                row_full = 0;
                break;
            }
        }
        if (row_full) {
            cleared++;
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < BOARD_WIDTH; j++) {
                    board[k][j] = board[k - 1][j];
                    board_colors[k][j] = board_colors[k - 1][j];
                }
            }
            for (int j = 0; j < BOARD_WIDTH; j++) {
                board[0][j] = 0;
                board_colors[0][j] = 0;
            }
            i++;
        }
    }

    // If any rows are cleared update stats
    if (cleared > 0) {
        // Show added stats
        int score_increase = points[cleared] * level;
        mvprintw(0, 0, "Lines: %d(+%d) PTS: %d(+%d)", lines_cleared, cleared, score, score_increase);
        mvprintw(BOARD_HEIGHT - 1, BOARD_WIDTH * BLOCK_SIZE + 5, "LVL: %d", level);
        refresh();
        napms(600);
        // Clear text
        move(0, 0);
        clrtoeol();

        // Increment stats
        lines_cleared += cleared;
        score += score_increase;
        if (lines_cleared >= 10 * level && level < 10) {
            level++;
            gravity_interval -= 1;

            move(BOARD_HEIGHT - 1, BOARD_WIDTH * BLOCK_SIZE + 5);
            clrtoeol();
            refresh();
            napms(200);
            flash();
            attron(COLOR_PAIR(11));
            mvprintw(BOARD_HEIGHT - 1, BOARD_WIDTH * BLOCK_SIZE + 5, "LVL UP!");
            attroff(COLOR_PAIR(11));
            refresh();
            napms(200);
            move(BOARD_HEIGHT - 1, BOARD_WIDTH * BLOCK_SIZE + 5);
            clrtoeol();
            refresh();
            napms(200);
        }

        // And show updated stats
        mvprintw(0, 0, "Lines: %d PTS: %d", lines_cleared, score);
        mvprintw(BOARD_HEIGHT - 1, BOARD_WIDTH * BLOCK_SIZE + 5, "LVL: %d ", level);
    }
}

int main() {
    // Initialization
    initscr();
    resize_term(WINDOW_HEIGHT, WINDOW_WIDTH);
    PDC_set_title("C Tetris Terminal");
    srand(time(NULL));
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);


    start_color();

    // Full colors for tetrominos and game
    init_pair(1, COLOR_CYAN, COLOR_CYAN); // I
    init_pair(2, COLOR_YELLOW, COLOR_YELLOW); // O
    init_pair(3, COLOR_GREEN, COLOR_GREEN); // S
    init_pair(4, COLOR_RED, COLOR_RED); // Z
    init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA); // T
    init_pair(6, COLOR_BLUE, COLOR_BLUE); // L
    // For J tetromino we add in a orange color but this may be gray in you're terminal if unsupported
    init_color(8, 1000, 500, 0); // Orange
    init_pair(7, 8, 8); // J (Orange)
    init_pair(8, COLOR_WHITE, COLOR_WHITE); // For borders and ghost piece

    // Menu colors
    init_pair(9, COLOR_RED, COLOR_BLACK); // Game over
    init_pair(10, COLOR_CYAN, COLOR_BLACK); // Press any key to start
    init_pair(11, COLOR_GREEN, COLOR_BLACK); // LEVEL UP! and Made by me

    // Start menu
    while (getch() == ERR)
    {
        // Print the "Tetris" logo with each letter in a different color of the rainbow
        for (int line = 0; line < 5; line++) {
            for (int col = 0; col < 23; col++) {
                if (tetris_logo[line][col] == '|') {
                    chtype color = COLOR_PAIR(rand() % 6 + 1);
                    attron(color);
                    mvprintw(line + 4, col + 7, "%c", tetris_logo[line][col]);
                    attroff(color);
                } else {
                    mvprintw(line + 4, col + 7, "%c", tetris_logo[line][col]);
                }
            }
        }

        attron(COLOR_PAIR(10));
        mvprintw(13, (WINDOW_WIDTH - 35) / 2, "A/D - Move | W - Rotate | C - Hold");
        mvprintw(14, 1, "S - Soft Drop | Space - Hard Drop");
        mvprintw(16, (WINDOW_WIDTH - 22) / 2, "Press any key to start");
        attroff(COLOR_PAIR(10));
        attron(COLOR_PAIR(11));
        mvprintw(WINDOW_HEIGHT - 1, (WINDOW_WIDTH - 17) / 2, "Made by: SplatTab");
        attroff(COLOR_PAIR(11));
        refresh();
    }

    clear();

    // Game loop starts here
    do {
    // Clear board
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board[i][j] = 0;
        }
    }

    int game_over = 0;
    int gravity_timer = 0;
    int rand_shape = rand() % 7;
    int next_rand_shape = rand() % 7;
    memcpy(tetromino.shape, tetromino_shapes[rand_shape], sizeof(tetromino_shapes[rand_shape]));
    memcpy(next_tetromino.shape, tetromino_shapes[next_rand_shape], sizeof(tetromino_shapes[next_rand_shape]));
    held = 0;
    lines_cleared = 0;
    score = 0;
    level = 1;
    gravity_interval = GRAVITY_INTERVAL;

    // Main Game loop
    while (!game_over) {
        // Clear tetromino and set it's position to top middle
        clear_tetromino(tetromino);
        tetromino.x = BOARD_WIDTH / 2 - 2;
        tetromino.y = 0;

        // Set tetrimino to next tetrimino and generate the next tetrimino
        int next_shape = rand() % 7;
        memcpy(tetromino.shape, next_tetromino.shape, sizeof(next_tetromino.shape));
        memcpy(next_tetromino.shape, tetromino_shapes[next_shape], sizeof(tetromino_shapes[next_shape]));

        // If tetromino is not colliding check for user input and move tetromino down
        while (!check_collision(tetromino)) {
            if (is_termresized()) resize_term(0, 0);
            draw_board();
            draw_tetromino(tetromino, 0);

            ghost_tetromino = tetromino; // Update ghost tetromino
            while (!check_collision(ghost_tetromino)) ghost_tetromino.y++; // Move it down until it hits something
            draw_tetromino(ghost_tetromino, 1);

            refresh();

            // ---------------------------------------------------------------------------------------------
            // INPUT
            int ch = getch();
            switch (ch) {
                // Tetrimino moved left and right with A and D
                case 'a':
                    tetromino.x--;
                    if (check_collision(tetromino)) tetromino.x++; // If colliding with board push back
                    break;
                case 'd':
                    tetromino.x++;
                    if (check_collision(tetromino)) tetromino.x--;
                    break;
                // Moved down with s
                case 's':
                    tetromino.y++;
                    // if colliding with ground or another block add to the board
                    if (check_collision(tetromino)) {
                        tetromino.y--;
                        place_tetrimino_board(tetromino);
                        add_tetromino(tetromino);
                    }
                    break;
                // Hard drop with space
                case ' ':
                    while (!check_collision(tetromino)) {
                        tetromino.y++;
                    }
                    tetromino.y--;
                    place_tetrimino_board(tetromino);
                    add_tetromino(tetromino);
                    break;
                // Rotate with W
                case 'w':
                    rotate_tetromino(&tetromino);
                    break;
                // Hold tetromino with C
                case 'c':
                    if (!already_held) {
                        if (!held) {
                            // If no tetromino is held, hold the current tetromino and generate a new one
                            held_tetromino = tetromino;
                            held = 1;
                            memcpy(tetromino.shape, next_tetromino.shape, sizeof(next_tetromino.shape));
                            memcpy(next_tetromino.shape, tetromino_shapes[rand() % 7], sizeof(tetromino_shapes[next_shape]));
                        } else {
                            // If a tetromino is already held, swap it with the current tetromino
                            Tetromino temp = tetromino;
                            tetromino = held_tetromino;
                            held_tetromino = temp;
                        }
                        // Reset tetromino position and already_held flag
                        tetromino.x = BOARD_WIDTH / 2 - 2;
                        tetromino.y = 0;
                        already_held = 1;
                    }
                    break;
            }
            // ---------------------------------------------------------------------------------------------

            // Gravity timer increased every tick and if it reaches GRAVITY_INTERVAL tetromino is moved down as normal
            gravity_timer++;
            if (gravity_timer >= gravity_interval) {
                tetromino.y++;
                if (check_collision(tetromino)) {
                    tetromino.y--;
                    place_tetrimino_board(tetromino);
                    add_tetromino(tetromino);
                    break;
                }
                gravity_timer = 0;
            }
        }

        if (tetromino.y <= 1) {
            game_over = 1;
            break;
        }

        already_held = 0;
        add_tetromino(tetromino);
        clear_rows();
    }

    // ---------------------------------------------------------------------------------------------
    // GAME OVER
    attron(COLOR_PAIR(9));
    mvprintw(BOARD_HEIGHT / 2, 0, "GAME OVER");
    mvprintw(BOARD_HEIGHT / 2 + 1, 0, "PRESS R TO RESTART");
    attroff(COLOR_PAIR(9));
    refresh();

    // Wait for user to press r to restart and then loop restarts back at top
    int ch;
    do {
        if (is_termresized()) resize_term(0, 0);
        ch = getch();
    } while (ch != 'r');

    clear();
    // ---------------------------------------------------------------------------------------------
    } while (1);


    endwin();
    return 0;
}