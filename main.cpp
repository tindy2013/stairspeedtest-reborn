#include <bits/stdc++.h>
#include <chrono>

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

using namespace std;
using namespace chrono;

#define MAX_FILE_SIZE 100 * 1024 * 1024
#define BUF_SIZE 8192

//for use globally
bool rpcmode = false;
int socksport = 65432;
string socksaddr = "127.0.0.1";
string custom_group;
string pngpath;

bool ss_libev = true;
bool ssr_libev = true;
string def_test_file = "https://download.microsoft.com/download/2/0/E/20E90413-712F-438C-988E-FDAA79A8AC3D/dotnetfx35.exe";
vector<downloadLink> downloadFiles;
vector<linkMatchRule> matchRules;
vector<string> exclude_remarks, include_remarks, dict, trans;
string speedtest_mode = "all";
string override_conf_port = "";
int def_thread_count = 4;
bool export_with_maxspeed = false;
bool test_site_ping = false;
string export_sort_method = "none";

int avail_status[3] = {1, 1, 1};
unsigned int node_count = 0;

#ifdef _WIN32
HANDLE hProc = 0;
#endif // _WIN32

//declarations

int perform_test(nodeInfo *node, string localaddr, int localport, string username, string password, int thread_count);
int tcping(nodeInfo *node);
void getTestFile(nodeInfo *node, socks5Proxy proxy, vector<downloadLink> *downloadFiles, vector<linkMatchRule> *matchRules, string defaultTestFile);

//original codes

void clearTrans()
{
    vector<string>().swap(dict);
    vector<string>().swap(trans);
}

void addTrans(string dictval, string transval)
{
    dict.push_back(dictval);
    trans.push_back(transval);
}

/*
void clientcheck()
{
    #ifdef _WIN32
    string v2core_path = "tools\\clients\\v2ray-core\\v2-core.exe -version";
    string ssr_libev_path = "tools\\clients\\shadowsocksr-libev\\ssr-libev.exe -h";
    string ss_libev_path = "tools\\clients\\shadowsocks-libev\\ss-libev.exe -h";
    #else
    string v2core_path = "//usr//bin//v2ray//v2ray -version";
    string ssr_libev_path = "ssr-local -h";
    string ss_libev_path = "ss-local -h";
    #endif // _WIN32

    if(ChkProgram(v2core_path) != 0)
        avail_status[0] = 0;
    if(ChkProgram(ssr_libev_path) != 0)
        avail_status[1] = 0;
    if(ChkProgram(ss_libev_path) != 0)
        avail_status[2] = 0;
}
*/

int runClient(int client, string runpath)
{
#ifdef _WIN32
    string v2core_path = "tools\\clients\\v2ray-core\\v2-core.exe -config config.json";
    string ssr_libev_path = "tools\\clients\\shadowsocksr-libev\\ssr-libev.exe -c config.json";
    string ss_libev_path = "tools\\clients\\shadowsocks-libev\\ss-libev.exe -c config.json";

    string ssr_win_dir = "tools\\clients\\shadowsocksr-win\\";
    string ssr_win_path = ssr_win_dir + "shadowsocksr-win.exe";
    string ss_win_dir = "tools\\clients\\shadowsocks-win\\";
    string ss_win_path = ss_win_dir + "shadowsocks-win.exe";

    switch(client)
    {
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        writeLog(LOG_TYPE_INFO, "Starting up v2ray core...");
        runProgram(v2core_path, "", false, &hProc);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        if(ssr_libev)
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocksr-libev...");
            runProgram(ssr_libev_path, "", false, &hProc);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocksr-win...");
            fileCopy("config.json", ssr_win_dir + "gui-config.json");
            runProgram(ssr_win_path, "", false, &hProc);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        if(ss_libev)
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocks-libev...");
            runProgram(ss_libev_path, "", false, &hProc);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Starting up shadowsocks-win...");
            fileCopy("config.json", ss_win_dir + "gui-config.json");
            runProgram(ss_win_path, "", false, &hProc);
        }
        break;
    }
#else
    string v2core_path = "v2ray -config config.json";
    string ssr_libev_path = "ssr-local -c config.json";
    string ss_libev_path = "ss-local -c config.json";

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
        runProgram(ss_libev_path, "", false);
        break;
    }
#endif // _WIN32
    return 0;
}

int killClient(int client)
{
    //TerminateProcess(hProc, 0);
#ifdef _WIN32
    killByHandle(hProc);
    string v2core_name = "v2-core.exe";
    string ss_libev_name = "ss-libev.exe";
    string ssr_libev_name = "ssr-libev.exe";
    string ss_win_name = "shadowsocks-win.exe";
    string ssr_win_name = "shadowsocksr-win.exe";

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
#else
    string v2core_name = "v2ray";
    string ss_libev_name = "ss-local";
    string ssr_libev_name = "ssr-local";

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

void readConf(string path)
{
    string strTemp, itemname, itemval;
    string parent = "^\\[(.*?)\\]$", child = "^(.*?)=(.*?)$";
    vector<string> vchild;
    ifstream infile;
    smatch result;
    downloadLink link;
    linkMatchRule rule;
    unsigned int i;

    infile.open(path, ios::in);
    while(getline(infile, strTemp))
    {
        strTemp = UTF8ToGBK(strTemp); //convert utf-8 to gbk on windows
        if(strTemp.find(";") == 0)
            continue;
        if(regMatch(strTemp, parent))
        {
            //don't do anything right now
        }
        else if(regMatch(strTemp, child))
        {
            /*
            vchild = split(strTemp, "=");
            if(vchild.size() < 2)
                continue;
            itemname = vchild[0];
            itemval = "";
            for(unsigned i = 1; i < vchild.size(); i++)
                itemval += vchild[i];
            */
            itemname = strTemp.substr(0, strTemp.find("="));
            itemval = strTemp.substr(strTemp.find("=") + 1);

            if(itemname == "speedtest_mode")
                speedtest_mode = itemval;
            #ifdef _WIN32
            //csharp version only works on windows
            else if(itemname == "preferred_ss_client" && itemval == "ss-csharp")
                ss_libev = false;
            else if(itemname == "preferred_ssr_client" && itemval == "ssr-csharp")
                ssr_libev = false;
            #endif // _WIN32
            else if(itemname == "export_with_maxspeed")
                export_with_maxspeed = itemval == "true";
            else if(itemname == "override_conf_port")
                override_conf_port = itemval;
            else if(strFind(itemname, "exclude_remarks"))
                exclude_remarks.push_back(itemval);
            else if(strFind(itemname, "include_remarks"))
                include_remarks.push_back(itemval);
            else if(itemname == "test_file_urls")
            {
                vchild = split(itemval, "|");
                if(vchild.size() == 2)
                {
                    link.url = vchild[0];
                    link.tag = vchild[1];
                    downloadFiles.push_back(link);
                }

            }
            else if(itemname == "rules")
            {
                vchild = split(itemval, "|");
                if(vchild.size() >= 3)
                {
                    rule.rules.clear();
                    rule.mode = vchild[0];
                    for(i = 1; i < vchild.size() - 1; i++)
                    {
                        rule.rules.push_back(vchild[i]);
                    }
                    rule.tag = vchild[vchild.size() - 1];
                    matchRules.push_back(rule);
                }
            }
            else if(itemname == "thread_count")
                def_thread_count = stoi(itemval);
            /*
            else if(itemname == "speetest_with_tls")
                useTLS = itemval == "true";
            else if(itemname == "colorset")
                colorgroup = vchild[1];
            else if(itemname == "bounds")
                bounds = vchild[1];
            else if(itemname == "colorcount")
                color_count = stoi(vchild[1]);
            */
        }
    }
    infile.close();
}

void chkArg(int argc, char* argv[])
{
    for(int i = 0; i<argc; i++)
    {
        if(!strcmp(argv[i], "/rpc"))
            rpcmode = true;
    }
}

void switchCodepage()
{
    //only needed on windows
#ifdef _WIN32
    system("chcp 65001>nul");
#endif // _WIN32
}

void exportHTML()
{
    string htmpath = replace_all_distinct(resultPath, ".log", ".htm");
    //string pngname = replace_all_distinct(replace_all_distinct(resultpath, ".log", ".png"), "results\\", "");
    //string resultname = replace_all_distinct(resultpath, "results\\", "");
    //string htmname = replace_all_distinct(htmpath, "results\\", "");
    //string rendercmd = "..\\tools\\misc\\phantomjs.exe ..\\tools\\misc\\render_alt.js " + htmname + " " + pngname + " " + export_sort_method;
    exportResult(htmpath, "tools\\misc\\util.js", "tools\\misc\\style.css", export_with_maxspeed);
    //runprogram(rendercmd, "results", true);
}

int singleTest(nodeInfo *node)
{
    int retVal = 0;
    string logdata = "", testserver, username, password;
    int testport;
    socks5Proxy proxy;

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
        writeToFile("config.json", node->proxyStr, true);
        runClient(node->linkType, "");
    }
    proxy.address = testserver;
    proxy.port = testport;
    proxy.username = username;
    proxy.password = password;

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
            return SPEEDTEST_ERROR_NORESOLVE;
        }
        if(node->pkLoss == "100.00%")
        {
            writeLog(LOG_TYPE_ERROR, "Cannot connect to this node.");
            printMsg(SPEEDTEST_ERROR_NOCONNECTION, node, rpcmode);
            killClient(node->linkType);
            return SPEEDTEST_ERROR_NOCONNECTION;
        }
        for(int i = 0; i < 6; i++)
        {
            logdata += to_string(node->rawPing[i]) + " ";
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
        clearTrans();
        addTrans("?id?", to_string(node->id));
        printMsgWithDict(SPEEDTEST_ERROR_GEOIPERR, rpcmode, dict, trans);
    }

    if(test_site_ping)
    {
        printMsg(SPEEDTEST_MESSAGE_STARTGPING, node, rpcmode);
        writeLog(LOG_TYPE_INFO, "Now performing site ping...");
        websitePing(node, "https://www.google.com/", testserver, testport, username, password);
        logdata = "";
        for(int i = 0; i < 6; i++)
        {
            logdata += to_string(node->rawSitePing[i]) + " ";
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
        for(int i = 0; i < 20; i++)
        {
            logdata += to_string(node->rawSpeed[i]) + " ";
        }
        writeLog(LOG_TYPE_RAW, logdata);
        if(node->totalRecvBytes == 0)
        {
            writeLog(LOG_TYPE_ERROR, "Speedtest returned no speed.");
            printMsg(SPEEDTEST_ERROR_RETEST, node, rpcmode);
            perform_test(node, testserver, testport, username, password, def_thread_count);
            logdata = "";
            for(int i = 0; i < 20; i++)
            {
                logdata += to_string(node->rawSpeed[i]) + " ";
            }
            writeLog(LOG_TYPE_RAW, logdata);
            if(node->totalRecvBytes == 0)
            {
                writeLog(LOG_TYPE_ERROR, "Speedtest returned no speed 2 times.");
                printMsg(SPEEDTEST_ERROR_NOSPEED, node, rpcmode);
                printMsg(SPEEDTEST_MESSAGE_GOTSPEED, node, rpcmode);
                killClient(node->linkType);
                return SPEEDTEST_ERROR_NOSPEED;
            }
        }
    }
    writeLog(LOG_TYPE_INFO, "Average speed: " + node->avgSpeed + "  Max speed: " + node->maxSpeed + " Traffic used in bytes: " + to_string(node->totalRecvBytes));
    printMsg(SPEEDTEST_MESSAGE_GOTSPEED, node, rpcmode);
    printMsg(SPEEDTEST_MESSAGE_GOTRESULT, node, rpcmode);
    node->online = true;
    killClient(node->linkType);
    return SPEEDTEST_ERROR_NONE;
}

void batchTest(vector<nodeInfo> nodes)
{
    nodeInfo node;
    unsigned int onlines = 0, index = 0;
    long long tottraffic = 0;

    node_count = nodes.size();
    writeLog(LOG_TYPE_INFO, "Total node(s) found: " + to_string(node_count));
    if(node_count == 0)
    {
        writeLog(LOG_TYPE_ERROR, "No nodes are found in this subscription.");
        printMsg(SPEEDTEST_ERROR_NONODES, &node, rpcmode);
    }
    else
    {
        resultInit(export_with_maxspeed);
        writeLog(LOG_TYPE_INFO, "Speedtest will now begin.");
        printMsg(SPEEDTEST_MESSAGE_BEGIN, &node, rpcmode);
        //first print out all nodes when in Web mode
        if(rpcmode)
        {
            for(index = 0; index < node_count; index++)
            {
                printMsg(SPEEDTEST_MESSAGE_GOTSERVER, &nodes[index], rpcmode);
            }
        }
        //then we start testing nodes
        for(index = 0; index < node_count; index++)
        {
            if(custom_group.size() != 0)
                nodes[index].group = custom_group;
            singleTest(&nodes[index]);
            writeResult(&nodes[index], export_with_maxspeed);
            tottraffic += nodes[index].totalRecvBytes;
            if(nodes[index].online)
                onlines++;
        }
        resultEOF(speedCalc(tottraffic * 1.0), onlines, nodes.size());
        writeLog(LOG_TYPE_INFO, "All nodes tested. Total/Online nodes: " + to_string(node_count) + "/" + to_string(onlines) + " Traffic used: " + speedCalc(tottraffic * 1.0));
        printMsg(SPEEDTEST_MESSAGE_PICSAVING, &node, rpcmode);
        writeLog(LOG_TYPE_INFO, "Now exporting result...");
        exportHTML();
        pngpath = exportRender(resultPath, nodes, export_with_maxspeed, export_sort_method);
        writeLog(LOG_TYPE_INFO, "Result saved to " + pngpath + " .");
        //printMsg(SPEEDTEST_MESSAGE_PICSAVED, &node, rpcmode);
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

int main(int argc, char* argv[])
{
    vector<nodeInfo> nodes;
    nodeInfo node;
    string link, strSub, strInput, fileContent, strProxy;
    int linkType = -1;
    cout<<fixed;
    cout<<setprecision(2);

    logInit(rpcmode);
    //speedtest.bat :main
    readConf("pref.ini");
    chkArg(argc, argv);
#ifdef _WIN32
    //start up windows socket library first
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printMsg(SPEEDTEST_ERROR_WSAERR, &node, rpcmode);
        return -1;
    }
    //along with some console window info
    if(rpcmode)
        switchCodepage();
    else
        SetConsoleTitle("Stair Speedtest " VERSION);
#endif // _WIN32
    //kill any client before testing
    killClient(SPEEDTEST_MESSAGE_FOUNDVMESS);
    killClient(SPEEDTEST_MESSAGE_FOUNDSS);
    killClient(SPEEDTEST_MESSAGE_FOUNDSSR);
    socksport = checkPort(socksport);
    writeLog(LOG_TYPE_INFO, "Using local port: " + to_string(socksport));
    writeLog(LOG_TYPE_INFO, "Init completed.");
    //intro message
    printMsg(SPEEDTEST_MESSAGE_WELCOME, &node, rpcmode);
    getline(cin, link);
    writeLog(LOG_TYPE_INFO, "Input data: " + link);
    if(rpcmode)
    {
        vector<string> webargs = split(link, "^");
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
    writeLog(LOG_TYPE_INFO, "Received Link.");
    if(strFind(link, "vmess://"))
        linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
    else if(strFind(link, "ss://"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSS;
    else if(strFind(link, "ssr://"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
    else if(strFind(link, "socks://") || strFind(link, "https://t.me/socks") || strFind(link, "tg://socks"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
    else if(strFind(link, "http://") || strFind(link, "https://") || strFind(link, "surge:///install-config"))
        linkType = SPEEDTEST_MESSAGE_FOUNDSUB;
    else if(link == "data:upload")
        linkType = SPEEDTEST_MESSAGE_FOUNDUPD;
    else if(fileExist(link))
        linkType = SPEEDTEST_MESSAGE_FOUNDLOCAL;


    switch(linkType)
    {
    case SPEEDTEST_MESSAGE_FOUNDSUB:
        printMsg(SPEEDTEST_MESSAGE_FOUNDSUB, &node, rpcmode);
        if(!rpcmode)
        {
            printMsg(SPEEDTEST_MESSAGE_GROUP, &node, rpcmode);
            getline(cin, strInput);
            if(strInput.size())
            {
                custom_group = GBKToUTF8(strInput);
                writeLog(LOG_TYPE_INFO, "Received custom group: " + custom_group);
            }
        }
        switchCodepage();
        writeLog(LOG_TYPE_INFO, "Downloading subscription data...");
        printMsg(SPEEDTEST_MESSAGE_FETCHSUB, &node, rpcmode);
        if(strFind(link, "surge:///install-config")) //surge config link
            link = UrlDecode(getUrlArg(link, "url"));
        strSub = webGet(link);
        if(strSub.size() == 0)
        {
            //try to get it again with system proxy
            strProxy = getSystemProxy();
            if(strProxy != "")
                strSub = webGet(link, strProxy);
        }
        writeLog(LOG_TYPE_INFO, "Parsing subscription data...");
        if(strSub.size())
        {
            explodeConfContent(strSub, override_conf_port, socksport, ss_libev, ssr_libev, &nodes, &exclude_remarks, &include_remarks);
            batchTest(nodes);
            writeLog(LOG_TYPE_INFO, "Subscription test completed.");
        }
        else
        {
            writeLog(LOG_TYPE_ERROR, "Cannot download subscription data.");
            printMsg(SPEEDTEST_ERROR_INVALIDSUB, &node, rpcmode);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDLOCAL:
        printMsg(SPEEDTEST_MESSAGE_FOUNDLOCAL, &node, rpcmode);
        if(!rpcmode)
        {
            printMsg(SPEEDTEST_MESSAGE_GROUP, &node, rpcmode);
            getline(cin, strInput);
            if(strInput.size())
            {
                custom_group = GBKToUTF8(strInput);
                writeLog(LOG_TYPE_INFO, "Received custom group: " + custom_group);
            }
        }
        switchCodepage();
        writeLog(LOG_TYPE_INFO, "Parsing configuration file data...");
        printMsg(SPEEDTEST_MESSAGE_PARSING, &node, rpcmode);
        if(explodeConf(link, override_conf_port, socksport, ss_libev, ssr_libev, &nodes, &exclude_remarks, &include_remarks) == SPEEDTEST_ERROR_UNRECOGFILE)
        {
            printMsg(SPEEDTEST_ERROR_UNRECOGFILE, &node, rpcmode);
            writeLog(LOG_TYPE_ERROR, "Invalid configuration file!");
        }
        else
        {
            batchTest(nodes);
        }
        writeLog(LOG_TYPE_INFO, "Configuration test completed.");
        break;
    case SPEEDTEST_MESSAGE_FOUNDUPD:
        printMsg(SPEEDTEST_MESSAGE_FOUNDUPD, &node, rpcmode);
        cin.clear();
        //now we should ready to receive a large amount of data from stdin
        getline(cin, fileContent);
        //writeLog(LOG_TYPE_RAW, fileContent);
        fileContent = base64_decode(fileContent.substr(fileContent.find(",") + 1));
        writeLog(LOG_TYPE_RAW, fileContent);
        writeLog(LOG_TYPE_INFO, "Parsing configuration file data...");
        printMsg(SPEEDTEST_MESSAGE_PARSING, &node, rpcmode);
        if(explodeConfContent(fileContent, override_conf_port, socksport, ss_libev, ssr_libev, &nodes, &exclude_remarks, &include_remarks) == SPEEDTEST_ERROR_UNRECOGFILE)
        {
            printMsg(SPEEDTEST_ERROR_UNRECOGFILE, &node, rpcmode);
            writeLog(LOG_TYPE_ERROR, "Invalid configuration file!");
        }
        else
        {
            batchTest(nodes);
        }
        writeLog(LOG_TYPE_INFO, "Configuration test completed.");
        break;
    default:
        if(linkType > 0)
        {
            node_count = 1;
            switchCodepage();
            printMsg(linkType, &node, rpcmode);
            explode(link, ss_libev, ssr_libev, override_conf_port, socksport, &node);
            if(custom_group.size() != 0)
                node.group = custom_group;
            if(node.server == "")
            {
                writeLog(LOG_TYPE_ERROR, "No valid link found.");
                printMsg(SPEEDTEST_ERROR_NORECOGLINK, &node, rpcmode);
            }
            else
            {
                printMsg(SPEEDTEST_MESSAGE_BEGIN, &node, rpcmode);
                if(rpcmode)
                {
                    clearTrans();
                    addTrans("?group?", node.group);
                    addTrans("?remarks?", node.remarks);
                    addTrans("?index?", to_string(node.id + 1));
                    printMsgWithDict(SPEEDTEST_MESSAGE_GOTSERVER, rpcmode, dict, trans);
                }
                singleTest(&node);
            }
            writeLog(LOG_TYPE_INFO, "Single node test completed.");
        }
        else
        {
            writeLog(LOG_TYPE_ERROR, "No valid link found.");
            printMsg(SPEEDTEST_ERROR_NORECOGLINK, &node, rpcmode);
        }
    }
    logEOF();
    clearTrans();
    //printMsg(SPEEDTEST_MESSAGE_EOF, &node, rpcmode);
    printMsgWithDict(SPEEDTEST_MESSAGE_EOF, rpcmode, dict, trans);
    sleep(1);
#ifdef _WIN32
    if(!rpcmode)
        system("pause>nul");
    //stop socket library before exit
    WSACleanup();
#endif // _WIN32
    return 0;
}
