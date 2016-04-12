#include "helperfun.h"
#include <mutex>
#include <assert.h>


//安装自动启动
bool install_autorun()
{
    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);

    //写入注册表，以便开机自动运行
    HKEY hKey;
    bool result = false;

    //打开注册表
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey ) == ERROR_SUCCESS)
    {
        //新增一个值,名称随意命名,值为要开机运行的文件的完整路径
        RegSetValueExA(hKey, "mhbot", 0, REG_SZ, (const BYTE*)buf, MAX_PATH);
        //关闭注册表:
        RegCloseKey(hKey);

        result = true;
    }

    return result;
}

bool uninstall_autorun()
{

    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);

    bool result = false;
    HKEY hKey;

    //打开注册表:路径如下
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        //删除键值
        RegDeleteValueA(hKey, "mhbot");
        //关闭注册表
        RegCloseKey(hKey);

        result = true;
    }

    return result;
}

int GetBytesPerPixel(int depth)
{
    return (depth==32 ? 4 : 3);
}

int GetBytesPerRow(int width, int depth)
{
    int bytesPerPixel = GetBytesPerPixel(depth);
    int bytesPerRow = ((width * bytesPerPixel + 3) & ~3);
    return bytesPerRow;
}

int GetBitmapBytes(int width, int height, int depth)
{
    return height * GetBytesPerRow(width, depth);
}

int make_mouse_value(int x, int y)
{
    int v = 0;
    v = x;
    y = y << 16;
    v = v + y;

    return v;
}



CvPoint getNextMinLoc(IplImage* result, int templatWidth,int templatHeight, double maxValIn, CvPoint lastLoc)
{

    int y,x;
    int startY,startX,endY,endX;

    //计算大矩形的左上角坐标
    startY = lastLoc.y - templatHeight;
    startX = lastLoc.x - templatWidth;

    //计算大矩形的右下角的坐标  大矩形的定义 可以看视频的演示
    endY = lastLoc.y + templatHeight;
    endX = lastLoc.x + templatWidth;

    //不允许矩形越界
    startY = startY < 0 ? 0 : startY;
    startX = startX < 0 ? 0 : startX;
    endY = endY > result->height-1 ? result->height-1 : endY;
    endX = endX > result->width - 1 ? result->width - 1 : endX;

    //将大矩形内部 赋值为最大值 使得 以后找的最小值 不会位于该区域  避免找到重叠的目标
    for(y=startY;y<endY;y++){
        for(x=startX;x<endX;x++){
            cvSetReal2D(result,y,x,maxValIn);
        }
    }


    double minVal;
    double maxVal;
    CvPoint minLoc;
    CvPoint maxLoc;

    //查找result中的最小值 及其所在坐标
    cvMinMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc,NULL);
    if(maxVal > 0.9)
    {
        return true;
    }


    return false;


}



std::string int2hex(int i)
{
    std::stringstream stream;
    stream << std::hex << i;
    return stream.str();
}
