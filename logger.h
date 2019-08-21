#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include "misc.h"
#include "printout.h"

#include <bits/stdc++.h>
#include <sys/time.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32

#define LOG_TYPE_INFO 1
#define LOG_TYPE_ERROR 2
#define LOG_TYPE_RAW 3

extern string resultPath, logPath;

using namespace std;

int makeDir(const char *path);
string getTime(int type);
void resultInit(bool export_with_maxspeed);
void logInit(bool rpcmode);
void writeResult(nodeInfo *node, bool export_with_maxspeed);
void writeLog(int type, string content);
void resultEOF(string traffic, int worknodes, int totnodes);
void logEOF();
void exportResult(string outpath, string utiljspath, string stylepath, bool export_with_maxspeed);

#endif // LOGGER_H_INCLUDED
