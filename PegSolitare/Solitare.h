#ifndef SOLITARE_H
#define SOLITARE_H

#include <easyx.h>
#include <string>
using namespace std;

class Button {
private:
    int x, y, width, height;
    const TCHAR* text; // 修改为 const TCHAR*

public:
    Button(int x, int y, int width, int height, const TCHAR* text); // 修改构造函数
    void draw() const;
    bool isClicked(int mouseX, int mouseY) const;//const 用来表示不会修改变量，只读
};

class Chess {
    // 类内部暂时不需要定义
};

void init();       // 初始化图形界面
void mainMenu();   // 渲染主菜单
void chooseGame(); //选择关卡界面
#endif // SOLITARE_H
