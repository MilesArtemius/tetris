#include "tetris.h"

// gcc -pthread tetris.c && ./a.out

void* seek(void* arg) {
    while (command != ESCAPE) {
        command = (char) tolower(getchar());
    }
    endgame = true;
    return NULL;
}

void* loop(void* arg) {
    pthread_t command_seeker;
    initTermios();
    pthread_create(&command_seeker, NULL, &seek, NULL);

    next = rand() % 7;
    choose_next();

    redraw_score();
    while (!endgame) {
        process_game();
        redraw_all();

        nanosleep(&ellipse, NULL);
    }

    pthread_cancel(command_seeker);
    resetTermios();

    return NULL;
}

int main() {
    pthread_t main_thread;
    int result;

    ellipse.tv_sec = 0;
    ellipse.tv_nsec = 500000000L;

    fill_in_figures_info();

    init_field();

    srand((byte) time(0));

    redraw_status(IN_TIME); //TODO: correct.

    pthread_create(&main_thread, NULL, &loop, NULL);
    result = pthread_join(main_thread, NULL);

    printf("Game ended with result: %i\n", result);
    return 0;
}





/* Initialize new terminal i/o settings */
void initTermios() {
    printf("\033[?25l");

    tcgetattr(0, &old); /* grab old terminal i/o settings */
    new = old; /* make new settings same as old settings */
    new.c_lflag |= ~ICANON; /* disable buffered i/o */
    new.c_lflag &= ~ECHO; /* set no echo mode */
    tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios() {
    printf("\033[?25h");

    tcsetattr(0, TCSANOW, &old);
}





void fill_in_figures_info() {
    figure_info = malloc(7 * sizeof(Figure));
    figure_info[LINE].width = 1;
    figure_info[LINE].height = 4;
    figure_info[SQUARE].width = 2;
    figure_info[SQUARE].height = 2;
    figure_info[L_SHAPE].width = 2;
    figure_info[L_SHAPE].height = 3;
    figure_info[L_SHAPE_REVERSED].width = 2;
    figure_info[L_SHAPE_REVERSED].height = 3;
    figure_info[S_SHAPE].width = 3;
    figure_info[S_SHAPE].height = 2;
    figure_info[S_SHAPE_REVERSED].width = 3;
    figure_info[S_SHAPE_REVERSED].height = 2;
    figure_info[T_SHAPE].width = 3;
    figure_info[T_SHAPE].height = 2;
}





void init_field() {
    field = (Pixel**) calloc(WIDTH, sizeof(Pixel*));
    for (int i = 0; i < WIDTH; i++) {
        field[i] = (Pixel*) calloc(HEIGHT, sizeof(Pixel));
        for (int j = 0; j < HEIGHT; j++) {
            field[i][j].graph = ' ';
            field[i][j].state = empty;
        }
    }

    for (int i = 0; i < 30; i++) {
        field[16][i].graph = (char) 254;
        field[16][i].state = protected;
        field[0][i].graph = (char) 254;
        field[0][i].state = protected;
    }
    for (int i = 0; i < 17; i++) {
        field[i][29].graph = (char) 254;
        field[i][29].state = protected;
    }
}

void redraw_all() {
    printf("\033[;H\033[3J\033[2J");
    for (int i = 4; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            printf("%c", field[j][i].graph);
        }
        printf("\n");
    }
}

void redraw_field() { //TODO: actually redraw.
    clear(1, 29, 15, 25);

}

void redraw_status(char* status) { //TODO: actually redraw.
    int status_len = (int) strlen(status);
    int delta = (WIDTH - status_len) / 2;
    if (delta > 0) {
        for (int i = 0; i < status_len; ++i) {
            field[i+(delta)][31].graph = status[i];
        }
    }
}

void redraw_score() { //TODO: actually redraw.
    char next [6] = "Score:";
    for (int i = 21; i < 27; ++i) {
        field[i][20].graph = next[i-21];
    }

    char score_string[6];
    sprintf(score_string, "%i", score);
    int score_string_len = (int) strlen(score_string);
    int delta = (6 - score_string_len) / 2;
    for (int i = 0; i < score_string_len; ++i) {
        field[i+21+delta][22].graph = score_string[i];
    }
}

void redraw_next(int figure) { //TODO: actually redraw.
    char next [5] = "Next:";
    for (int i = 21; i < 27; ++i) {
        field[i][10].graph = next[i-21];
    }

    clear(21, 12, 5, 5);

    int delta_x = (5 - figure_info[figure].width) / 2;
    int delta_y = 4 - figure_info[figure].height;
    draw_figure(21 + delta_x, 12 - delta_y, figure);
}





void process_game() {
    boolean is_whole;

    for (int j = 4; j <= 30; j++) {
        is_whole = true;
        for (int i = 0; i < 17; i++) {
            if ((field[i][j].state == set) || (field[i][j].state == protected)) {
                if (field[i][j-1].state == moving) recursively_freeze(i, j-1);
                if  ((command == LEFT) && (i < 16) && (field[i+1][j].state == moving)) command = 1;
                if  ((command == RIGHT) && (i > 0) && (field[i-1][j].state == moving)) command = 1;
            }
            if ((i > 0) && (i < 16) && (field[i][j].state != set)) is_whole = false;
        }
        if (is_whole) {
            for (int n = j; n >= 0; n--) {
                for (int m = 1; m < 16; m++) {
                    field[m][n] = field[m][n-1];
                }
            }
            for (int m = 1; m < 16; m++) {
                field[m][0].state = empty;
                field[m][0].graph = ' ';
            }

            score++;
            if (score > 999999) {
                redraw_status(VICTORY);
                endgame = true;
            } else {
                redraw_score();
            }
        }
    }

    for (int k = 1; k < 16; k++) {
        if (field[k][4].state == set) {
            redraw_status(LOSS);
            endgame = true;
        }
    }

    smth_moved = false;

    for (int j = 30; j >= 0; j--) {
        if (command == LEFT) {
            for (int i = 0; i <= 17; i++) {
                if ((i - 1 >= 0) && (field[i][j].state == moving)) {
                    field[i - 1][j].state = moving;
                    field[i - 1][j].graph = field[i][j].graph;
                    field[i][j].state = empty;
                    field[i][j].graph = ' ';
                }
            }
        } else if (command == RIGHT) {
            for (int i = 17; i >= 0; i--) {
                if ((i + 1 <= 16) && (field[i][j].state == moving)) {
                    field[i + 1][j].state = moving;
                    field[i + 1][j].graph = field[i][j].graph;
                    field[i][j].state = empty;
                    field[i][j].graph = ' ';
                }
            }
        }

        for (int i = 0; i <= 17; i++) {
            if (field[i][j].state == moving) {
                smth_moved = true;
                field[i][j + 1].state = moving;
                field[i][j + 1].graph = field[i][j].graph;
                field[i][j].state = empty;
                field[i][j].graph = ' ';
            }
        }
    }

    command = 1;
    if (!smth_moved) {
        choose_next();
    }
}

void recursively_freeze(int x, int y) {
    if (field[x][y].state != moving) return;

    field[x][y].state = set;
    field[x][y].graph = '#';

    recursively_freeze(x+1, y);
    recursively_freeze(x-1, y);
    recursively_freeze(x, y+1);
    recursively_freeze(x, y-1);
}

void choose_next() { // there are seven types of figure: -, o, L, Lr, S, Sr and T.
    int pos = (rand() % (16 - figure_info[next].width)) + 1;
    draw_figure(pos, 0, next);

    next = rand() % 7;
    redraw_next(next);
}

void draw_figure(int x, int y, int figure) {
    switch (figure) {
        case LINE:
            draw_line(x, y);
            break;
        case SQUARE:
            draw_square(x, y);
            break;
        case L_SHAPE:
            draw_l_shape(x, y);
            break;
        case L_SHAPE_REVERSED:
            draw_l_shape_reversed(x, y);
            break;
        case S_SHAPE:
            draw_s_shape(x, y);
            break;
        case S_SHAPE_REVERSED:
            draw_s_shape_reversed(x, y);
            break;
        case T_SHAPE:
            draw_t_shape(x, y);
            break;
        default:
            draw_square(x, y);
            break;
    }
}

void draw_line(int x, int y) {
    for (int i = 0; i < 4; i++) {
        field[x][i+y].state = moving;
        field[x][i+y].graph = 'L';
    }
}

void draw_square(int x, int y) {
    for (int i = 2; i < 4; i++) {
        for (int j = x; j < x+2; j++) {
            field[j][i+y].state = moving;
            field[j][i+y].graph = 'S';
        }
    }
}

void draw_l_shape(int x, int y) {
    for (int i = 1; i < 4; i++) {
        field[x][i+y].state = moving;
        field[x][i+y].graph = 'K';
    }
    field[x+1][3+y].state = moving;
    field[x+1][3+y].graph = 'K';
}

void draw_l_shape_reversed(int x, int y) {
    for (int i = 1; i < 4; i++) {
        field[x+1][i+y].state = moving;
        field[x+1][i+y].graph = 'K';
    }
    field[x][3+y].state = moving;
    field[x][3+y].graph = 'K';
}

void draw_s_shape(int x, int y) {
    for (int i = 0; i < 2; i++) {
        field[x+i][2+y].state = moving;
        field[x+i][2+y].graph = 'H';
        field[x+i+1][3+y].state = moving;
        field[x+i+1][3+y].graph = 'H';
    }
}

void draw_s_shape_reversed(int x, int y) {
    for (int i = 0; i < 2; i++) {
        field[x+i][3+y].state = moving;
        field[x+i][3+y].graph = 'H';
        field[x+i+1][2+y].state = moving;
        field[x+i+1][2+y].graph = 'H';
    }
}

void draw_t_shape(int x, int y) {
    for (int i = 0; i < 3; i++) {
        field[x+i][3+y].state = moving;
        field[x+i][3+y].graph = 'Z';
    }
    field[x+1][2+y].state = moving;
    field[x+1][2+y].graph = 'Z';
}

void clear(int x, int y, int width, int height) {
    for (int i = x; i < x+width; i++) {
        for (int j = y; j < y+height; j++) {
            field[i][j].graph = ' ';
            field[i][j].state = protected;
        }
    }
}
