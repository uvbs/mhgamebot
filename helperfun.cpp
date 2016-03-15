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
