#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "misc.h"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <spawn.h>
#endif // _WIN32

#include "processes.h"

//Runner runner;

#ifdef _WIN32
HANDLE hProc = 0;
HANDLE job = 0;
JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_limits = {};
#else
FILE *pPipe;
#endif // _WIN32

int chkProgram(string command)
{
    /*
    char psBuffer[128];
    FILE *pPipe;

    if((pPipe = _popen(command.data(),"rt")) == NULL) return -1;

    // Read pipe until end of file, or an error occurs.

    while(fgets(psBuffer, 128, pPipe));//return value has no use, just dump them

    // Close pipe and print return value of pPipe.
    if(feof(pPipe))
    {
        _pclose(pPipe);
        return 0;
    }
    else
        return -2;
    */
    return 0;
}


bool runProgram(string command, string runpath, bool wait)
{
#ifdef _WIN32
    BOOL retval = false;
    STARTUPINFO si = {};
    PROCESS_INFORMATION pi = {};
    char curdir[512] = {}, *cmdstr = {}, *pathstr = {};
    string path;
    job = CreateJobObject(NULL, NULL);
    job_limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if(strFind(runpath, ":")) //is an absolute path
    {
        path = runpath;
    }
    else //is a relative path
    {
        GetCurrentDirectory(512, curdir);
        path = string(curdir) + "\\";
        if(runpath != "")
            path += runpath + "\\";
    }
    cmdstr = const_cast<char*>(command.data());
    pathstr = const_cast<char*>(path.data());
    string prgname = path + command.substr(0, command.find(" "));
    string prgargs = command.substr(command.find(" ") + 1);
    retval = CreateProcess(NULL, cmdstr, NULL, NULL, false, CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB, NULL, pathstr, &si, &pi);

    AssignProcessToJobObject(job, pi.hProcess);
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &job_limits, sizeof(job_limits));

    hProc = pi.hProcess;
    if(wait)
    {
        WaitForSingleObject(hProc, INFINITE);
        CloseHandle(hProc);
    }
    return retval;
#else
    /*
    string total_path = runpath == "" ? "" : runpath + PATH_SLASH;
    total_path += command;
    pPipe = popen(total_path.data(), "r");
    */
    char curdir[1024] = {};
    getcwd(curdir, 1024);
    chdir(runpath.data());
    pPipe = popen(command.data(), "r");
    chdir(curdir);
    return true;
#endif // _WIN32
}

#ifdef _WIN32
void killByHandle()
{
    if(hProc != NULL)
    {
        TerminateProcess(hProc, 0);
        CloseHandle(hProc);
    }
}
#endif // _WIN32


/*
void runprogram(string command, string runpath, bool wait)
{
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si,sizeof(si));
    ZeroMemory(&pi,sizeof(pi));
    char curdir[512]= {},*cmdstr= {},*pathstr= {};
    command = "/c " + command;
    si.wShowWindow=true;
    cmdstr=const_cast<char*>(command.data());
    GetCurrentDirectory(512,curdir);
    runpath=string(curdir) + "\\"+runpath+"\\";
    pathstr=const_cast<char*>(runpath.data());
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = "open";
    ShExecInfo.lpFile = "cmd.exe";
    ShExecInfo.lpParameters = cmdstr;
    ShExecInfo.lpDirectory = pathstr;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    if(wait)
    {
        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
        CloseHandle(hProc);
    }
#else
    //wait for Linux codes
#endif // _WIN32
    return;
}
*/

bool killProgram(string program)
{
#ifdef _WIN32
    /*
    //using an effective method
    program = "taskkill /f /im " + program + " >nul 2>nul";
    system(program.data());
    return true;
    */
    /*
    // first get all the process so that we can get the process id
    DWORD processes[1024], count;
    if( !EnumProcesses( processes, sizeof(processes), &count ) )
    {
        return false;
    }

    count /= sizeof(DWORD);
    for(unsigned int i = 0; i < count; i++)
    {
        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
        if(processes[i] != 0)
        {
            // remember to open with PROCESS_ALL_ACCESS, otherwise you will not be able to kill it
            HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, processes[i] );
            if(NULL != hProcess)
            {
                HMODULE hMod;
                DWORD cbNeeded;
                if(EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                {
                    GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR));

                    // find the process and kill it
                    if(program.compare(szProcessName)==0)
                    {
                        DWORD result = WAIT_OBJECT_0;
                        while(result == WAIT_OBJECT_0)
                        {
                            // use WaitForSingleObject to make sure it's dead
                            result = WaitForSingleObject(hProcess, 100);
                            TerminateProcess(hProcess, 0);
                        }

                        CloseHandle(hProcess);
                    }
                }
            }
        }
    }
    return true;
    */
    HANDLE hShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0), hProcess = 0;
    if(hShot == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    PROCESSENTRY32 pe = { sizeof(pe) };

    for(BOOL bRet = Process32First(hShot, &pe); bRet; bRet = Process32Next(hShot, &pe))
    {
        if(program.compare(pe.szExeFile) == 0)
        {
            hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            if(hProcess == NULL)
            {
                CloseHandle(hShot);
                return false;
            }

            DWORD result = WAIT_OBJECT_0;
            while(result == WAIT_OBJECT_0)
            {
                result = WaitForSingleObject(hProcess, 50);
                TerminateProcess(hProcess, 0);
            }
            CloseHandle(hProcess);
        }
    }
    CloseHandle(hShot);
    return true;
#else
    //if(!feof(pPipe))
        //pclose(pPipe);
    program = "pkill -f " + program;
    system(program.data());
    return true;
#endif
}

