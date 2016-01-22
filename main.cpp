
#include "scriptapp.h"

//梦幻西游2鼠标和真实的窗口座标有飘逸(误差)
//而且每次都不同

//1, 每次移动鼠标, 找图确定鼠标位置, /2再次匹配, 直到匹配到
//2, 1像素的逐渐移动,


int main(int argc, char *argv[])
{

    ScriptApp app;
    app.Run();

    return 0;
}

