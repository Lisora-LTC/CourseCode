#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <iomanip>

void input_judge(int board [4][4],char &inn, int &exitcode);
bool exit_confirm();
void play_game();

void wait_for_enter();
void print_menu();
void print_help();
void print_exit();
void print_interface(int board[4][4], int score, int step);
void input_fault();

void initialize_board(int board[4][4]);
void random_Generate(int board[4][4]);

void num_move(int board[4][4], int type, bool &realmerged);
void clear_screen_alternative();
bool findMax(int board[4][4]);
bool is_game_over(int board[4][4]);
#endif // GAME_H
