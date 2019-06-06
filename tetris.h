#ifndef TETRIS_MAIN_H
#define TETRIS_MAIN_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>

#define byte unsigned char
#define protected 0
#define set 1
#define moving 2
#define empty 3

#define boolean unsigned char
#define true 1
#define false 0

#define WIDTH 30
#define HEIGHT 33

#define IN_TIME "Playing..."
#define VICTORY "Press any key to start playing!"
#define LOSS "Game ended!"

#define ESCAPE 27
#define LEFT 97
#define RIGHT 100

#define LINE 0
#define SQUARE 1
#define L_SHAPE 2
#define L_SHAPE_REVERSED 3
#define S_SHAPE 4
#define S_SHAPE_REVERSED 5
#define T_SHAPE 6

struct timespec ellipse;
static struct termios old, new;
static boolean endgame = false;
static boolean smth_moved = false;
static char command = 1;
static unsigned int score = 0;
int next;

typedef struct {
    char graph;
    byte state;
} Pixel;

typedef struct {
    byte width;
    byte height;
} Figure;

Pixel** field = NULL;
Figure* figure_info = NULL;

void initTermios();
void resetTermios();

void fill_in_figures_info();

void init_field();
void redraw_all();
void redraw_field();
void redraw_status(char* status);
void redraw_score();
void redraw_next(int figure);

void process_game();
void recursively_freeze(int x, int y);
void choose_next();

void draw_figure(int x, int y, int figure);
void draw_line(int x, int y);
void draw_square(int x, int y);
void draw_l_shape(int x, int y);
void draw_l_shape_reversed(int x, int y);
void draw_s_shape(int x, int y);
void draw_s_shape_reversed(int x, int y);
void draw_t_shape(int x, int y);

void clear(int x, int y, int width, int height);

#endif //TETRIS_MAIN_H
