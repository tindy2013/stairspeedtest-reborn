#ifndef NODEINFO_H_INCLUDED
#define NODEINFO_H_INCLUDED

#include <string>
#include <future>

#include "geoip.h"

class GeoIPHelper
{
public:

    void set(std::shared_future<geoIPInfo> future)
    {
        _priv_inter_future = future;
        _priv_set = true;
    }

    geoIPInfo get()
    {
        if(!_priv_set)
            return geoIPInfo();
        if(!_priv_fetched)
        {
            _priv_inter_store = _priv_inter_future.get();
            _priv_fetched = true;
        }
        return _priv_inter_store;
    }

    GeoIPHelper()
    {
        _priv_fetched = false;
    }

    GeoIPHelper(std::shared_future<geoIPInfo> future)
    {
        _priv_fetched = false;
        set(future);
    }

    ~GeoIPHelper() = default;

private:
    bool _priv_fetched = false, _priv_set = false;
    std::shared_future<geoIPInfo> _priv_inter_future = std::future<geoIPInfo>();
    geoIPInfo _priv_inter_store;
};

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
    GeoIPHelper inboundGeoIP;
    GeoIPHelper outboundGeoIP;
    std::string testFile;
    std::string ulTarget;
};

#endif // NODEINFO_H_INCLUDED
