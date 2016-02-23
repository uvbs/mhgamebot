#include "scriptapp.h"
#include <Shlwapi.h>>

void set_workdir()
{
    char path[MAX_PATH];
    ::GetModuleFileNameA(NULL, path, MAX_PATH);
    ::PathRemoveFileSpecA(path);
    ::SetCurrentDirectoryA(path);
}

int main(int argc, char *argv[])
{
    set_workdir();

    ScriptApp app;
    app.run();

    getchar();
    return 0;
}

