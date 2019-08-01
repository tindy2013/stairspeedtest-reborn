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
vector<string> exclude_remarks, include_remarks, dict, trans;
string speedtest_mode = "all";
string override_conf_port = "";
int def_thread_count = 4;
bool export_with_maxspeed = false;
string export_sort_method = "none";

int avail_status[3] = {1, 1, 1};

#ifdef _WIN32
HANDLE hProc = 0;
#endif // _WIN32

//declarations

int perform_test(nodeInfo *node, string testfile, string localaddr, int localport, int thread_count);
int tcping(nodeInfo *node);

//original codes

void cleartrans()
{
    vector<string>().swap(dict);
    vector<string>().swap(trans);
}

void addtrans(string dictval, string transval)
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

int runclient(int client, string runpath)
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
        writelog(LOG_TYPE_INFO, "Starting up v2ray core...");
        RunProgram(v2core_path, "", false, &hProc);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        if(ssr_libev)
        {
            writelog(LOG_TYPE_INFO, "Starting up shadowsocksr-libev...");
            RunProgram(ssr_libev_path, "", false, &hProc);
        }
        else
        {
            writelog(LOG_TYPE_INFO, "Starting up shadowsocksr-win...");
            fileCopy("config.json", ssr_win_dir + "gui-config.json");
            RunProgram(ssr_win_path, "", false, &hProc);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        if(ss_libev)
        {
            writelog(LOG_TYPE_INFO, "Starting up shadowsocks-libev...");
            RunProgram(ss_libev_path, "", false, &hProc);
        }
        else
        {
            writelog(LOG_TYPE_INFO, "Starting up shadowsocks-win...");
            fileCopy("config.json", ss_win_dir + "gui-config.json");
            RunProgram(ss_win_path, "", false, &hProc);
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
        writelog(LOG_TYPE_INFO, "Starting up v2ray core...");
        RunProgram(v2core_path, "", false);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        writelog(LOG_TYPE_INFO, "Starting up shadowsocksr-libev...");
        RunProgram(ssr_libev_path, "", false);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        writelog(LOG_TYPE_INFO, "Starting up shadowsocks-libev...");
        RunProgram(ss_libev_path, "", false);
        break;
    }
#endif // _WIN32
    return 0;
}

int killclient(int client)
{
    //TerminateProcess(hProc, 0);
#ifdef _WIN32
    KillByHandle(hProc);
    string v2core_name = "v2-core.exe";
    string ss_libev_name = "ss-libev.exe";
    string ssr_libev_name = "ssr-libev.exe";
    string ss_win_name = "shadowsocks-win.exe";
    string ssr_win_name = "shadowsocksr-win.exe";

    switch(client)
    {
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        writelog(LOG_TYPE_INFO, "Killing v2ray core...");
        KillProgram(v2core_name);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        if(ssr_libev)
        {
            writelog(LOG_TYPE_INFO, "Killing shadowsocksr-libev...");
            KillProgram(ssr_libev_name);
        }
        else
        {
            writelog(LOG_TYPE_INFO, "Killing shadowsocksr-win...");
            KillProgram(ssr_win_name);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        if(ss_libev)
        {
            writelog(LOG_TYPE_INFO, "Killing shadowsocks-libev...");
            KillProgram(ss_libev_name);
        }
        else
        {
            writelog(LOG_TYPE_INFO, "Killing shadowsocks-win...");
            KillProgram(ss_win_name);
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
        writelog(LOG_TYPE_INFO, "Killing v2ray core...");
        KillProgram(v2core_name);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        writelog(LOG_TYPE_INFO, "Killing shadowsocksr-libev...");
        KillProgram(ssr_libev_name);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        writelog(LOG_TYPE_INFO, "Killing shadowsocks-libev...");
        KillProgram(ss_libev_name);
        break;
    }
#endif
    return 0;
}

void readconf(string path)
{
    string strTemp, itemval = "";
    string parent = "^\\[(.*?)\\]$", child = "^(.*?)=(.*?)$";
    vector<string> vchild;
    ifstream infile;
    smatch result;
    infile.open(path, ios::in);
    while(getline(infile, strTemp))
    {
        strTemp = UTF8ToGBK(strTemp); //convert utf-8 to gbk on windows
        if(strTemp.find(";") == 0)
            continue;
        if(regmatch(strTemp, parent))
        {
            //don't do anything right now
        }
        else if(regmatch(strTemp, child))
        {
            vchild = split(strTemp, "=");
            if(vchild.size() < 2)
                continue;
            itemval = "";
            for(unsigned i = 1; i<vchild.size(); i++)
                itemval += vchild[i];
            if(vchild[0] == "speedtest_mode")
                speedtest_mode = vchild[1];
            #ifdef _WIN32
            //csharp version only works on windows
            else if(vchild[0] == "preferred_ss_client"&&vchild[1] == "ss-csharp")
                ss_libev = false;
            else if(vchild[0] == "preferred_ssr_client"&&vchild[1] == "ssr-csharp")
                ssr_libev = false;
            #endif // _WIN32
            else if(vchild[0] == "export_with_maxspeed")
                export_with_maxspeed = vchild[1] == "true";
            else if(vchild[0] == "override_conf_port")
                override_conf_port = vchild[1];
            else if(strfind(vchild[0], "exclude_remarks"))
                exclude_remarks.push_back(itemval);
            else if(strfind(vchild[0], "include_remarks"))
                include_remarks.push_back(itemval);
            else if(vchild[0] == "test_file_url")
                def_test_file = itemval;
            /*
            else if(vchild[0] == "colorset")
                colorgroup = vchild[1];
            else if(vchild[0] == "bounds")
                bounds = vchild[1];
            else if(vchild[0] == "colorcount")
                color_count = stoi(vchild[1]);
            */
        }
    }
    infile.close();
}

void chkarg(int argc, char* argv[])
{
    for(int i = 0; i<argc; i++)
    {
        if(!strcmp(argv[i], "/rpc"))
            rpcmode = true;
    }
}

void switch_codepage()
{
    //only needed on windows
#ifdef _WIN32
    system("chcp 65001>nul");
#endif // _WIN32
}

void export_html()
{
    string htmpath = replace_all_distinct(resultpath, ".log", ".htm");
    //string pngname = replace_all_distinct(replace_all_distinct(resultpath, ".log", ".png"), "results\\", "");
    //string resultname = replace_all_distinct(resultpath, "results\\", "");
    //string htmname = replace_all_distinct(htmpath, "results\\", "");
    //string rendercmd = "..\\tools\\misc\\phantomjs.exe ..\\tools\\misc\\render_alt.js " + htmname + " " + pngname + " " + export_sort_method;
    exportresult(htmpath, "tools\\misc\\util.js", "tools\\misc\\style.css", export_with_maxspeed);
    //runprogram(rendercmd, "results", true);
}

/*
string getRegVal()
{
    HKEY key;
    DWORD dwSize;
    DWORD dwType = REG_SZ;
    wchar_t *data = {};
    string retdata;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", 0, KEY_QUERY_VALUE, &key) == 0)
    {
        if (RegQueryValueEx(key, "ProxyServer", 0, &dwType, NULL, &dwSize) == 0)
        {
            RegQueryValueEx(key, "ProxyServer", 0, &dwType, (LPBYTE)data, &dwSize);
        }
    }
    cout<<data;
    DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, data, -1, NULL, 0, NULL, FALSE);
    char *psText;
    psText = new char[dwNum];
    WideCharToMultiByte (CP_OEMCP, NULL, data, -1, psText, dwNum, NULL, FALSE);
    retdata = psText;
    delete []psText;
    return retdata;
}
*/

int singletest(string testfile, nodeInfo *node)
{
    int retVal = 0;
    string logdata = "";
    writelog(LOG_TYPE_INFO, "Writing config file...");
    write2file("config.json", node->proxystr, true);
    //retVal = execve("tools\\clients\\v2ray-core\\v2-core.exe", NULL, NULL);
    //if(retVal == ENOENT) cout<<"error"<<endl;
    runclient(node->linktype, "");
    //website_ping(node, "https://www.google.com/", socksaddr, socksport, "", "");
    printmsg(SPEEDTEST_MESSAGE_GOTSERVER, node, rpcmode);
    writelog(LOG_TYPE_INFO, "Received server. Group: " + node->group + " Name: " + node->remarks);
    printmsg(SPEEDTEST_MESSAGE_STARTPING, node, rpcmode);
    if(speedtest_mode != "speedonly")
    {
        writelog(LOG_TYPE_INFO, "Now performing TCP ping...");
        retVal = tcping(node);
        for(int i = 0; i<6; i++)
        {
            logdata += to_string(node->raw_ping[i]) + " ";
        }
        writelog(LOG_TYPE_RAW, logdata);
        if(retVal == SPEEDTEST_ERROR_NORESOLVE)
        {
            writelog(LOG_TYPE_ERROR, "Node address resolve error.");
            printmsg(SPEEDTEST_ERROR_NORESOLVE, node, rpcmode);
            return SPEEDTEST_ERROR_NORESOLVE;
        }
        if(node->pkloss == "100.00%")
        {
            writelog(LOG_TYPE_ERROR, "Cannot connect to this node.");
            printmsg(SPEEDTEST_ERROR_NOCONNECTION, node, rpcmode);
            killclient(node->linktype);
            return SPEEDTEST_ERROR_NOCONNECTION;
        }
        writelog(LOG_TYPE_INFO, "TCP Ping: " + node->avgping + "  Packet Loss: " + node->pkloss);
    }
    else
        node->pkloss = "0.00%";
    //website_ping(node, "https://www.bing.com", socksaddr, socksport, "", "");
    printmsg(SPEEDTEST_MESSAGE_GOTPING, node, rpcmode);
    printmsg(SPEEDTEST_MESSAGE_STARTSPEED, node, rpcmode);
    //node->total_recv_bytes = 1;
    if(speedtest_mode != "pingonly")
    {
        writelog(LOG_TYPE_INFO, "Now performing file download speed test...");
        perform_test(node, testfile, socksaddr, socksport, def_thread_count);
        logdata = "";
        for(int i = 0; i<20; i++)
        {
            logdata += to_string(node->raw_speed[i]) + " ";
        }
        writelog(LOG_TYPE_RAW, logdata);
        if(node->total_recv_bytes == 0)
        {
            writelog(LOG_TYPE_ERROR, "Speedtest returned no speed.");
            printmsg(SPEEDTEST_ERROR_RETEST, node, rpcmode);
            perform_test(node, testfile, socksaddr, socksport, def_thread_count);
            logdata = "";
            for(int i = 0; i<20; i++)
            {
                logdata += to_string(node->raw_speed[i]) + " ";
            }
            writelog(LOG_TYPE_RAW, logdata);
            if(node->total_recv_bytes == 0)
            {
                writelog(LOG_TYPE_ERROR, "Speedtest returned no speed 2 times.");
                printmsg(SPEEDTEST_ERROR_NOSPEED, node, rpcmode);
                printmsg(SPEEDTEST_MESSAGE_GOTSPEED, node, rpcmode);
                killclient(node->linktype);
                return SPEEDTEST_ERROR_NOSPEED;
            }
        }
    }
    writelog(LOG_TYPE_INFO, "Average speed: " + node->avgspeed + "  Max speed: " + node->maxspeed + " Traffic used in bytes: " + to_string(node->total_recv_bytes));
    printmsg(SPEEDTEST_MESSAGE_GOTSPEED, node, rpcmode);
    printmsg(SPEEDTEST_MESSAGE_GOTRESULT, node, rpcmode);
    node->online = true;
    killclient(node->linktype);
    return SPEEDTEST_ERROR_NONE;
}

void batchtest(vector<nodeInfo> nodes)
{
    nodeInfo node;
    unsigned int node_count = 0, onlines = 0;
    long long tottraffic = 0;

    node_count = nodes.size();
    writelog(LOG_TYPE_INFO, "Total node(s) found: " + to_string(node_count));
    if(node_count == 0)
    {
        writelog(LOG_TYPE_ERROR, "No nodes are found in this subscription.");
        printmsg(SPEEDTEST_ERROR_NONODES, &node, rpcmode);
    }
    else
    {
        resultinit(export_with_maxspeed);
        writelog(LOG_TYPE_INFO, "Speedtest will now begin.");
        printmsg(SPEEDTEST_MESSAGE_BEGIN, &node, rpcmode);
        for(unsigned i = 0; i < node_count; i++)
        {
            if(custom_group.size() != 0)
                nodes[i].group = custom_group;
            singletest(def_test_file, &nodes[i]);
            writeresult(&nodes[i], export_with_maxspeed);
            tottraffic += nodes[i].total_recv_bytes;
            if(nodes[i].online)
                onlines++;
        }
        resulteof(speedCalc(tottraffic * 1.0), onlines, nodes.size());
        writelog(LOG_TYPE_INFO, "All nodes tested. Total/Online nodes: " + to_string(node_count) + "/" + to_string(onlines) + " Traffic used: " + speedCalc(tottraffic * 1.0));
        printmsg(SPEEDTEST_MESSAGE_PICSAVING, &node, rpcmode);
        writelog(LOG_TYPE_INFO, "Now exporting result...");
        export_html();
        pngpath = export_render(resultpath, nodes, export_with_maxspeed, export_sort_method);
        writelog(LOG_TYPE_INFO, "Result saved to " + pngpath + " .");
        //printmsg(SPEEDTEST_MESSAGE_PICSAVED, &node, rpcmode);
        {
            cleartrans();
            addtrans("?picpath?", pngpath);
            printmsg_with_dict(SPEEDTEST_MESSAGE_PICSAVED, rpcmode, dict, trans);
        }
        if(rpcmode)
        {
            cleartrans();
            addtrans("?data?", "data:image/png;base64," + fileToBase64(pngpath));
            printmsg_with_dict(SPEEDTEST_MESSAGE_PICDATA, rpcmode, dict, trans);
        }
    }
}

int main(int argc, char* argv[])
{
    vector<nodeInfo> nodes;
    nodeInfo node;
    string link, strSub, strInput, filecontent;
    int linktype = -1;
    //cin>>link;
    cout<<fixed;
    cout<<setprecision(2);
    //cout.rdbuf()->pubsetbuf(0, 0);
    //ios_base::sync_with_stdio(false);
    loginit(rpcmode);
    //rpcmode = true;
    //locale::global(locale(""));
    //cout<<getRegVal();
    //speedtest.bat :main
    readconf("pref.ini");
    chkarg(argc, argv);
#ifdef _WIN32
    //start up windows socket library first
    WSADATA wsd; //WSADATA变量
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        printmsg(SPEEDTEST_ERROR_WSAERR, &node, rpcmode);
        return -1;
    }
    //along with some console window info
    if(rpcmode)
        switch_codepage();
    else
        SetConsoleTitle(L"Stair Speedtest");
#endif // _WIN32
    //kill any client before testing
    killclient(SPEEDTEST_MESSAGE_FOUNDVMESS);
    killclient(SPEEDTEST_MESSAGE_FOUNDSS);
    killclient(SPEEDTEST_MESSAGE_FOUNDSSR);
    socksport = checkPort(socksport);
    writelog(LOG_TYPE_INFO, "Using local port: " + to_string(socksport));
    writelog(LOG_TYPE_INFO, "Init completed.");
    //intro message
    printmsg(SPEEDTEST_MESSAGE_WELCOME, &node, rpcmode);
    getline(cin, link);
    writelog(LOG_TYPE_INFO, "Input data: " + link);
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
    writelog(LOG_TYPE_INFO, "Received Link.");
    if(strfind(link, "vmess://"))
        linktype = SPEEDTEST_MESSAGE_FOUNDVMESS;
    else if(strfind(link, "ss://"))
        linktype = SPEEDTEST_MESSAGE_FOUNDSS;
    else if(strfind(link, "ssr://"))
        linktype = SPEEDTEST_MESSAGE_FOUNDSSR;
    else if(strfind(link, "http://")||strfind(link, "https://"))
        linktype = SPEEDTEST_MESSAGE_FOUNDSUB;
    else if(link == "data:upload")
        linktype = SPEEDTEST_MESSAGE_FOUNDUPD;
    else if(fileExist(link))
        linktype = SPEEDTEST_MESSAGE_FOUNDLOCAL;


    switch(linktype)
    {
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        switch_codepage();
        printmsg(SPEEDTEST_MESSAGE_FOUNDVMESS, &node, rpcmode);
        explodevmess(link, override_conf_port, socksport, &node);
        if(custom_group.size() != 0)
            node.group = custom_group;
        printmsg(SPEEDTEST_MESSAGE_BEGIN, &node, rpcmode);
        singletest(def_test_file, &node);
        writelog(LOG_TYPE_INFO, "Single node test completed.");
        break;
    case SPEEDTEST_MESSAGE_FOUNDSS:
        switch_codepage();
        printmsg(SPEEDTEST_MESSAGE_FOUNDSS, &node, rpcmode);
        explodess(link, ss_libev, override_conf_port, socksport, &node);
        if(custom_group.size() != 0)
            node.group = custom_group;
        printmsg(SPEEDTEST_MESSAGE_BEGIN, &node, rpcmode);
        singletest(def_test_file, &node);
        writelog(LOG_TYPE_INFO, "Single node test completed.");
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        switch_codepage();
        printmsg(SPEEDTEST_MESSAGE_FOUNDSSR, &node, rpcmode);
        explodessr(link, ssr_libev, override_conf_port, socksport, &node);
        if(custom_group.size() != 0)
            node.group = custom_group;
        printmsg(SPEEDTEST_MESSAGE_BEGIN, &node, rpcmode);
        singletest(def_test_file, &node);
        writelog(LOG_TYPE_INFO, "Single node test completed.");
        break;
    case SPEEDTEST_MESSAGE_FOUNDSUB:
        printmsg(SPEEDTEST_MESSAGE_FOUNDSUB, &node, rpcmode);
        if(!rpcmode)
        {
            printmsg(SPEEDTEST_MESSAGE_GROUP, &node, rpcmode);
            getline(cin, strInput);
            if(strInput.size())
            {
                custom_group = GBKToUTF8(strInput);
                writelog(LOG_TYPE_INFO, "Received custom group: " + custom_group);
            }
        }
        switch_codepage();
        writelog(LOG_TYPE_INFO, "Downloading subscription data...");
        printmsg(SPEEDTEST_MESSAGE_FETCHSUB, &node, rpcmode);
        strSub = webGet(link);
        if(strSub.size())
        {
            explodesub(strSub, ss_libev, ssr_libev, override_conf_port, socksport, &nodes, &exclude_remarks, &include_remarks);
            batchtest(nodes);
            writelog(LOG_TYPE_INFO, "Subscription test completed.");
        }
        else
        {
            writelog(LOG_TYPE_ERROR, "Cannot download subscription data.");
            printmsg(SPEEDTEST_ERROR_INVALIDSUB, &node, rpcmode);
        }
        break;
    case SPEEDTEST_MESSAGE_FOUNDLOCAL:
        printmsg(SPEEDTEST_MESSAGE_FOUNDLOCAL, &node, rpcmode);
        if(!rpcmode)
        {
            printmsg(SPEEDTEST_MESSAGE_GROUP, &node, rpcmode);
            getline(cin, strInput);
            if(strInput.size())
            {
                custom_group = GBKToUTF8(strInput);
                writelog(LOG_TYPE_INFO, "Received custom group: " + custom_group);
            }
        }
        switch_codepage();
        writelog(LOG_TYPE_INFO, "Parsing configuration file data...");
        printmsg(SPEEDTEST_MESSAGE_PARSING, &node, rpcmode);
        if(explodeconf(link, override_conf_port, socksport, ss_libev, ssr_libev, &nodes, &exclude_remarks, &include_remarks) == SPEEDTEST_ERROR_UNRECOGFILE)
        {
            printmsg(SPEEDTEST_ERROR_UNRECOGFILE, &node, rpcmode);
            writelog(LOG_TYPE_ERROR, "Invalid configuration file!");
        }
        else
        {
            batchtest(nodes);
        }
        writelog(LOG_TYPE_INFO, "Configuration test completed.");
        break;
    case SPEEDTEST_MESSAGE_FOUNDUPD:
        printmsg(SPEEDTEST_MESSAGE_FOUNDUPD, &node, rpcmode);
        cin.clear();
        //now we should ready to receive a large amount of data from stdin
        getline(cin, filecontent);
        //writelog(LOG_TYPE_RAW, filecontent);
        filecontent = base64_decode(filecontent.substr(filecontent.find(",") + 1));
        writelog(LOG_TYPE_RAW, filecontent);
        writelog(LOG_TYPE_INFO, "Parsing configuration file data...");
        printmsg(SPEEDTEST_MESSAGE_PARSING, &node, rpcmode);
        if(explodeconfcontent(filecontent, override_conf_port, socksport, ss_libev, ssr_libev, &nodes, &exclude_remarks, &include_remarks) == SPEEDTEST_ERROR_UNRECOGFILE)
        {
            printmsg(SPEEDTEST_ERROR_UNRECOGFILE, &node, rpcmode);
            writelog(LOG_TYPE_ERROR, "Invalid configuration file!");
        }
        else
        {
            batchtest(nodes);
        }
        writelog(LOG_TYPE_INFO, "Configuration test completed.");
        break;
    default:
        writelog(LOG_TYPE_ERROR, "No valid link found.");
        printmsg(SPEEDTEST_ERROR_NORECOGLINK, &node, rpcmode);
        //node.linktype = linktype;
    }
    logeof();
    cleartrans();
    //printmsg(SPEEDTEST_MESSAGE_EOF, &node, rpcmode);
    printmsg_with_dict(SPEEDTEST_MESSAGE_EOF, rpcmode, dict, trans);
    sleep(1);
#ifdef _WIN32
    if(!rpcmode)
        system("pause>nul");
    //stop socket library before exit
    WSACleanup();
#endif // _WIN32
    return 0;
}
