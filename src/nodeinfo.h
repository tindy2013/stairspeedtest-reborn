#ifndef NODEINFO_H_INCLUDED
#define NODEINFO_H_INCLUDED

#include <string>

#include "geoip.h"

struct nodeInfo
{
    int linkType = -1;
    int id = -1;
    int groupID = -1;
    bool online = false;
    std::string group;
    std::string remarks;
    std::string server;
    int port = 0;
    std::string proxyStr;
    long long rawSpeed[20] = {};
    long long totalRecvBytes = 0;
    int duration = 0;
    std::string avgSpeed = "N/A";
    std::string maxSpeed = "N/A";
    std::string ulSpeed = "N/A";
    std::string pkLoss = "100.00%";
    int rawPing[6] = {};
    std::string avgPing = "0.00";
    int rawSitePing[10] = {};
    std::string sitePing = "0.00";
    std::string traffic;
    geoIPInfo inboundGeoIP;
    geoIPInfo outboundGeoIP;
    std::string testFile;
    std::string ulTarget;
};

#endif // NODEINFO_H_INCLUDED
