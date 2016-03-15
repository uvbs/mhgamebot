#ifndef HELPERFUN_H
#define HELPERFUN_H

#include "define.h"

#include <Shlwapi.h>


//安装自动启动
bool install_autorun();
bool uninstall_autorun();

int GetBytesPerPixel(int depth);
int GetBytesPerRow(int width, int depth);
int GetBitmapBytes(int width, int height, int depth);

int make_mouse_value(int x, int y);

#endif // HELPERFUN_H

