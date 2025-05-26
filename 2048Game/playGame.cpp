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
	long long score = 0;		  // 分数
	int step = 0;		  // 步数
	char choice = '\0';	  // 用户选择
    int exitcode=0;
    bool realmerged=0;
    long long now_score=0;
    bool continued=0;
	// 游戏初始化，包括生成随机数等操作
    initialize_board(board);
	// 打印游戏界面
	print_interface(board, score, step);

	while (1)
	{
        exitcode=0;
		// 获取用户输入
		choice = _getch();
		
		// 根据用户输入进行相应操作
        now_score=0;
        input_judge(board,now_score,choice,exitcode);
        // 判断游戏是否结束，如果结束则跳出循环
        if(exitcode==1){
            return;
        }
        if(exitcode==3){
            score+=now_score;
            step++;
            random_Generate(board);
        }
		
        // 打印游戏界面
		
        print_interface(board, score, step);
        if(exitcode==4){
            if(is_game_over(board)){
                cout << "游戏结束，你无路可走了！按下任意键返回菜单" << endl;
                _getch(); // 等待用户按下任意键
                print_exit();
                return; // 游戏结束返回主菜单
            }
        }
        if(continued==0&&findMax(board)){
            exitcode=5;
        }
        if(exitcode==5&&continued==0){
            continued=1;
            cout << "恭喜你赢了！请选择是否继续游戏，按上下左右继续，按下其他键返回" << endl;
            choice = _getch(); // 等待按下任意键
            if (choice == 0 || choice == (char)224) { // 特殊键的前导码
                choice = _getch(); // 获取真正的键码
            }
            if (choice == 72 || choice == 80 || choice == 75 || choice == 77) { // 上下左右键
                // 继续游戏
                cout << "继续游戏！" << endl;
                exitcode=6;
            }else{
                print_exit();
                return;   // 游戏结束返回主菜单
            }
            
        }
        if(exitcode==6){
            print_interface(board, score, step);
            cout << string(80, ' ')<<endl; // 输出空格覆盖提示信息
            cout << string(80, ' ');
        }
	}
}

void initialize_board(int board[4][4]){
    // 设置随机数种子
    srand(static_cast<unsigned int>(time(0)));
    
    // 随机在两个位置生成2或4
    for(int i = 0; i < 2; i++){
        int row, col, value;
        
        // 确保生成的位置是空的
        do {
            row = rand() % 4;
            col = rand() % 4;
        } while(board[row][col] != 0);
        
        // 80%的概率生成2，20%的概率生成4
        if (rand() % 10 < 8) {
            value = 2;
        } else {
            value = 4;
        }
        
        // 在棋盘上放置数字
        board[row][col] = value;
    }
}

void random_Generate(int board[4][4]){
    srand(static_cast<unsigned int>(time(0)));
    int row, col, value;
    
    // 确保生成的位置是空的
    do {
        row = rand() % 4;
        col = rand() % 4;
    } while(board[row][col] != 0);
    
    // 80%的概率生成2，20%的概率生成4
    if (rand() % 10 < 8) {
        value = 2;
    } else {
        value = 4;
    }
    
    // 在棋盘上放置数字
    board[row][col] = value;
    
}

void num_move(int board[4][4], int type,long long &now_score, bool &realmerged) {
    int original_board[4][4];
    // 1. 记录棋盘的原始状态
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            original_board[r][c] = board[r][c];
        }
    }

    // realmerged 由调用者在调用前初始化为 false

    // 定义起点和步进方向
    int start_x[4] = {0, 3, 0, 0}; // 上、下、左、右
    int start_y[4] = {0, 0, 0, 3};
    int dx[4] = {1, -1, 0, 0};     // 行方向
    int dy[4] = {0, 0, 1, -1};     // 列方向
    // type: 0=up, 1=down, 2=left, 3=right

    for (int i = 0; i < 4; i++) { // 处理棋盘的每一行或每一列
        int tmp[4] = {0}, idx = 0, last = 0;
        // 2. 收集当前行/列的非零元素到 tmp 数组
        for (int j = 0; j < 4; j++) {
            int x, y;
            if (type == 0 || type == 1) { // 向上或向下移动 (处理列)
                x = start_x[type] + dx[type] * j;
                y = i;
            } else { // 向左或向右移动 (处理行)
                x = i;
                y = start_y[type] + dy[type] * j;
            }
            if (board[x][y] != 0) {
                tmp[idx++] = board[x][y];
            }
        }
        
        // 3. 合并 tmp 数组中的相同元素到 merged_elements 数组
        int merged_elements[4] = {0}, m = 0;
        while (last < idx) {
            if (last + 1 < idx && tmp[last] == tmp[last + 1]) {
                merged_elements[m++] = tmp[last] * 2;
                now_score+= tmp[last] * 2;
                last += 2;
            } else {
                merged_elements[m++] = tmp[last++];
            }
        }

        // 4. 将 merged_elements 写回棋盘的当前行/列
        for (int j = 0; j < 4; ++j) {
            int x, y;
            if (type == 0 || type == 1) { // 向上或向下
                x = start_x[type] + dx[type] * j;
                y = i;
            } else { // 向左或向右
                x = i;
                y = start_y[type] + dy[type] * j;
            }
            
            if (j < m) {
                board[x][y] = merged_elements[j];
            } else {
                board[x][y] = 0;
            }
        }
    }

    // 5. 操作完成后，比较当前棋盘与原始棋盘状态
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (board[r][c] != original_board[r][c]) {
                realmerged = true;
                return;            
            }
        }
    }
    
}

void input_judge(int board[4][4], long long &now_score,char &inn, int &exitcode) {
    exitcode=3;
    bool realmerged=0;
    if (inn == 0 || inn == (char)224) { // 特殊键的前导码
        inn = _getch(); // 获取真正的键码
    }
    switch (inn) {
    case 27: // ESC 键
        if (exit_confirm()) {
            print_exit();
            exitcode = 1; // esc 退出
        }else{
            exitcode =6;//覆盖提示信息
        }
        break;
    case 72: // 上键
        num_move(board, 0, now_score, realmerged); // 0=up
        break;
    case 80: // 下键
        num_move(board, 1, now_score,realmerged); // 1=down
        break;
    case 75: // 左键
        num_move(board, 2, now_score,realmerged); // 2=left
        break;
    case 77: // 右键
        num_move(board, 3, now_score,realmerged); // 3=right
        break;
    default:
        input_fault();
        exitcode=2;
        break;
    }
    if(realmerged==0 && exitcode == 3){
        exitcode=4;//没有合并，不生成
    }
    return;
}


void input_fault() {
    cout << "输入错误，请重新输入有效的方向键或按 ESC 键退出。" << endl;
}

bool findMax(int board[4][4]) {
    int maxVal = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (board[i][j] > maxVal) {
                maxVal = board[i][j];
            }
        }
    }
    return maxVal >= 16;
}

bool is_game_over(int board[4][4]) {//检查是否无路可走
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (board[i][j] == 0) return false;
        }
    }
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (board[i][j] == board[i][j + 1]) return false;
        }
    }
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 3; ++i) {
            if (board[i][j] == board[i + 1][j]) return false;
        }
    }
    return true;
}

