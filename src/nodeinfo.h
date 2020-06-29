#ifndef NODEINFO_H_INCLUDED
#define NODEINFO_H_INCLUDED

#include <string>
#include <future>

#include "geoip.h"

template <typename T> class FutureHelper
{
public:

    void set(std::shared_future<T> future)
    {
        _priv_inter_future = future;
        _priv_fetched = false;
        _priv_set = true;
    }

    void set(T content)
    {
        _priv_inter_store = content;
        _priv_fetched = true;
        _priv_set = true;
    }

    T get()
    {
        if(!_priv_set)
            return T();
        if(!_priv_fetched)
        {
            _priv_inter_store = _priv_inter_future.get();
            _priv_fetched = true;
        }
        return _priv_inter_store;
    }

    FutureHelper()
    {
        _priv_fetched = false;
    }

    FutureHelper(std::shared_future<T>&& future)
    {
        set(future);
    }

    FutureHelper(T&& content)
    {
        set(content);
    }

    FutureHelper& operator= (std::shared_future<T>&& future)
    {
        set(future);
        return *this;
    }

    FutureHelper& operator= (T&& content)
    {
        set(content);
        return *this;
    }

    ~FutureHelper() = default;

private:
    bool _priv_fetched = false, _priv_set = false;
    std::shared_future<T> _priv_inter_future = std::future<T>();
    T _priv_inter_store;
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
    FutureHelper<geoIPInfo> inboundGeoIP;
    FutureHelper<geoIPInfo> outboundGeoIP;
    std::string testFile;
    std::string ulTarget;
    FutureHelper<std::string> natType {"Unknown"};
};

#endif // NODEINFO_H_INCLUDED
