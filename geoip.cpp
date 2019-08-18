#include <rapidjson/document.h>

#include "webget.h"
#include "geoip.h"
#include "misc.h"

using namespace rapidjson;

geoIPInfo getGeoIPInfo(string ip, string proxy)
{
    string strRet, address = ip;
    geoIPInfo info;
    Document json;

    if(address == "")
    {
        strRet = webGet("https://api.ip.sb/geoip", proxy);
    }
    else
    {
        if(!isIPv4(address))
        {
            //maybe it is a host name
            address = hostnameToIPv4(ip);
            if(address == "")
            {
                //maybe it's an IPv6 address, restore it
                address = ip;
            }
        }
        strRet = webGet("https://api.ip.sb/geoip/" + address, proxy);
    }
    if(strRet == "")
        return info;

    json.Parse(strRet.data());

    if(json.HasMember("code"))
    {
        if(json["code"].GetInt() == 401)
        {
            return info;
        }
    }
    if(json.HasMember("ip"))
        info.ip = json["ip"].GetString();
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

geoIPInfo getGeoIPInfo(string ip, socks5Proxy proxy)
{
    return getGeoIPInfo(ip, buildSocks5ProxyString(proxy));
}
