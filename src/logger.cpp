#include <string>
#include <fstream>
#include <sstream>
#include <mutex>

#include "logger.h"
#include "version.h"
#include "misc.h"
#include "printout.h"

#include <sys/time.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32

typedef std::lock_guard<std::mutex> guarded_mutex;
std::mutex logger_mutex;

std::string curtime, result_content;
std::string resultPath, logPath;

int makeDir(const char *path)
{
#ifdef _WIN32
    return mkdir(path);
#else
    return mkdir(path, 0755);
#endif // _WIN32
}

std::string getTime(int type)
{
    time_t lt;
    char tmpbuf[32], cMillis[7];
    std::string format;
    timeval tv;
    gettimeofday(&tv, NULL);
    snprintf(cMillis, 7, "%.6ld", (long)tv.tv_usec);
    lt = time(NULL);
    struct tm *local = localtime(&lt);
    switch(type)
    {
    case 1:
        format = "%Y%m%d-%H%M%S";
        break;
    case 2:
        format = "%Y/%m/%d %a %H:%M:%S." + std::string(cMillis);
        break;
    case 3:
        format = "%Y-%m-%d %H:%M:%S";
        break;
    }
    strftime(tmpbuf, 32, format.data(), local);
    return std::string(tmpbuf);
}

void logInit(bool rpcmode)
{
    curtime = getTime(1);
    logPath = "logs" PATH_SLASH + curtime + ".log";
    std::string log_header = "Stair Speedtest " VERSION " started in ";
    if(rpcmode)
        log_header += "GUI mode.";
    else
        log_header += "CLI mode.";
    writeLog(LOG_TYPE_INFO, log_header);
}

void resultInit()
{
    curtime = getTime(1);
    resultPath = "results" PATH_SLASH + curtime + ".log";
}

void writeLog(int type, std::string content, int level)
{
    guarded_mutex guard(logger_mutex);
    std::string timestr = "[" + getTime(2) + "]", typestr = "[UNKNOWN]";
    switch(type)
    {
    case LOG_TYPE_ERROR:
        typestr = "[ERROR]";
        break;
    case LOG_TYPE_INFO:
        typestr = "[INFO]";
        break;
    case LOG_TYPE_RAW:
        typestr = "[RAW]";
        break;
    case LOG_TYPE_WARN:
        typestr = "[WARNING]";
        break;
    case LOG_TYPE_GEOIP:
        typestr = "[GEOIP]";
        break;
    case LOG_TYPE_TCPING:
        typestr = "[TCPING]";
        break;
    case LOG_TYPE_FILEDL:
        typestr = "[FILEDL]";
        break;
    case LOG_TYPE_FILEUL:
        typestr = "[FILEUL]";
        break;
    case LOG_TYPE_RULES:
        typestr = "[RULES]";
        break;
    case LOG_TYPE_GPING:
        typestr = "[GPING]";
        break;
    case LOG_TYPE_RENDER:
        typestr = "[RENDER]";
        break;
    case LOG_TYPE_STUN:
        typestr = "[STUN]";
    }
    content = timestr + typestr + content + "\n";
    fileWrite(logPath, content, false);
}

void logEOF()
{
    writeLog(LOG_TYPE_INFO,"Program terminated.");
    fileWrite(logPath, "--EOF--", false);
}

/*

void resultInit(bool export_with_maxspeed)
{
    curtime = getTime(1);
    resultPath = "results" PATH_SLASH + curtime + ".log";
    result_content = "group,remarks,loss,ping,avgspeed";
    if(export_with_maxspeed)
        result_content += ",maxspeed";
    result_content += "\n";
    fileWrite(resultPath, result_content, true);
}

void writeResult(nodeInfo *node, bool export_with_maxspeed)
{
    std::string content = node->group + "," + node->remarks + "," + node->pkLoss + "," + node->avgPing + "," + node->avgSpeed;
    if(export_with_maxspeed)
        content += "," + node->maxSpeed;
    result_content += content + "\n";
    //write2file(resultPath,result_content.str(),true);
    writeToFile(resultPath, content, false);
}

void resultEOF(std::string traffic, int worknodes, int totnodes)
{
    result_content += "Traffic used : " + traffic + ". Working Node(s) : [" + std::to_string(worknodes) + "/" + std::to_string(totnodes) + "]\n";
    result_content += "Generated at " + getTime(3) + "\n";
    result_content += "By Stair Speedtest " VERSION ".\n";
    writeToFile(resultPath,result_content,true);
}

void exportResult(std::string outpath, std::string utiljspath, std::string stylepath, bool export_with_maxspeed)
{
    if(utiljspath.empty())
        return;
    std::string strInput;
    vector<std::string> params;
    ifstream inputjs, inputstyle;
    ofstream outfile;
    stringstream result_content_stream;
    result_content_stream<<result_content;
    inputjs.open(utiljspath, ios::in);
    inputstyle.open(stylepath, ios::in);
    outfile.open(outpath, ios::out);
    outfile<<"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /><style type=\"text/css\">"<<endl;
    while(getline(inputstyle, strInput))
    {
        outfile<<strInput<<endl;
    }
    inputstyle.close();
    outfile<<"</style><script language=\"javascript\">"<<endl;
    while(getline(inputjs, strInput))
    {
        outfile<<strInput<<endl;
    }
    inputjs.close();
    outfile<<"</script></head><body onload=\"loadevent()\"><table id=\"table\" rules=\"all\">";
    while(getline(result_content_stream, strInput))
    {
        if(strInput.empty())
            continue;
        if(strFind(strInput, "avgspeed"))
            continue;
        if(strFind(strInput, "%,"))
        {
            params = split(strInput, ",");
            outfile<<"<tr><td>"<<params[0]<<"</td><td>"<<params[1]<<"</td><td>"<<params[2]<<"</td><td>"<<params[3]<<"</td><td class=\"speed\">"<<params[4]<<"</td>";
            if(export_with_maxspeed)
                outfile<<"<td class=\"speed\">"<<params[5]<<"</td>";
            outfile<<"</tr>";
        }
        if(strFind(strInput, "Traffic used :"))
            outfile<<"<tr id=\"traffic\"><td>"<<strInput<<"</td></tr>";
        if(strFind(strInput, "Generated at"))
            outfile<<"<tr id=\"gentime\"><td>"<<strInput<<"</td></tr>";
        if(strFind(strInput, "By "))
            outfile<<"<tr id=\"about\"><td>"<<strInput<<"</td></tr>";
    }
    outfile<<"</table></body></html>";
    outfile.close();
}
*/
