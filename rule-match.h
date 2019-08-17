#ifndef RULEMATCH_H_INCLUDED
#define RULEMATCH_H_INCLUDED

#include <bits/stdc++.h>
#include <rapidjson/document.h>

#include "webget.h"

using namespace std;

struct geoIPInfo
{
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

#endif // RULEMATCH_H_INCLUDED
