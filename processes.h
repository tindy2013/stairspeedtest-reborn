#ifndef PROCESSES_H_INCLUDED
#define PROCESSES_H_INCLUDED

#include <string>

using namespace std;

int chkProgram(string command);
bool runProgram(string command, string runpath, bool wait);
void killByHandle();
bool killProgram(string program);
/*
class Runner
{
private:
    FILE *pPipe;
    string sPath;
    bool killed=false;
    #ifdef _WIN32
    HANDLE hProc;
    #endif // _WIN32
public:
    void run(string FilePath)
    {
        #ifdef _WIN32
        sPath=FilePath;
        RunProgram(FilePath,"",false,&hProc);
        #else
        pPipe=popen(FilePath.data(),"r");
        #endif
        killed=false;
    }
    void kill()
    {
        killed=true;
        #ifdef _WIN32
        KillByHandle(hProc);
        #else
        pclose(pPipe);
        #endif // _WIN32
    }
    Runner()
    {
        //build object only
    }
    Runner(string FilePath)
    {
        run(FilePath);
    }
    ~Runner()
    {
        kill();
    }
    #ifndef _WIN32
    void write(string content)
    {
        fwrite(content.data(),content.size(),1,pPipe);
    }
    string read(int len)
    {
        string retdata;
        fread(&retdata,len,1,pPipe);
        return retdata;
    }
    #endif // _WIN32
};
*/
#endif // PROCESSES_H_INCLUDED
