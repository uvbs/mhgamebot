#ifndef HELPERFUN_H
#define HELPERFUN_H


#include <Shlwapi.h>
#include <Windows.h>

#include <QDebug>
#include <regex>
#include <opencv2/opencv.hpp>

using namespace cv;


//安装自动启动
bool install_autorun();
bool uninstall_autorun();

int GetBytesPerPixel(int depth);
int GetBytesPerRow(int width, int depth);
int GetBitmapBytes(int width, int height, int depth);

int make_mouse_value(int x, int y);

CvPoint getNextMinLoc(IplImage* result , int templatWidth,int templatHeight,double maxValIn , CvPoint lastLoc);

template <typename T>
T readmem(HANDLE process, std::string addr_string)
{
    std::vector<std::string> re;
    std::regex r("[+-]?\\w+");
    const std::sregex_token_iterator end;  //需要注意一下这里

    for (std::sregex_token_iterator i(addr_string.begin(),addr_string.end(), r); i != end ; ++i){
        re.push_back(*i);
    }

    T result;
    DWORD value = 0;
    DWORD addr;

    qDebug() << addr_string.c_str() << endl;

    //read mem
    for(int i = 0; i < re.size(); i++){
        addr = value + std::stoi(re[i], nullptr, 16);
        qDebug() << "read mem " << addr;
       ::ReadProcessMemory(process, (LPVOID)addr, &value, sizeof(DWORD), NULL);
        qDebug() << " result " << value;
    }


    return (T)value;
}

std::string int2hex(int i);


#endif // HELPERFUN_H

