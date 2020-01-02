#ifndef PROCESSES_H_INCLUDED
#define PROCESSES_H_INCLUDED

#include <string>

int chkProgram(std::string command);
bool runProgram(std::string command, std::string runpath, bool wait);
void killByHandle();
bool killProgram(std::string program);
/*
class Runner
{
private:
    FILE *pPipe;
    std::string sPath;
    bool killed=false;
    #ifdef _WIN32
    HANDLE hProc;
    #endif // _WIN32
public:
    void run(std::string FilePath)
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
    Runner(std::string FilePath)
    {
        run(FilePath);
    }
    ~Runner()
    {
        kill();
    }
    #ifndef _WIN32
    void write(std::string content)
    {
        fwrite(content.data(),content.size(),1,pPipe);
    }
    std::string read(int len)
    {
        std::string retdata;
        fread(&retdata,len,1,pPipe);
        return retdata;
    }
    #endif // _WIN32
};
*/
#endif // PROCESSES_H_INCLUDED
