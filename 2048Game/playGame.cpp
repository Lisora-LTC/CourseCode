#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <iomanip>
#include "Game.h"
using namespace std;

void play_game()
{
	// 变量声明
	int board[4][4] = {0}; // 4*4的棋盘
	int score = 0;		  // 分数
	int step = 0;		  // 步数
	char choice = '\0';	  // 用户选择
    int exitcode=0;
	// 游戏初始化，包括生成随机数等操作
    
	// 打印游戏界面
	print_interface(board, score, step);

	while (1)
	{
        exitcode=0;
		// 获取用户输入
		choice = _getch();
		
		// 根据用户输入进行相应操作
        input_judge(board,choice,exitcode);
		// 更新相关数据

		// 打印游戏界面
        if(exitcode!=2){
            print_interface(board, score, step);
        }
		

		// 判断游戏是否结束，如果结束则跳出循环
        if(exitcode==1){
            return;
        }
	}

	// 游戏结束
	print_exit();
}

void num_move(int board[4][4],int type){
    // 定义起点和步进方向
    int start_x[4] = {0, 3, 0, 0}; // 上、下、左、右
    int start_y[4] = {0, 0, 0, 3};
    int dx[4] = {1, -1, 0, 0};     // 行方向
    int dy[4] = {0, 0, 1, -1};     // 列方向
    // type: 0=up, 1=down, 2=left, 3=right
    for(int i=0;i<4;i++){
        int tmp[4]={0}, idx=0, last=0;
        // 收集非零元素
        for(int j=0;j<4;j++){
            int x, y;
            if(type == 0 || type == 1) { // 上或下，行变，列不变
                x = start_x[type] + dx[type]*j;
                y = i;
            } else { // 左或右，列变，行不变
                x = i;
                y = start_y[type] + dy[type]*j;
            }
            if(board[x][y]!=0){
                tmp[idx++] = board[x][y];
            }
        }
        // 合并相同元素
        int merged[4]={0}, m=0;
        while(last<idx){
            if(last+1<idx && tmp[last]==tmp[last+1]){
                merged[m++] = tmp[last]*2;
                last+=2;
            }else{
                merged[m++] = tmp[last++];
            }
        }
        // 写回到棋盘
        for(int j=0;j<4;j++){
            int x, y;
            if(type == 0 || type == 1) { // 上或下，行变，列不变
                x = start_x[type] + dx[type]*j;
                y = i;
            } else { // 左或右，列变，行不变
                x = i;
                y = start_y[type] + dy[type]*j;
            }
            if (j < m) {
                board[x][y] = merged[j];
            } else {
                board[x][y] = 0;
            }
        }
    }
}

void input_judge(int board[4][4], char inn, int &exitcode) {
    switch (inn) {
    case 27: // ESC 键
        if (exit_confirm()) {
            exitcode = 1; // esc 退出
        }
        break;
    case 72: // 上键
        num_move(board, 0); // 0=up
        break;
    case 80: // 下键
        num_move(board, 1); // 1=down
        break;
    case 75: // 左键
        num_move(board, 2); // 2=left
        break;
    case 77: // 右键
        num_move(board, 3); // 3=right
        break;
    default:
        input_fault();
        exitcode=2;
        break;
    }
}

void input_fault() {
    cout << "输入错误，请重新输入有效的方向键或按 ESC 键退出。" << endl;
}

