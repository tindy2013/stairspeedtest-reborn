#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <bits/stdc++.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32

#include "misc.h"
#include "printout.h"

#define LOG_TYPE_INFO 1
#define LOG_TYPE_ERROR 2
#define LOG_TYPE_RAW 3

extern string resultpath, logpath;

using namespace std;

int makedir(const char *path);
string getTime(int type);
void resultinit(bool export_with_maxspeed);
void loginit(bool rpcmode);
void writeresult(nodeInfo *node, bool export_with_maxspeed);
void writelog(int type, string content);
void resulteof(string traffic, int worknodes, int totnodes);
void logeof();
void exportresult(string outpath, string utiljspath, string stylepath, bool export_with_maxspeed);

#endif // LOGGER_H_INCLUDED
