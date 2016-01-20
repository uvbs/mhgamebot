#ifndef WRITEBMP_H
#define WRITEBMP_H


#include <windows.h>
#include <string>
#include <vector>





BOOL WriteBmp(const std::string &strFile,const std::vector<BYTE> &vtData,const SIZE &sizeImg);
BOOL WriteBmp(const std::string &strFile,HDC hdc);
BOOL WriteBmp(const std::string &strFile,HDC hdc,const RECT &rcDC);

#endif // WRITEBMP_H
