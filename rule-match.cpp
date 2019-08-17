#include "rule-match.h"

using namespace rapidjson;

geoIPInfo getGeoIPInfo(string ip, string proxy)
{
    string strRet;
    geoIPInfo info;
    Document json;

    if(ip == "")
    {
        strRet = webGet("https://api.ip.sb/geoip", proxy);
    }
    else
    {
        strRet = webGet("https://api.ip.sb/geoip/" + ip);
    }
    if(strRet == "")
        return info;

    json.Parse(strRet.data());

    if(json.HasMember("country_code"))
        info.country_code = json["country_code"].GetString();
    if(json.HasMember("country"))
        info.country = json["country"].GetString();
    if(json.HasMember("region_code"))
        info.region_code = json["region_code"].GetString();
    if(json.HasMember("region"))
        info.region = json["region"].GetString();
    if(json.HasMember("city"))
        info.city = json["city"].GetString();
    if(json.HasMember("postal_code"))
        info.postal_code = json["postal_code"].GetString();
    if(json.HasMember("continent_code"))
        info.continent_code = json["continent_code"].GetString();
    if(json.HasMember("latitude"))
        info.latitude = to_string(json["latitude"].GetFloat());
    if(json.HasMember("longitude"))
        info.longitude = to_string(json["longitude"].GetFloat());
    if(json.HasMember("organization"))
        info.organization = json["organization"].GetString();
    if(json.HasMember("asn"))
        info.asn = to_string(json["asn"].GetInt());
    if(json.HasMember("timezone"))
        info.timezone = json["timezone"].GetString();

    return info;
}
