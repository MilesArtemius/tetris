#include <ctype.h>
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

    choose_next();
    while (!endgame) {
        redraw_all();
        process_game();

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

void redraw_field() {

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

void redraw_score() {

}

void redraw_next() {

}





void process_game() {
    smth_moved = false;
    for (int i = 1; i < 15; i++) {
        for (int j = 25; j > 0; j--) {
            if (field[i][j].state == moving) {
                smth_moved = true;
                if ((field[i][j+1].state == set) || (field[i][j+1].state == protected)) {
                    field[i][j].state = set;
                    field[i][j].graph = '#';
                } else {
                    field[i][j+1].state = moving;
                    field[i][j+1].graph = field[i][j].graph;
                    field[i][j].state = empty;
                    field[i][j].graph = ' ';
                }
            }
        }
    }

    if (!smth_moved) {
        choose_next();
    }
}

void choose_next() { // there are seven types of figure: -, o, L, Lr, S, Sr and T.
    int figure = rand() % 7;
    int pos = (rand() % (17 - figure_info[figure].width)) + 1;
    switch (figure) {
        case LINE:
            draw_line(pos);
            break;
        case SQUARE:
            draw_square(pos);
            break;
        case L_SHAPE:
            draw_l_shape(pos);
            break;
        case L_SHAPE_REVERSED:
            draw_l_shape_reversed(pos);
            break;
        case S_SHAPE:
            draw_s_shape(pos);
            break;
        case S_SHAPE_REVERSED:
            draw_s_shape_reversed(pos);
            break;
        case T_SHAPE:
            draw_t_shape(pos);
            break;
        default:
            draw_square(pos);
            break;
    }

    //int num = (rand() % (upper - lower + 1)) + lower;

    redraw_next();
}

void draw_line(int pos) {
    for (int i = 0; i < 5; i++) {
        field[pos][i].state = moving;
        field[pos][i].graph = 'L';
    }
}

void draw_square(int pos) {
    for (int i = 2; i < 5; i++) {
        for (int j = pos; j < pos+2; j++) {
            field[j][i].state = moving;
            field[j][i].graph = 'L';
        }
    }
}

void draw_l_shape(int pos) {
    for (int i = 1; i < 5; i++) {
            field[pos][i].state = moving;
            field[pos][i].graph = 'L';
    }
    field[pos+1][4].state = moving;
    field[pos+1][4].graph = 'L';
}

void draw_l_shape_reversed(int pos) {
    for (int i = 1; i < 5; i++) {
        field[pos+1][i].state = moving;
        field[pos+1][i].graph = 'L';
    }
    field[pos][4].state = moving;
    field[pos][4].graph = 'L';
}

void draw_s_shape(int pos) {
    for (int i = 0; i < 2; i++) {
        field[pos+i][3].state = moving;
        field[pos+i][3].graph = 'L';
        field[pos+i+1][4].state = moving;
        field[pos+i+1][4].graph = 'L';
    }
}

void draw_s_shape_reversed(int pos) {
    for (int i = 0; i < 2; i++) {
        field[pos+i][4].state = moving;
        field[pos+i][4].graph = 'L';
        field[pos+i+1][3].state = moving;
        field[pos+i+1][3].graph = 'L';
    }
}

void draw_t_shape(int pos) {
    for (int i = 0; i < 3; i++) {
        field[pos+i][4].state = moving;
        field[pos+i][4].graph = 'L';
    }
    field[pos+1][3].state = moving;
    field[pos+1][3].graph = 'L';
}
