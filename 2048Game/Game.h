#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <iomanip>

void input_judge(char inn, int &exitcode);
bool exit_confirm();
void play_game();

void wait_for_enter();
void print_menu();
void print_help();
void print_exit();
void print_interface(int board[4][4], int score, int step);
void input_fault();

void num_move(int type);

#endif // GAME_H
