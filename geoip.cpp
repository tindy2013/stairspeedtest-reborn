#include <rapidjson/document.h>

#include "webget.h"
#include "geoip.h"
#include "misc.h"
#include "logger.h"

using namespace rapidjson;

geoIPInfo getGeoIPInfo(string ip, string proxy)
{
    writeLog(LOG_TYPE_GEOIP, "GeoIP parse begin.");
    string strRet, address = ip;
    geoIPInfo info;
    Document json;

    if(address == "")
    {
        writeLog(LOG_TYPE_GEOIP, "No address provided, getting GeoIP through proxy '" + proxy + "'.");
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
        writeLog(LOG_TYPE_GEOIP, "Getting GeoIP of '" + address + "' through proxy '" + proxy + "'.");
        strRet = webGet("https://api.ip.sb/geoip/" + address, proxy);
    }
    if(strRet == "")
    {
        writeLog(LOG_TYPE_GEOIP, "No GeoIP result. Leaving...");
        return info;
    }

    writeLog(LOG_TYPE_GEOIP, "Parsing GeoIP result...");
    json.Parse(strRet.data());

    if(json.HasMember("code"))
    {
        if(json["code"].GetInt() == 401)
        {
            writeLog(LOG_TYPE_GEOIP, "Invalid address.");
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

    writeLog(LOG_TYPE_GEOIP, "Parse GeoIP complete. Leaving...");
    return info;
}

geoIPInfo getGeoIPInfo(string ip, socks5Proxy proxy)
{
    return getGeoIPInfo(ip, buildSocks5ProxyString(proxy));
}
