#include "misc.h"
#include "socket.h"
#include "printout.h"

using namespace std;
using namespace std::chrono;

const int times_to_ping = 6;

int tcping(nodeInfo *node)
{
    int retVal;

    string host, addr;
    int port;

    host = node->server;
    port = node->port;

    char* addrstr = {};
    if(!regMatch(host, "\\d+\\.\\d+\\.\\d+\\.\\d+") && !strFind(host,":"))
    {
        addrstr = hostnameToIPv4(host, port);
        if(strlen(addrstr) == 0)
            return SPEEDTEST_ERROR_NORESOLVE;
        addr = addrstr;
    }
    else
        addr = host;

    int loopcounter = 0, succeedcounter = 0, failcounter = 0, totduration = 0;
    //simpleSend(addr, port, "."); //establish connection
    while ((loopcounter < times_to_ping))
    {
        auto start = steady_clock::now();
        retVal = simpleSend(addr, port, ".");
        auto end = steady_clock::now();
        if(retVal != 1)
        {
            succeedcounter++;
            auto duration = duration_cast<milliseconds>(end - start);
            int deltatime = duration.count();
            node->rawPing[loopcounter] = deltatime;
            totduration += deltatime;
        }
        else
        {
            failcounter++;
            node->rawPing[loopcounter] = 0;
        }
        loopcounter++;
        sleep(1000);
    }
    float pingval = 0.0;
    if(succeedcounter > 0)
        pingval = totduration * 1.0 / succeedcounter;
    char strtmp[16] = {};
    float pkLoss = failcounter * 100.0 / times_to_ping;
    snprintf(strtmp, sizeof(strtmp), "%0.2f", pkLoss);
    node->pkLoss = string(strtmp) + string("%");
    snprintf(strtmp, sizeof(strtmp), "%0.2f", pingval);
    node->avgPing = string(strtmp);
    return SPEEDTEST_MESSAGE_GOTPING;
}
