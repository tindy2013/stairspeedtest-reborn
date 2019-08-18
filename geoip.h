#ifndef GEOIP_H_INCLUDED
#define GEOIP_H_INCLUDED

#include <string>

#include "socket.h"

using namespace std;

struct geoIPInfo
{
    string ip;
    string country_code;
    string country;
    string region_code;
    string region;
    string city;
    string postal_code;
    string continent_code;
    string latitude;
    string longitude;
    string organization;
    string asn;
    string timezone;
};

geoIPInfo getGeoIPInfo(string ip, string proxy);
geoIPInfo getGeoIPInfo(string ip, socks5Proxy proxy);

#endif // GEOIP_H_INCLUDED
