#include <iostream>
#include <chrono>

#include "misc.h"
#include "socket.h"
#include "printout.h"
#include "logger.h"

using namespace std;
using namespace std::chrono;

const int times_to_ping = 6;

void draw_progress_tping(int progress, int values[6])
{
    cerr<<"\r[";
    for(int i = 0; i <= progress; i++)
    {
        cerr<<(values[i] == 0 ? "*" : "-");
    }
    if(progress == times_to_ping - 1)
    {
        cerr<<"]";
    }
    cerr<<" "<<progress + 1<<"/"<<times_to_ping<<" "<<values[progress]<<"ms";
}

int tcping(nodeInfo *node)
{
    writeLog(LOG_TYPE_TCPING, "TCP Ping begin.");
    int retVal;

    string host, addr, addrstr;
    int port;

    host = node->server;
    port = node->port;
    writeLog(LOG_TYPE_TCPING, "Ping target: " + host + ":" + to_string(port) + ".");

    bool IPv6 = isIPv6(host);

    if(!isIPv4(host) && !IPv6)
    {
        writeLog(LOG_TYPE_TCPING, "Host name provided. Resolving into IP address.");
        addr = hostnameToIPAddr(host);
        if(addr == "")
            return SPEEDTEST_ERROR_NORESOLVE;
    }
    else
    {
        writeLog(LOG_TYPE_TCPING, "IP address provided. Skip resolve.");
        addr = host;
    }

    addrstr = IPv6 ? "[" + addr + "]" : addr;

    writeLog(LOG_TYPE_TCPING, "Start probing " + addrstr + ":" + to_string(port) + ".");
    int loopcounter = 0, succeedcounter = 0, failcounter = 0, totduration = 0;
    //simpleSend(addr, port, "."); //establish connection
    while((loopcounter < times_to_ping))
    {
        auto start = steady_clock::now();
        retVal = simpleSend(addr, port, ".");
        auto end = steady_clock::now();
        auto duration = duration_cast<milliseconds>(end - start);
        int deltatime = duration.count();
        if(retVal != 1)
        {
            succeedcounter++;
            node->rawPing[loopcounter] = deltatime;
            totduration += deltatime;
            writeLog(LOG_TYPE_TCPING, "Probing " + addrstr + ":" + to_string(port) + "/tcp - Port is open - time=" + to_string(deltatime) + "ms");
        }
        else
        {
            failcounter++;
            node->rawPing[loopcounter] = 0;
            writeLog(LOG_TYPE_TCPING, "Probing " + addrstr + ":" + to_string(port) + "/tcp - No response - time=" + to_string(deltatime) + "ms");
        }
        draw_progress_tping(loopcounter, node->rawPing);
        loopcounter++;
        if(loopcounter < times_to_ping)
        {
            if(retVal != 1)
                sleep(1000); //passed, sleep longer
            else
                sleep(200); //not passed, sleep shorter
        }

    }
    cerr<<endl;
    float pingval = 0.0;
    if(succeedcounter > 0)
        pingval = totduration * 1.0 / succeedcounter;
    char strtmp[16] = {};
    float pkLoss = failcounter * 100.0 / times_to_ping;
    snprintf(strtmp, sizeof(strtmp), "%0.2f%%", pkLoss);
    node->pkLoss.assign(strtmp);
    snprintf(strtmp, sizeof(strtmp), "%0.2f", pingval);
    node->avgPing.assign(strtmp);
    writeLog(LOG_TYPE_TCPING, "Ping statistics for " + addrstr + ":" + to_string(port) + " : " \
             + to_string(loopcounter) + " probes sent, " + to_string(succeedcounter) + " successful, " + to_string(failcounter) + " failed. " \
             + "(" + node->pkLoss + " fail)");
    return SPEEDTEST_MESSAGE_GOTPING;
}
