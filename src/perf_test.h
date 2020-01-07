#ifndef PERF_TEST_H_INCLUDED
#define PERF_TEST_H_INCLUDED
#include "socket.h"
#include "misc.h"

struct nodePerfInfo
{
    int linkType = -1;
    int id = 0;
    int groupID = 0;
    bool online = false;
    std::string group;
    std::string remarks;
    std::string server;
    int port = 0;
    std::string proxyStr;
    std::string pkLoss = "100.00%";
    int rawPing[6] = {};
    std::string avgPing = "0.00";
    int rawSitePing[3] = {};
    std::string sitePing = "0.00";
    int rawTelegramPing[3] = {};
    std::string telegramPing = "0.00";
    int rawCloudflarePing[3] = {};
    std::string cloudflarePing = "0.00";
    geoIPInfo inboundGeoIP;
    geoIPInfo outboundGeoIP;
};

void testTelegram(std::string localaddr, int localport, nodePerfInfo *node);
void testCloudflare(std::string localaddr, int localport, nodePerfInfo *node);

#endif // PERF_TEST_H_INCLUDED
