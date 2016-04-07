#include "scriptmanager.h"
#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyle>
#include <QFile>
#include <mutex>
#include <clocale>
#include <DbgHelp.h>


#if defined(Q_OS_WIN32)


//最主要的函数, 生成Dump
void DumpMiniDump(HANDLE hFile, EXCEPTION_POINTERS* excpInfo)
{

    qDebug() << "writing minidump";
    MINIDUMP_EXCEPTION_INFORMATION eInfo;
    eInfo.ThreadId = GetCurrentThreadId(); //把需要的信息添进去
    eInfo.ExceptionPointers = excpInfo;
    eInfo.ClientPointers = FALSE;

    // 调用, 生成Dump. 98不支持
    // Dump的类型是小型的, 节省空间. 可以参考MSDN生成更详细的Dump.
    MiniDumpWriteDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                hFile,
                MiniDumpNormal,
                &eInfo,
                NULL,
                NULL);
}


LONG unhandle_exception_handler(EXCEPTION_POINTERS *ExceptionInfo)
{
    //
    HANDLE hFile = CreateFileA("MiniDump.dmp", GENERIC_READ | GENERIC_WRITE,
                               0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if(hFile == INVALID_HANDLE_VALUE){
        qDebug() << "create dump file fail!";
    }
    else
    {
        qDebug() << "create dump...";
        DumpMiniDump(hFile, ExceptionInfo);
    }

    return 0;
}



#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //当前工作目录
    QString mhbot_dir = QCoreApplication::applicationDirPath();
    QDir::setCurrent(mhbot_dir);



#if defined(Q_OS_WIN32)
    //安装异常处理
    LPTOP_LEVEL_EXCEPTION_FILTER result = SetUnhandledExceptionFilter(unhandle_exception_handler);
    if(result == nullptr)
    {
        QMessageBox::information(nullptr, QString::fromLocal8Bit("提示"),
                                 QString::fromLocal8Bit("异常处理安装失败!"),
                                 QMessageBox::Yes);

        return -1;
    }
#endif



    QFile file(":/resource/stylecss.css");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());


    MainWindow wnd;
    wnd.show();

    return app.exec();
}

