#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <iomanip>
#include "Game.h"
using namespace std;

void play_game()
{
	// 变量声明
	int board[4][4] = {}; // 4*4的棋盘
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
        input_judge(choice,exitcode);
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

void num_move(int type){

}

void input_judge(char inn, int &exitcode) {
    switch (inn) {
    case 27: // ESC 键
        if (exit_confirm()) {
            exitcode = 1; // esc 退出
        }
        break;
    case 72: // 上键
        cout << "检测到上键" << endl;
        // 添加处理上键的逻辑
        break;
    case 80: // 下键
        cout << "检测到下键" << endl;
        // 添加处理下键的逻辑
        break;
    case 75: // 左键
        cout << "检测到左键" << endl;
        // 添加处理左键的逻辑
        break;
    case 77: // 右键
        cout << "检测到右键" << endl;
        // 添加处理右键的逻辑
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

