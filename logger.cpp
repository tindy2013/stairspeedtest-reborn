#include "logger.h"
#include "version.h"

using namespace std;

string curtime, result_content;
string resultpath, logpath;

int makedir(const char *path)
{
#ifdef _WIN32
    return mkdir(path);
#else
    return mkdir(path,0755);
#endif // _WIN32
}

string getTime(int type)
{
    time_t lt;
    char tmpbuf[32];
    string format;
    lt = time(NULL);
    struct tm *local = localtime(&lt);
    switch(type)
    {
    case 1:
        format = "%Y%m%d-%H%M%S";
        break;
    case 2:
        format = "%Y/%m/%d %a %H:%M:%S";
        break;
    case 3:
        format = "%Y-%m-%d %H:%M:%S";
        break;
    }
    strftime(tmpbuf,32,format.data(),local);
    return string(tmpbuf);
}

void loginit(bool rpcmode)
{
    curtime = getTime(1);
    logpath = "logs" PATH_SLASH + curtime + ".log";
    string log_header = "Stair Speedtest started in ";
    if(rpcmode)
        log_header += "GUI mode.";
    else
        log_header += "CLI mode.";
    makedir("logs");
    writelog(LOG_TYPE_INFO,log_header);
}

void resultinit(bool export_with_maxspeed)
{
    resultpath = "results" PATH_SLASH + curtime + ".log";
    makedir("results");
    result_content = "group,remarks,loss,ping,avgspeed";
    if(export_with_maxspeed)
        result_content += ",maxspeed";
    result_content += "\n";
    //result_content<<endl;
    write2file(resultpath,result_content,true);
}

void writelog(int type,string content)
{
    string timestr = "[" + getTime(2) + "]",typestr;
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
    }
    content = timestr + typestr + content;
    write2file(logpath,content,false);
}

void writeresult(nodeInfo *node,bool export_with_maxspeed)
{
    string content = node->group + "," + node->remarks + "," + node->pkloss + "," + node->avgping + "," + node->avgspeed;
    if(export_with_maxspeed)
        content += "," + node->maxspeed;
    result_content += content + "\n";
    //write2file(resultpath,result_content.str(),true);
    write2file(resultpath,content,false);
}

void logeof()
{
    writelog(LOG_TYPE_INFO,"Program terminated.");
    write2file(logpath,"--EOF--",false);
}

void resulteof(string traffic,int worknodes,int totnodes)
{
    result_content += "Traffic used : " + traffic + ". Working Node(s) : [" + to_string(worknodes) + "/" + to_string(totnodes) + "]\n";
    result_content += "Generated at " + getTime(3) + "\n";
    result_content += "By Stair Speedtest " VERSION ".\n";
    write2file(resultpath,result_content,true);
}

void exportresult(string outpath,string utiljspath,string stylepath,bool export_with_maxspeed)
{
    if(utiljspath=="")
        return;
    string strInput;
    vector<string> params;
    ifstream inputjs,inputstyle;
    ofstream outfile;
    stringstream result_content_stream;
    result_content_stream<<result_content;
    inputjs.open(utiljspath,ios::in);
    inputstyle.open(stylepath,ios::in);
    outfile.open(outpath,ios::out);
    outfile<<"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /><style type=\"text/css\">"<<endl;
    while(getline(inputstyle,strInput))
    {
        outfile<<strInput<<endl;
    }
    inputstyle.close();
    outfile<<"</style><script language=\"javascript\">"<<endl;
    while(getline(inputjs,strInput))
    {
        outfile<<strInput<<endl;
    }
    inputjs.close();
    outfile<<"</script></head><body onload=\"loadevent()\"><table id=\"table\" rules=\"all\">";
    while(getline(result_content_stream,strInput))
    {
        if(strInput=="")
            continue;
        if(strfind(strInput,"avgspeed"))
            continue;
        if(strfind(strInput,"%,"))
        {
            params = split(strInput,",");
            outfile<<"<tr><td>"<<params[0]<<"</td><td>"<<params[1]<<"</td><td>"<<params[2]<<"</td><td>"<<params[3]<<"</td><td class=\"speed\">"<<params[4]<<"</td>";
            if(export_with_maxspeed)
                outfile<<"<td class=\"speed\">"<<params[5]<<"</td>";
            outfile<<"</tr>";
        }
        if(strfind(strInput,"Traffic used :"))
            outfile<<"<tr id=\"traffic\"><td>"<<strInput<<"</td></tr>";
        if(strfind(strInput,"Generated at"))
            outfile<<"<tr id=\"gentime\"><td>"<<strInput<<"</td></tr>";
        if(strfind(strInput,"By "))
            outfile<<"<tr id=\"about\"><td>"<<strInput<<"</td></tr>";
    }
    outfile<<"</table></body></html>";
    outfile.close();
}
