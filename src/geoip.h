#ifndef GEOIP_H_INCLUDED
#define GEOIP_H_INCLUDED

#include <string>

struct geoIPInfo
{
    std::string ip;
    std::string country_code;
    std::string country;
    std::string region_code;
    std::string region;
    std::string city;
    std::string postal_code;
    std::string continent_code;
    std::string latitude;
    std::string longitude;
    std::string organization;
    std::string asn;
    std::string timezone;
};

geoIPInfo getGeoIPInfo(const std::string &ip, const std::string &proxy);

#endif // GEOIP_H_INCLUDED
