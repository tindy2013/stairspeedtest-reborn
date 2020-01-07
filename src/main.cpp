#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <iostream>
#include <regex>
#include <fstream>
#include <iomanip>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif // _WIN32

#include "socket.h"
#include "misc.h"
#include "speedtestutil.h"
#include "printout.h"
#include "webget.h"
#include "logger.h"
#include "renderer.h"
#include "processes.h"
#include "rulematch.h"
#include "version.h"
#include "ini_reader.h"
#include "multithread_test.h"
#include "nodeinfo.h"

using namespace std::chrono;
using namespace std::__cxx11;

#define MAX_FILE_SIZE 100 * 1024 * 1024

//for use globally
bool rpcmode = false;
bool multilink = false;
int socksport = 65432;
std::string socksaddr = "127.0.0.1";
std::string custom_group;
std::string pngpath;

//for use of web server
bool webserver_mode = false;
std::string listen_address = "127.0.0.1";
int listen_port = 10870, cur_node_id = -1;

bool ss_libev = true;
bool ssr_libev = true;
std::string def_test_file = "https://download.microsoft.com/download/2/0/E/20E90413-712F-438C-988E-FDAA79A8AC3D/dotnetfx35.exe";
std::string def_upload_target = "http://losangeles.speed.googlefiber.net:3004/upload?time=0";
std::vector<downloadLink> downloadFiles;
std::vector<linkMatchRule> matchRules;
string_array custom_exclude_remarks, custom_include_remarks, dict, trans;
std::vector<nodeInfo> allNodes;
std::vector<color> custom_color_groups;
std::vector<int> custom_color_bounds;
std::string speedtest_mode = "all";
std::string override_conf_port = "";
std::string export_color_style = "rainbow";
int def_thread_count = 4;
bool export_with_maxspeed = false;
bool export_as_new_style = true;
bool test_site_ping = true;
bool test_upload = false;
bool multilink_export_as_one_image = false;
bool single_test_force_export = false;
bool verbose = false;
std::string export_sort_method = "none";

int avail_status[4] = {0, 0, 0, 0};
unsigned int node_count = 0;
int curGroupID = 0;

//declarations

int tcping(nodeInfo *node);
void getTestFile(nodeInfo *node, std::string proxy, std::vector<downloadLink> *downloadFiles, std::vector<linkMatchRule> *matchRules, std::string defaultTestFile);
void ssrspeed_webserver_routine(std::string listen_address, int listen_port);

//original codes

#ifndef _WIN32

int _getch()
{
    int ch;
    struct termios tm, tm_old;
    int fd = 0;

    if (tcgetattr(fd, &tm) < 0)
    {
        return -1;
    }

    tm_old = tm;
    cfmakeraw(&tm);
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
    {
        return -1;
    }

    ch = std::cin.get();
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
    {
        return -1;
    }
    return ch;
}


void SetConsoleTitle(std::string title)
{
    system(std::string("echo \"\\033]0;" + title + "\\007\\c\"").data());
}

#endif // _WIN32

void clearTrans()
{
    eraseElements(dict);
    eraseElements(trans);
}

void addTrans(std::string dictval, std::string transval)
{
    dict.push_back(dictval);
    trans.push_back(transval);
}

void copyNodes(std::vector<nodeInfo> *source, std::vector<nodeInfo> *dest)
{
    for(auto &x : *source)
    {
        dest->push_back(x);
    }
}

void copyNodesWithGroupID(std::vector<nodeInfo> *source, std::vector<nodeInfo> *dest, int groupID)
{
    for(auto &x : *source)
    {
        if(x.groupID == groupID)
            dest->push_back(x);
    }
}

void clientCheck()
{
#ifdef _WIN32
    std::string v2core_path = "tools\\clients\\v2ray-core\\v2-core.exe";
    std::string ssr_libev_path = "tools\\clients\\shadowsocksr-libev\\ssr-libev.exe";
    std::string ss_libev_path = "tools\\clients\\shadowsocks-libev\\ss-libev.exe";
#else
    std::string v2core_path = "tools/clients/v2ray";
    std::string ssr_libev_path = "tools/clients/ssr-local";
    std::string ss_libev_path = "tools/clients/ss-local";
#endif // _WIN32

    if(fileExist(v2core_path))
    {
        avail_status[SPEEDTEST_MESSAGE_FOUNDVMESS] = 1;
        writeLog(LOG_TYPE_INFO, "Found V2Ray core at path " + v2core_path);
    }
    else
    {
        avail_status[SPEEDTEST_MESSAGE_FOUNDVMESS] = 0;
        writeLog(LOG_TYPE_WARN, "V2Ray core not found at path " + v2core_path);
    }
    if(fileExist(ss_libev_path))
    {
        avail_status[SPEEDTEST_MESSAGE_FOUNDSS] = 1;
        writeLog(LOG_TYPE_INFO, "Found Shadowsocks-libev at path " + ss_libev_path);
    }
    else
    {
        avail_status[SPEEDTEST_MESSAGE_FOUNDSS] = 0;
        writeLog(LOG_TYPE_WARN, "Shadowsocks-libev not found at path " + ss_libev_path);
    }
    if(fileExist(ssr_libev_path))
    {
        avail_status[SPEEDTEST_MESSAGE_FOUNDSSR] = 1;
        writeLog(LOG_TYPE_INFO, "Found ShadowsocksR-libev at path " + ssr_libev_path);
    }
    else
    {
        avail_status[SPEEDTEST_MESSAGE_FOUNDSSR] = 0;
        writeLog(LOG_TYPE_WARN, "ShadowsocksR-libev not found at path " + ssr_libev_path);
    }
}

int runClient(int client, std::string runpath)
{
#ifdef _WIN32
    std::string v2core_path = "tools\\clients\\v2ray-core\\v2-core.exe -config config.json";
    std::string ssr_libev_path = "tools\\clients\\shadowsocksr-libev\\ssr-libev.exe -c config.json";

    std::string ss_libev_dir = "tools\\clients\\shadowsocks-libev\\";
    std::string ss_libev_path = ss_libev_dir + "ss-libev.exe -c ..\\..\\..\\config.json";

    std::string ssr_win_dir = "tools\\clients\\shadowsocksr-win\\";
    std::string ssr_win_path = ssr_win_dir + "shadowsocksr-win.exe";
    std::string ss_win_dir = "tools\\clients\\shadowsocks-win\\";
    std::string ss_win_path = ss_win_dir + "shadowsocks-win.exe";

    switch(client)
    {
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        writeLog(LOG_TYPE_INFO, "Starting up v2ray core...");
        runProgram(v2core_path, "", false);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        if(ssr_libev)
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocksr-libev...");
            runProgram(ssr_libev_path, "", false);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocksr-win...");
            fileCopy("config.json", ssr_win_dir + "gui-config.json");
            runProgram(ssr_win_path, "", false);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        if(ss_libev)
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocks-libev...");
            runProgram(ss_libev_path, ss_libev_dir, false);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocks-win...");
            fileCopy("config.json", ss_win_dir + "gui-config.json");
            runProgram(ss_win_path, ss_win_dir, false);
        }
        break;
    }
#else
    std::string v2core_path = "tools/clients/v2ray -config config.json";
    std::string ssr_libev_path = "tools/clients/ssr-local -c config.json";

    std::string ss_libev_dir = "tools/clients/";
    std::string ss_libev_path = "./ss-local -c ../../config.json";

    switch(client)
    {
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        writeLog(LOG_TYPE_INFO, "Starting up v2ray core...");
        runProgram(v2core_path, "", false);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        writeLog(LOG_TYPE_INFO, "Starting up shadowsocksr-libev...");
        runProgram(ssr_libev_path, "", false);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        writeLog(LOG_TYPE_INFO, "Starting up shadowsocks-libev...");
        runProgram(ss_libev_path, ss_libev_dir, false);
        break;
    }
#endif // _WIN32
    return 0;
}

int killClient(int client)
{
    //TerminateProcess(hProc, 0);
#ifdef _WIN32
    killByHandle();
    /*
    std::string v2core_name = "v2-core.exe";
    std::string ss_libev_name = "ss-libev.exe";
    std::string ssr_libev_name = "ssr-libev.exe";
    std::string ss_win_name = "shadowsocks-win.exe";
    std::string ssr_win_name = "shadowsocksr-win.exe";

    switch(client)
    {
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        writeLog(LOG_TYPE_INFO, "Killing v2ray core...");
        killProgram(v2core_name);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        if(ssr_libev)
        {
            writeLog(LOG_TYPE_INFO, "Killing shadowsocksr-libev...");
            killProgram(ssr_libev_name);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Killing shadowsocksr-win...");
            killProgram(ssr_win_name);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        if(ss_libev)
        {
            writeLog(LOG_TYPE_INFO, "Killing shadowsocks-libev...");
            killProgram(ss_libev_name);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Killing shadowsocks-win...");
            killProgram(ss_win_name);
        }
        break;
    }
    */
#else
    std::string v2core_name = "v2ray";
    std::string ss_libev_name = "ss-local";
    std::string ssr_libev_name = "ssr-local";

    switch(client)
    {
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        writeLog(LOG_TYPE_INFO, "Killing v2ray core...");
        killProgram(v2core_name);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        writeLog(LOG_TYPE_INFO, "Killing shadowsocksr-libev...");
        killProgram(ssr_libev_name);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        writeLog(LOG_TYPE_INFO, "Killing shadowsocks-libev...");
        killProgram(ss_libev_name);
        break;
    }
#endif
    return 0;
}

void readConf(std::string path)
{
    downloadLink link;
    linkMatchRule rule;
    color tmpColor;
    unsigned int i;
    string_array vChild, vArray;
    INIReader ini;
    std::string strTemp;

    ini.do_utf8_to_gbk = true;
    ini.ParseFile("pref.ini");

    ini.EnterSection("common");
    if(ini.ItemPrefixExist("exclude_remark"))
        ini.GetAll("exclude_remark", custom_exclude_remarks);
    if(ini.ItemPrefixExist("include_remark"))
        ini.GetAll("include_remark", custom_include_remarks);

    ini.EnterSection("advanced");
    if(ini.ItemExist("speedtest_mode"))
        speedtest_mode = ini.Get("speedtest_mode");
    if(ini.ItemExist("test_site_ping"))
        test_site_ping = ini.GetBool("test_site_ping");
    if(ini.ItemExist("test_upload"))
        test_upload = ini.GetBool("test_upload");
#ifdef _WIN32
    if(ini.ItemExist("preferred_ss_client"))
    {
        strTemp = ini.Get("preferred_ss_client");
        if(strTemp == "ss-csharp")
            ss_libev = false;
    }
    if(ini.ItemExist("preferred_ssr_client"))
    {
        strTemp = ini.Get("preferred_ssr_client");
        if(strTemp == "ssr-csharp")
            ssr_libev = false;
    }
#endif // _WIN32
    if(ini.ItemExist("override_conf_port"))
        override_conf_port = ini.Get("override_conf_port");
    if(ini.ItemExist("thread_count"))
        def_thread_count = stoi(ini.Get("thread_count"));

    ini.EnterSection("export");
    if(ini.ItemExist("export_with_maxspeed"))
        export_with_maxspeed = ini.GetBool("export_with_maxspeed");
    if(ini.ItemExist("export_sort_method"))
        export_sort_method = ini.Get("export_sort_method");
    if(ini.ItemExist("multilink_export_as_one_image"))
        multilink_export_as_one_image = ini.GetBool("multilink_export_as_one_image");
    if(ini.ItemExist("single_test_force_export"))
        single_test_force_export = ini.GetBool("single_test_force_export");
    if(ini.ItemExist("export_as_new_style"))
        export_as_new_style = ini.GetBool("export_as_new_style");
    if(ini.ItemExist("export_color_style"))
        export_color_style = ini.Get("export_color_style");
    if(ini.ItemExist("custom_color_groups"))
    {
        vChild = split(ini.Get("custom_color_groups"), "|");
        if(vChild.size() >= 2)
        {
            for(i = 0; i < vChild.size() - 1; i++)
            {
                vArray = split(vChild[i], ",");
                if(vArray.size() == 3)
                {
                    tmpColor.red = stoi(trim(vArray[0]));
                    tmpColor.green = stoi(trim(vArray[1]));
                    tmpColor.blue = stoi(trim(vArray[2]));
                    custom_color_groups.push_back(tmpColor);
                }
            }
        }
    }
    if(ini.ItemExist("custom_color_bounds"))
    {
        vChild = split(ini.Get("custom_color_bounds"), "|");
        if(vChild.size() >= 2)
        {
            for(i = 0; i < vChild.size() - 1; i++)
            {
                custom_color_bounds.push_back(stoi(vChild[i]));
            }
        }
    }
    if(ini.ItemExist("export_as_ssrspeed"))
        export_as_ssrspeed = ini.GetBool("export_as_ssrspeed");

    ini.EnterSection("rules");
    if(ini.ItemPrefixExist("test_file_urls"))
    {
        eraseElements(vArray);
        ini.GetAll("test_file_urls", vArray);
        for(auto &x : vArray)
        {
            vChild = split(x, "|");
            if(vChild.size() == 2)
            {
                link.url = vChild[0];
                link.tag = vChild[1];
                downloadFiles.push_back(link);
            }
        }
    }
    if(ini.ItemPrefixExist("rules"))
    {
        eraseElements(vArray);
        ini.GetAll("rules", vArray);
        for(auto &x : vArray)
        {
            vChild = split(x, "|");
            if(vChild.size() >= 3)
            {
                eraseElements(rule.rules);
                rule.mode = vChild[0];
                for(i = 1; i < vChild.size() - 1; i++)
                {
                    rule.rules.push_back(vChild[i]);
                }
                rule.tag = vChild[vChild.size() - 1];
                matchRules.push_back(rule);
            }
        }
    }
    if(export_color_style == "custom")
    {
        colorgroup.swap(custom_color_groups);
        bounds.swap(custom_color_bounds);
    }

    ini.EnterSection("webserver");
    if(ini.ItemExist("webserver_mode"))
        webserver_mode = ini.GetBool("webserver_mode");
    if(ini.ItemExist("listen_address"))
        listen_address = ini.Get("listen_address");
    if(ini.ItemExist("listen_port"))
        listen_port = ini.GetInt("listen_port");
}

void signalHandler(int signum)
{
    std::cerr << "Interrupt signal (" << signum << ") received.\n";

    /*
    killClient(SPEEDTEST_MESSAGE_FOUNDSS);
    killClient(SPEEDTEST_MESSAGE_FOUNDSSR);
    killClient(SPEEDTEST_MESSAGE_FOUNDVMESS);
    */
    killByHandle();
    writeLog(LOG_TYPE_INFO, "Received SIGINT. Exit right now.");
    logEOF();

    exit(signum);
}

void chkArg(int argc, char* argv[])
{
    for(int i = 0; i < argc; i++)
    {
        if(!strcmp(argv[i], "/rpc"))
            rpcmode = true;
        if(!strcmp(argv[i], "/web"))
            webserver_mode = true;
    }
}

/*
void exportHTML()
{
    std::string htmpath = replace_all_distinct(resultPath, ".log", ".htm");
    //std::string pngname = replace_all_distinct(replace_all_distinct(resultpath, ".log", ".png"), "results\\", "");
    //std::string resultname = replace_all_distinct(resultpath, "results\\", "");
    //std::string htmname = replace_all_distinct(htmpath, "results\\", "");
    //std::string rendercmd = "..\\tools\\misc\\phantomjs.exe ..\\tools\\misc\\render_alt.js " + htmname + " " + pngname + " " + export_sort_method;
    exportResult(htmpath, "tools\\misc\\util.js", "tools\\misc\\style.css", export_with_maxspeed);
    //runprogram(rendercmd, "results", true);
}
*/

void saveResult(std::vector<nodeInfo> *nodes)
{
    INIReader ini;
    std::string data;

    ini.SetCurrentSection("Basic");
    ini.Set("Tester", "Stair Speedtest Reborn " VERSION);
    ini.Set("GenerationTime", getTime(3));

    for(nodeInfo &x : *nodes)
    {
        ini.SetCurrentSection(x.group + "^" + x.remarks);
        ini.Set("AvgPing", x.avgPing);
        ini.Set("PkLoss", x.pkLoss);
        ini.Set("SitePing", x.sitePing);
        ini.Set("AvgSpeed", x.avgSpeed);
        ini.Set("MaxSpeed", x.maxSpeed);
        ini.Set("ULSpeed", x.ulSpeed);
        ini.SetLong("UsedTraffic", x.totalRecvBytes);
        ini.SetLong("GroupID", x.groupID);
        ini.SetLong("ID", x.id);
        ini.SetBool("Online", x.online);
        ini.SetArray("RawPing", ",", x.rawPing);
        ini.SetArray("RawSitePing", ",", x.rawSitePing);
        ini.SetArray("RawSpeed", ",", x.rawSpeed);
    }

    ini.ToFile(resultPath);
}

int singleTest(nodeInfo *node)
{
    int retVal = 0;
    std::string logdata = "", testserver, username, password, proxy;
    int testport;
    node->ulTarget = def_upload_target; //for now only use default
    cur_node_id = node->id;

    auto start = steady_clock::now();
    if(node->proxyStr == "LOG") //import from result
    {
        if(!rpcmode)
        {
            clearTrans();
            addTrans("?group?", node->group);
            addTrans("?remarks?", node->remarks);
            addTrans("?index?", to_string(node->id + 1));
            addTrans("?total?", to_string(node_count));
            printMsgWithDict(SPEEDTEST_MESSAGE_GOTSERVER, rpcmode, dict, trans);
        }
        writeLog(LOG_TYPE_INFO, "Received server. Group: " + node->group + " Name: " + node->remarks);
        printMsg(SPEEDTEST_MESSAGE_GOTPING, node, rpcmode);
        printMsg(SPEEDTEST_MESSAGE_GOTGPING, node, rpcmode);
        printMsg(SPEEDTEST_MESSAGE_GOTSPEED, node, rpcmode);
        printMsg(SPEEDTEST_MESSAGE_GOTUPD, node, rpcmode);
        writeLog(LOG_TYPE_INFO, "Average speed: " + node->avgSpeed + "  Max speed: " + node->maxSpeed + "  Upload speed: " + node->ulSpeed + "  Traffic used in bytes: " + to_string(node->totalRecvBytes));
        printMsg(SPEEDTEST_MESSAGE_GOTRESULT, node, rpcmode);
        return SPEEDTEST_ERROR_NONE;
    }

    if(node->linkType == SPEEDTEST_MESSAGE_FOUNDSOCKS)
    {
        testserver = node->server;
        testport = node->port;
        username = getUrlArg(node->proxyStr, "user");
        password = getUrlArg(node->proxyStr, "pass");
    }
    else
    {
        testserver = socksaddr;
        testport = socksport;
        writeLog(LOG_TYPE_INFO, "Writing config file...");
        fileWrite("config.json", node->proxyStr, true);
        if(node->linkType != -1 && avail_status[node->linkType] == 1)
            runClient(node->linkType, "");
    }
    proxy = buildSocks5ProxyString(testserver, testport, username, password);

    //printMsg(SPEEDTEST_MESSAGE_GOTSERVER, node, rpcmode);
    if(!rpcmode)
    {
        clearTrans();
        addTrans("?group?", node->group);
        addTrans("?remarks?", node->remarks);
        addTrans("?index?", to_string(node->id + 1));
        addTrans("?total?", to_string(node_count));
        printMsgWithDict(SPEEDTEST_MESSAGE_GOTSERVER, rpcmode, dict, trans);
    }
    writeLog(LOG_TYPE_INFO, "Received server. Group: " + node->group + " Name: " + node->remarks);
    printMsg(SPEEDTEST_MESSAGE_STARTPING, node, rpcmode);
    if(speedtest_mode != "speedonly")
    {
        writeLog(LOG_TYPE_INFO, "Now performing TCP ping...");
        retVal = tcping(node);
        if(retVal == SPEEDTEST_ERROR_NORESOLVE)
        {
            writeLog(LOG_TYPE_ERROR, "Node address resolve error.");
            printMsg(SPEEDTEST_ERROR_NORESOLVE, node, rpcmode);
            auto end = steady_clock::now();
            auto duration = duration_cast<seconds>(end - start);
            node->duration = duration.count();
            return SPEEDTEST_ERROR_NORESOLVE;
        }
        if(node->pkLoss == "100.00%")
        {
            writeLog(LOG_TYPE_ERROR, "Cannot connect to this node.");
            printMsg(SPEEDTEST_ERROR_NOCONNECTION, node, rpcmode);
            killClient(node->linkType);
            auto end = steady_clock::now();
            auto duration = duration_cast<seconds>(end - start);
            node->duration = duration.count();
            return SPEEDTEST_ERROR_NOCONNECTION;
        }
        for(auto &x : node->rawPing)
        {
            logdata += to_string(x) + " ";
        }
        writeLog(LOG_TYPE_RAW, logdata);
        writeLog(LOG_TYPE_INFO, "TCP Ping: " + node->avgPing + "  Packet Loss: " + node->pkLoss);
    }
    else
        node->pkLoss = "0.00%";
    printMsg(SPEEDTEST_MESSAGE_GOTPING, node, rpcmode);

    writeLog(LOG_TYPE_INFO, "Now performing GeoIP parse...");
    printMsg(SPEEDTEST_MESSAGE_STARTGEOIP, node, rpcmode);
    getTestFile(node, proxy, &downloadFiles, &matchRules, def_test_file);
    if(node->outboundGeoIP.organization != "")
    {
        clearTrans();
        addTrans("?id?", to_string(node->id));
        addTrans("?isp?", node->outboundGeoIP.organization);
        addTrans("?location?", node->outboundGeoIP.country_code);
        writeLog(LOG_TYPE_INFO, "Got outbound ISP: " + node->outboundGeoIP.organization + "  Country code: " + node->outboundGeoIP.country_code);
        printMsgWithDict(SPEEDTEST_MESSAGE_GOTGEOIP, rpcmode, dict, trans);
    }
    else
    {
        printMsg(SPEEDTEST_ERROR_GEOIPERR, node, rpcmode);
    }

    if(test_site_ping)
    {
        printMsg(SPEEDTEST_MESSAGE_STARTGPING, node, rpcmode);
        writeLog(LOG_TYPE_INFO, "Now performing site ping...");
        //websitePing(node, "https://www.google.com/", testserver, testport, username, password);
        sitePing(node, testserver, testport, username, password, "https://www.google.com/");
        logdata = "";
        for(auto &x : node->rawSitePing)
        {
            logdata += to_string(x) + " ";
        }
        writeLog(LOG_TYPE_RAW, logdata);
        writeLog(LOG_TYPE_INFO, "Site ping: " + node->sitePing);
        printMsg(SPEEDTEST_MESSAGE_GOTGPING, node, rpcmode);
    }

    printMsg(SPEEDTEST_MESSAGE_STARTSPEED, node, rpcmode);
    //node->total_recv_bytes = 1;
    if(speedtest_mode != "pingonly")
    {
        writeLog(LOG_TYPE_INFO, "Now performing file download speed test...");
        perform_test(node, testserver, testport, username, password, def_thread_count);
        logdata = "";
        for(auto &x : node->rawSpeed)
        {
            logdata += to_string(x) + " ";
        }
        writeLog(LOG_TYPE_RAW, logdata);
        if(node->totalRecvBytes == 0)
        {
            writeLog(LOG_TYPE_ERROR, "Speedtest returned no speed.");
            printMsg(SPEEDTEST_ERROR_RETEST, node, rpcmode);
            perform_test(node, testserver, testport, username, password, def_thread_count);
            logdata = "";
            for(auto &x : node->rawSpeed)
            {
                logdata += to_string(x) + " ";
            }
            writeLog(LOG_TYPE_RAW, logdata);
            if(node->totalRecvBytes == 0)
            {
                writeLog(LOG_TYPE_ERROR, "Speedtest returned no speed 2 times.");
                printMsg(SPEEDTEST_ERROR_NOSPEED, node, rpcmode);
                printMsg(SPEEDTEST_MESSAGE_GOTSPEED, node, rpcmode);
                killClient(node->linkType);
                auto end = steady_clock::now();
                auto duration = duration_cast<seconds>(end - start);
                node->duration = duration.count();
                return SPEEDTEST_ERROR_NOSPEED;
            }
        }
    }
    printMsg(SPEEDTEST_MESSAGE_GOTSPEED, node, rpcmode);
    if(test_upload)
    {
        writeLog(LOG_TYPE_INFO, "Now performing upload speed test...");
        printMsg(SPEEDTEST_MESSAGE_STARTUPD, node, rpcmode);
        upload_test(node, testserver, testport, username, password);
        printMsg(SPEEDTEST_MESSAGE_GOTUPD, node, rpcmode);
    }
    writeLog(LOG_TYPE_INFO, "Average speed: " + node->avgSpeed + "  Max speed: " + node->maxSpeed + "  Upload speed: " + node->ulSpeed + "  Traffic used in bytes: " + to_string(node->totalRecvBytes));
    printMsg(SPEEDTEST_MESSAGE_GOTRESULT, node, rpcmode);
    node->online = true;
    killClient(node->linkType);
    auto end = steady_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    node->duration = duration.count();
    sleep(300);
    return SPEEDTEST_ERROR_NONE;
}

void batchTest(std::vector<nodeInfo> *nodes)
{
    nodeInfo node;
    unsigned int onlines = 0;
    long long tottraffic = 0;
    cur_node_id = -1;

    node_count = nodes->size();
    writeLog(LOG_TYPE_INFO, "Total node(s) found: " + to_string(node_count));
    if(node_count == 0)
    {
        writeLog(LOG_TYPE_ERROR, "No nodes are found in this subscription.");
        printMsgDirect(SPEEDTEST_ERROR_NONODES, rpcmode);
    }
    else
    {
        resultInit();
        writeLog(LOG_TYPE_INFO, "Speedtest will now begin.");
        printMsgDirect(SPEEDTEST_MESSAGE_BEGIN, rpcmode);
        //first print out all nodes when in Web mode
        if(rpcmode)
        {
            for(auto &x : *nodes)
            {
                printMsg(SPEEDTEST_MESSAGE_GOTSERVER, &x, rpcmode);
            }
        }
        //then we start testing nodes
        for(auto &x : *nodes)
        {
            if(custom_group.size() != 0)
                x.group = custom_group;
            singleTest(&x);
            //writeResult(&x, export_with_maxspeed);
            tottraffic += x.totalRecvBytes;
            if(x.online)
                onlines++;
        }
        //resultEOF(speedCalc(tottraffic * 1.0), onlines, nodes->size());
        writeLog(LOG_TYPE_INFO, "All nodes tested. Total/Online nodes: " + to_string(node_count) + "/" + to_string(onlines) + " Traffic used: " + speedCalc(tottraffic * 1.0));
        //exportHTML();
        saveResult(nodes);
        if(!multilink || (multilink && multilink_export_as_one_image))
        {
            printMsgDirect(SPEEDTEST_MESSAGE_PICSAVING, rpcmode);
            writeLog(LOG_TYPE_INFO, "Now exporting result...");
            pngpath = exportRender(resultPath, *nodes, export_with_maxspeed, export_sort_method, export_color_style, export_as_new_style);
            writeLog(LOG_TYPE_INFO, "Result saved to " + pngpath + " .");
            {
                clearTrans();
                addTrans("?picpath?", pngpath);
                printMsgWithDict(SPEEDTEST_MESSAGE_PICSAVED, rpcmode, dict, trans);
            }
            if(rpcmode)
            {
                clearTrans();
                addTrans("?data?", "data:image/png;base64," + fileToBase64(pngpath));
                printMsgWithDict(SPEEDTEST_MESSAGE_PICDATA, rpcmode, dict, trans);
            }
        }
    }
    cur_node_id = -1;
}

void rewriteNodeID(std::vector<nodeInfo> *nodes)
{
    int index = 0;
    for(auto &x : *nodes)
    {
        if(x.proxyStr == "LOG")
            return;
        x.id = index;
        index++;
    }
}

void rewriteNodeGroupID(std::vector<nodeInfo> *nodes, int groupID)
{
    for(auto &x : *nodes)
    {
        x.groupID = groupID;
    }
}

void addNodes(std::string link, bool multilink)
{
    int linkType = -1;
    std::vector<nodeInfo> nodes;
    nodeInfo node;
    std::string strSub, strInput, fileContent, strProxy;

    link = replace_all_distinct(link, "\"", "");
    writeLog(LOG_TYPE_INFO, "Received Link.");
    if(strFind(link, "vmess://") || strFind(link, "vmess1://"))
        linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
    else if(strFind(link, "ss://"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSS;
    else if(strFind(link, "ssr://"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
    else if(strFind(link, "socks://") || strFind(link, "https://t.me/socks") || strFind(link, "tg://socks"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
    else if(strFind(link, "http://") || strFind(link, "https://") || strFind(link, "surge:///install-config"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSUB;
    else if(strFind(link, "Netch://"))
        linkType = SPEEDTEST_MESSAGE_FOUNDNETCH;
    else if(link == "data:upload")
        linkType = SPEEDTEST_MESSAGE_FOUNDUPD;
    else if(fileExist(link))
        linkType = SPEEDTEST_MESSAGE_FOUNDLOCAL;

    switch(linkType)
    {
    case SPEEDTEST_MESSAGE_FOUNDSUB:
        printMsgDirect(SPEEDTEST_MESSAGE_FOUNDSUB, rpcmode);
        if(!rpcmode && !multilink && !webserver_mode)
        {
            printMsgDirect(SPEEDTEST_MESSAGE_GROUP, rpcmode);
            getline(std::cin, strInput);
            if(strInput.size())
            {
                custom_group = GBKToUTF8(strInput);
                writeLog(LOG_TYPE_INFO, "Received custom group: " + custom_group);
            }
        }
        writeLog(LOG_TYPE_INFO, "Downloading subscription data...");
        printMsgDirect(SPEEDTEST_MESSAGE_FETCHSUB, rpcmode);
        if(strFind(link, "surge:///install-config")) //surge config link
            link = UrlDecode(getUrlArg(link, "url"));
        strSub = webGet(link);
        if(strSub.size() == 0)
        {
            //try to get it again with system proxy
            writeLog(LOG_TYPE_WARN, "Cannot download subscription directly. Using system proxy.");
            strProxy = getSystemProxy();
            if(strProxy != "")
            {
                printMsgDirect(SPEEDTEST_ERROR_SUBFETCHERR, rpcmode);
                strSub = webGet(link, strProxy);
            }
            else
                writeLog(LOG_TYPE_WARN, "No system proxy is set. Skipping.");
        }
        if(strSub.size())
        {
            writeLog(LOG_TYPE_INFO, "Parsing subscription data...");
            explodeConfContent(strSub, override_conf_port, socksport, ss_libev, ssr_libev, nodes, custom_exclude_remarks, custom_include_remarks);
            rewriteNodeGroupID(&nodes, curGroupID);
            copyNodes(&nodes, &allNodes);
        }
        else
        {
            writeLog(LOG_TYPE_ERROR, "Cannot download subscription data.");
            printMsgDirect(SPEEDTEST_ERROR_INVALIDSUB, rpcmode);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDLOCAL:
        printMsgDirect(SPEEDTEST_MESSAGE_FOUNDLOCAL, rpcmode);
        if(!rpcmode && !multilink)
        {
            printMsgDirect(SPEEDTEST_MESSAGE_GROUP, rpcmode);
            getline(std::cin, strInput);
            if(strInput.size())
            {
                custom_group = GBKToUTF8(strInput);
                writeLog(LOG_TYPE_INFO, "Received custom group: " + custom_group);
            }
        }
        writeLog(LOG_TYPE_INFO, "Parsing configuration file data...");
        printMsgDirect(SPEEDTEST_MESSAGE_PARSING, rpcmode);
        if(explodeConf(link, override_conf_port, socksport, ss_libev, ssr_libev, nodes, custom_exclude_remarks, custom_include_remarks) == SPEEDTEST_ERROR_UNRECOGFILE)
        {
            printMsgDirect(SPEEDTEST_ERROR_UNRECOGFILE, rpcmode);
            writeLog(LOG_TYPE_ERROR, "Invalid configuration file!");
        }
        else
        {
            rewriteNodeGroupID(&nodes, curGroupID);
            copyNodes(&nodes, &allNodes);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDUPD:
        printMsgDirect(SPEEDTEST_MESSAGE_FOUNDUPD, rpcmode);
        std::cin.clear();
        //now we should ready to receive a large amount of data from stdin
        getline(std::cin, fileContent);
        fileContent = base64_decode(fileContent.substr(fileContent.find(",") + 1));
        writeLog(LOG_TYPE_INFO, "Parsing configuration file data...");
        printMsgDirect(SPEEDTEST_MESSAGE_PARSING, rpcmode);
        if(explodeConfContent(fileContent, override_conf_port, socksport, ss_libev, ssr_libev, nodes, custom_exclude_remarks, custom_include_remarks) == SPEEDTEST_ERROR_UNRECOGFILE)
        {
            printMsgDirect(SPEEDTEST_ERROR_UNRECOGFILE, rpcmode);
            writeLog(LOG_TYPE_ERROR, "Invalid configuration file!");
        }
        else
        {
            rewriteNodeGroupID(&nodes, curGroupID);
            copyNodes(&nodes, &allNodes);
        }
        break;
    default:
        if(linkType > 0)
        {
            node_count = 1;
            printMsg(linkType, &node, rpcmode);
            explode(link, ss_libev, ssr_libev, override_conf_port, socksport, node);
            if(custom_group.size() != 0)
                node.group = custom_group;
            if(node.server == "")
            {
                writeLog(LOG_TYPE_ERROR, "No valid link found.");
                printMsgDirect(SPEEDTEST_ERROR_NORECOGLINK, rpcmode);
            }
            else
            {
                node.groupID = curGroupID;
                allNodes.push_back(node);
            }
        }
        else
        {
            writeLog(LOG_TYPE_ERROR, "No valid link found.");
            printMsgDirect(SPEEDTEST_ERROR_NORECOGLINK, rpcmode);
        }
    }
}

int main(int argc, char* argv[])
{
    /*
    //do some trick to allow child processes die on termination
    #ifndef _WIN32
    setsid();
    unshare(CLONE_NEWPID | CLONE_NEWUSER);
    fileWrite("/proc/self/uid_map", "0 " + to_string(getuid()) + " 1", false);
    int retVal = fork();
    if(retVal == -1)
    {
        cerr << "error on fork" << endl;
        return 1;
    }
    else if(retVal != 0)
        return 0;
    #endif // _WIN32
    */
    std::vector<nodeInfo> nodes;
    nodeInfo node;
    std::string link;
    std::string curPNGPath, curPNGPathPrefix;
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    signal(SIGINT, signalHandler);

    chkArg(argc, argv);

    makeDir("logs");
    makeDir("results");
    logInit(rpcmode);
    readConf("pref.ini");
#ifdef _WIN32
    //start up windows socket library first
    WSADATA wsd;
    if(WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printMsgDirect(SPEEDTEST_ERROR_WSAERR, rpcmode);
        return -1;
    }
    //along with some console window info
    SetConsoleOutputCP(65001);
#endif // _WIN32
    if(!rpcmode)
        SetConsoleTitle("Stair Speedtest Reborn " VERSION);
    //kill any client before testing
    /*
    killClient(SPEEDTEST_MESSAGE_FOUNDVMESS);
    killClient(SPEEDTEST_MESSAGE_FOUNDSS);
    killClient(SPEEDTEST_MESSAGE_FOUNDSSR);
    */
    clientCheck();
    socksport = checkPort(socksport);
    writeLog(LOG_TYPE_INFO, "Using local port: " + to_string(socksport));
    writeLog(LOG_TYPE_INFO, "Init completed.");
    //intro message
    if(webserver_mode)
    {
        ssrspeed_webserver_routine(listen_address, listen_port);
        return 0;
    }
    printMsgDirect(SPEEDTEST_MESSAGE_WELCOME, rpcmode);
    getline(std::cin, link);
    writeLog(LOG_TYPE_INFO, "Input data: " + GBKToUTF8(link));
    if(rpcmode)
    {
        string_array webargs = split(link, "^");
        if(webargs.size() == 6)
        {
            link = webargs[0];
            if(webargs[1] != "?empty?")
                custom_group = webargs[1];
            speedtest_mode = webargs[2];
            export_sort_method = webargs[4];
            export_with_maxspeed = webargs[5] == "true";
        }
        else
        {
            link = "?empty?";
        }
    }
    if(strFind(link, "|"))
    {
        multilink = true;
        printMsgDirect(SPEEDTEST_MESSAGE_MULTILINK, rpcmode);
        string_array linkList = split(link, "|");
        for(auto &x : linkList)
        {
            addNodes(x, multilink);
            curGroupID++;
        }
    }
    else
    {
        addNodes(link, multilink);
    }
    rewriteNodeID(&allNodes); //reset all index
    if(allNodes.size() > 1) //group or multi-link
    {
        batchTest(&allNodes);
        if(multilink)
        {
            if(multilink_export_as_one_image)
            {
                printMsgDirect(SPEEDTEST_MESSAGE_PICSAVING, rpcmode);
                writeLog(LOG_TYPE_INFO, "Now exporting result...");
                curPNGPath = replace_all_distinct(resultPath, ".log", "") + "-multilink-all.png";
                pngpath = exportRender(curPNGPath, allNodes, export_with_maxspeed, export_sort_method, export_color_style, export_as_new_style);
                {
                    clearTrans();
                    addTrans("?picpath?", pngpath);
                    printMsgWithDict(SPEEDTEST_MESSAGE_PICSAVED, rpcmode, dict, trans);
                }
                writeLog(LOG_TYPE_INFO, "Result saved to " + pngpath + " .");
                if(rpcmode)
                {
                    clearTrans();
                    addTrans("?data?", "data:image/png;base64," + fileToBase64(pngpath));
                    printMsgWithDict(SPEEDTEST_MESSAGE_PICDATA, rpcmode, dict, trans);
                }
            }
            else
            {
                printMsgDirect(SPEEDTEST_MESSAGE_PICSAVING, rpcmode);
                curPNGPathPrefix = replace_all_distinct(resultPath, ".log", "");
                for(int i = 0; i < curGroupID; i++)
                {
                    eraseElements(nodes);
                    copyNodesWithGroupID(&allNodes, &nodes, i);
                    if(!nodes.size())
                        break;
                    if((nodes.size() == 1 && single_test_force_export) || nodes.size() > 1)
                    {
                        {
                            clearTrans();
                            addTrans("?id?", to_string(i +1));
                            printMsgWithDict(SPEEDTEST_MESSAGE_PICSAVINGMULTI, rpcmode, dict, trans);
                        }
                        writeLog(LOG_TYPE_INFO, "Now exporting result for group " + to_string(i + 1) + "...");
                        curPNGPath = curPNGPathPrefix + "-multilink-group" + to_string(i + 1) + ".png";
                        pngpath = exportRender(curPNGPath, nodes, export_with_maxspeed, export_sort_method, export_color_style, export_as_new_style);
                        {
                            clearTrans();
                            addTrans("?id?", to_string(i + 1));
                            addTrans("?picpath?", pngpath);
                            printMsgWithDict(SPEEDTEST_MESSAGE_PICSAVEDMULTI, rpcmode, dict, trans);
                        }
                        writeLog(LOG_TYPE_INFO, "Group " + to_string(i + 1) + " result saved to " + pngpath + " .");
                    }
                    else
                        writeLog(LOG_TYPE_INFO, "Group " + to_string(i + 1) + " result export skipped.");
                }
            }
        }
        writeLog(LOG_TYPE_INFO, "Multi-link test completed.");
    }
    else if(allNodes.size() == 1)
    {
        if(rpcmode)
        {
            printMsg(SPEEDTEST_MESSAGE_GOTSERVER, &allNodes[0], rpcmode);
        }
        singleTest(&allNodes[0]);
        if(single_test_force_export)
        {
            printMsgDirect(SPEEDTEST_MESSAGE_PICSAVING, rpcmode);
            writeLog(LOG_TYPE_INFO, "Now exporting result...");
            curPNGPath = "results" PATH_SLASH + getTime(1) + ".png";
            pngpath = exportRender(curPNGPath, allNodes, export_with_maxspeed, export_sort_method, export_color_style, export_as_new_style);
            {
                clearTrans();
                addTrans("?picpath?", pngpath);
                printMsgWithDict(SPEEDTEST_MESSAGE_PICSAVED, rpcmode, dict, trans);
            }
            writeLog(LOG_TYPE_INFO, "Result saved to " + pngpath + " .");
            if(rpcmode)
            {
                clearTrans();
                addTrans("?data?", "data:image/png;base64," + fileToBase64(pngpath));
                printMsgWithDict(SPEEDTEST_MESSAGE_PICDATA, rpcmode, dict, trans);
            }
        }
        writeLog(LOG_TYPE_INFO, "Single node test completed.");
    }
    else
    {
        writeLog(LOG_TYPE_ERROR, "No valid link found.");
        printMsgDirect(SPEEDTEST_ERROR_NORECOGLINK, rpcmode);
    }
    logEOF();
    printMsgDirect(SPEEDTEST_MESSAGE_EOF, rpcmode);
    sleep(1);
    //std::cin.clear();
    //std::cin.ignore();
    if(!rpcmode)
        _getch();
#ifdef _WIN32
    //stop socket library before exit
    WSACleanup();
#else
    std::cout<<std::endl;
#endif // _WIN32
    return 0;
}
