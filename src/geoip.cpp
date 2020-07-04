#include "webget.h"
#include "geoip.h"
#include "misc.h"
#include "logger.h"
#include "rapidjson_extra.h"
#include "socket.h"

using namespace rapidjson;

geoIPInfo getGeoIPInfo(const std::string &ip, const std::string &proxy)
{
    writeLog(LOG_TYPE_GEOIP, "GeoIP parse begin.");
    std::string strRet, address = ip;
    geoIPInfo info;
    Document json;

    if(address.empty())
    {
        writeLog(LOG_TYPE_GEOIP, "No address provided, getting GeoIP through proxy '" + proxy + "'.");
        strRet = webGet("https://api.ip.sb/geoip", proxy);
    }
    else
    {
        if(!isIPv4(address))
        {
            if(!isIPv6(address))
            {
                writeLog(LOG_TYPE_GEOIP, "Found host name. Resolving into IP address.");
                address = hostnameToIPAddr(ip);
                if(address.empty())
                {
                    writeLog(LOG_TYPE_GEOIP, "Host name resolve error. Leaving.");
                    return info;
                }
            }
            else
                writeLog(LOG_TYPE_GEOIP, "Found IPv6 address.");
        }
        else
            writeLog(LOG_TYPE_GEOIP, "Found IPv4 address.");
        writeLog(LOG_TYPE_GEOIP, "Getting GeoIP of '" + address + "' through proxy '" + proxy + "'.");
        strRet = webGet("https://api.ip.sb/geoip/" + address, proxy);
    }
    if(strRet.empty())
    {
        writeLog(LOG_TYPE_GEOIP, "No GeoIP result. Leaving.");
        return info;
    }

    writeLog(LOG_TYPE_GEOIP, "Fetched GeoIP data: " + strRet);
    writeLog(LOG_TYPE_GEOIP, "Parsing GeoIP result.");
    json.Parse(strRet.data());
    if(json.HasParseError())
    {
        writeLog(LOG_TYPE_GEOIP, "Parse GeoIP result error. Leaving.");
        return info;
    }

    if(json.HasMember("code"))
    {
        if(json["code"].GetInt() == 401)
        {
            writeLog(LOG_TYPE_GEOIP, "Invalid address. Leaving.");
            return info;
        }
    }
    info.ip = GetMember(json, "ip");
    info.country_code = GetMember(json, "country_code");
    info.country = GetMember(json, "country");
    info.region_code = GetMember(json, "region_code");
    info.region = GetMember(json, "region");
    info.city = GetMember(json, "city");
    info.postal_code = GetMember(json, "postal_code");
    info.continent_code = GetMember(json, "continent_code");
    info.latitude = GetMember(json, "latitude");
    info.longitude = GetMember(json, "longitude");
    info.organization = GetMember(json, "organization");
    info.asn = GetMember(json, "asn");
    info.timezone = GetMember(json, "timezone");

    writeLog(LOG_TYPE_GEOIP, "Parse GeoIP complete. Leaving.");
    return info;
}
