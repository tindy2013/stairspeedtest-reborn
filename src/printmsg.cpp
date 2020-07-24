#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "printout.h"
#include "version.h"

//define print-out messages
struct LOOKUP_ITEM
{
    int index;
    std::string info;
};

LOOKUP_ITEM SPEEDTEST_MESSAGES[] =
{
    {SPEEDTEST_MESSAGE_EOF, "\nSpeed Test done. Press any key to exit..."},
    {SPEEDTEST_MESSAGE_WELCOME, "Welcome to Stair Speedtest " VERSION "!\nWhich stair do you want to test today? (Supports single Shadowsocks/ShadowsocksD/ShadowsocksR/V2Ray link and their subscribe links)\nIf you want to test more than one link, separate them with '|'.\nLink: "},
    {SPEEDTEST_MESSAGE_MULTILINK, "Multiple link provided, parsing all nodes.\n\n"},
    {SPEEDTEST_MESSAGE_FOUNDVMESS, "Found single V2Ray link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDSS, "Found single Shadowsocks link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDSSR, "Found single ShadowsocksR link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDTROJAN, "Found single Trojan link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDSOCKS, "Found single Socks 5 link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDNETCH, "Found single Netch link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDSUB, "Found subscribe link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDLOCAL, "Found local configure file.\n"},
    {SPEEDTEST_MESSAGE_GROUP, "If you have imported an V2Ray subscribe link which doesn't contain a Group Name, you can specify a custom name below.\nIf you have imported an Shadowsocks/ShadowsocksR link which contains a Group Name, press Enter to skip.\nCustom Group Name: "},
    {SPEEDTEST_MESSAGE_GOTSERVER, "\nCurrent Server Group: ?1? Remarks: ?2? Index: ?0?/?3?\n"},
    {SPEEDTEST_MESSAGE_STARTPING, "Now performing TCP Ping...\n"},
    {SPEEDTEST_MESSAGE_STARTGEOIP, "Now performing GeoIP parse...\n"},
    {SPEEDTEST_MESSAGE_STARTGPING, "Now performing Google Ping...\n"},
    {SPEEDTEST_MESSAGE_STARTSPEED, "Now performing Speed Test...\n"},
    {SPEEDTEST_MESSAGE_STARTUPD, "Now performing Upload Test...\n"},
    {SPEEDTEST_MESSAGE_GOTRESULT, "Result: DL.Speed: ?0? Max.Speed: ?1? UL.Speed: ?2? Pk.Loss: ?3? Avg.Ping: ?4? Google Ping: ?5? NAT Type: ?6?\n"},
    {SPEEDTEST_MESSAGE_TRAFFIC, "Traffic used: ?traffic?\n"},
    {SPEEDTEST_MESSAGE_PICSAVING, "Now exporting picture...\n"},
    {SPEEDTEST_MESSAGE_PICSAVINGMULTI, "Now exporting picture for group ?0?...\n"},
    {SPEEDTEST_MESSAGE_PICSAVED, "Result picture saved to \"?0?\".\n"},
    {SPEEDTEST_MESSAGE_PICSAVEDMULTI, "Group ?0? result picture saved to \"?1?\".\n"},
    {SPEEDTEST_MESSAGE_FETCHSUB, "Downloading subscription data...\n"},
    {SPEEDTEST_MESSAGE_PARSING, "Parsing configuration file...\n"},
    {SPEEDTEST_MESSAGE_BEGIN, "Speed Test will now begin.\n"},
    {SPEEDTEST_MESSAGE_GOTGEOIP, "Parsed outbound server ISP: ?1?  Country Code: ?2?\n"},
    {SPEEDTEST_MESSAGE_STARTNAT, "Now performing UDP NAT Type test...\n"},
    {SPEEDTEST_MESSAGE_GOTNAT, "UDP NAT Type test result: ?1?\n"},
    {SPEEDTEST_ERROR_UNDEFINED, "Undefined error!\n"},
    {SPEEDTEST_ERROR_WSAERR, "WSA Startup error!\n"},
    {SPEEDTEST_ERROR_SOCKETERR, "Socket error!\n"},
    {SPEEDTEST_ERROR_NORECOGLINK, "No valid link found. Please check your link.\n"},
    {SPEEDTEST_ERROR_UNRECOGFILE, "This configure file is invalid. Please make sure this is an Shadowsocks/ShadowsocksR/v2rayN configuration file or a standard subscription file.\n"},
    {SPEEDTEST_ERROR_NOCONNECTION, "Cannot connect to server.\n"},
    {SPEEDTEST_ERROR_INVALIDSUB, "Nothing returned from subscribe link. Please check your subscribe link.\n"},
    {SPEEDTEST_ERROR_NONODES, "No nodes found. Please check your subscribe link.\n"},
    {SPEEDTEST_ERROR_NORESOLVE, "Cannot resolve server address.\n"},
    {SPEEDTEST_ERROR_RETEST, "Speed Test returned no speed. Retesting...\n"},
    {SPEEDTEST_ERROR_NOSPEED, "Speed Test returned no speed 2 times. Skipping...\n"},
    {SPEEDTEST_ERROR_SUBFETCHERR, "Cannot fetch subscription data with direct connect. Trying with system proxy...\n"},
    {SPEEDTEST_ERROR_GEOIPERR, "Cannot fetch GeoIP information. Skipping...\n"}
};

LOOKUP_ITEM SPEEDTEST_MESSAGES_RPC[] =
{
    {SPEEDTEST_MESSAGE_WELCOME, "{\"info\":\"started\"}\n"},
    {SPEEDTEST_MESSAGE_EOF, "{\"info\":\"eof\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDVMESS, "{\"info\":\"foundvmess\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDSS, "{\"info\":\"foundss\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDSSR, "{\"info\":\"foundssr\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDTROJAN, "{\"info\":\"foundtrojan\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDSOCKS, "{\"info\":\"foundsocks\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDNETCH, "\"info\":\"foundnetch\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDSUB, "{\"info\":\"foundsub\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDLOCAL, "{\"info\":\"foundlocal\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDUPD, "{\"info\":\"foundupd\"}\n"},
    {SPEEDTEST_MESSAGE_GOTSERVER, "{\"info\":\"gotserver\",\"id\":?0?,\"group\":\"?1?\",\"remarks\":\"?2?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTPING, "{\"info\":\"startping\",\"id\":?0?}\n"},
    {SPEEDTEST_MESSAGE_GOTPING, "{\"info\":\"gotping\",\"id\":?0?,\"ping\":\"?1?\",\"loss\":\"?2?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTGEOIP, "{\"info\":\"startgeoip\",\"id\":?0?}\n"},
    {SPEEDTEST_MESSAGE_GOTGEOIP, "{\"info\":\"gotgeoip\",\"id\":?0?,\"isp\":\"?1?\",\"location\":\"?2?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTNAT, "{\"info\":\"startnat\",\"id\":?0?}\n"},
    {SPEEDTEST_MESSAGE_GOTNAT, "{\"info\":\"gotnat\",\"id\":?0?,\"result\":\"?1?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTSPEED, "{\"info\":\"startspeed\",\"id\":?0?}\n"},
    {SPEEDTEST_MESSAGE_GOTSPEED, "{\"info\":\"gotspeed\",\"id\":?0?,\"speed\":\"?1?\",\"maxspeed\":\"?2?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTUPD, "{\"info\":\"startupd\",\"id\":?0?}\n"},
    {SPEEDTEST_MESSAGE_GOTUPD, "{\"info\":\"gotupd\",\"id\":?0?,\"ulspeed\":\"?1?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTGPING, "{\"info\":\"startgping\",\"id\":?0?}\n"},
    {SPEEDTEST_MESSAGE_GOTGPING, "{\"info\":\"gotgping\",\"id\":?0?,\"ping\":\"?1?\"}\n"},
    {SPEEDTEST_MESSAGE_TRAFFIC, "(\"info\":\"traffic\",\"size\":\"?0?\"}\n"},
    {SPEEDTEST_MESSAGE_PICSAVING, "{\"info\":\"picsaving\"}\n"},
    {SPEEDTEST_MESSAGE_PICSAVED, "{\"info\":\"picsaved\",\"path\":\"?0?\"}\n"},
    {SPEEDTEST_MESSAGE_PICSAVEDMULTI, "{\"info\":\"picsaved\",\"path\":\"?0?\"}\n"},
    {SPEEDTEST_MESSAGE_FETCHSUB, "{\"info\":\"fetchingsub\"}\n"},
    {SPEEDTEST_MESSAGE_PARSING, "{\"info\":\"parsing\"}\n"},
    {SPEEDTEST_MESSAGE_BEGIN, "{\"info\":\"begintest\"}\n"},
    {SPEEDTEST_MESSAGE_PICDATA, "{\"info\":\"picdata\",\"data\":\"?0?\"}\n"},
    {SPEEDTEST_ERROR_UNDEFINED, "{\"info\":\"error\",\"reason\":\"undef\"}\n"},
    {SPEEDTEST_ERROR_WSAERR, "{\"info\":\"error\",\"reason\":\"wsaerr\"}\n"},
    {SPEEDTEST_ERROR_SOCKETERR, "{\"info\":\"error\",\"reason\":\"socketerr\"}\n"},
    {SPEEDTEST_ERROR_NORECOGLINK, "{\"info\":\"error\",\"reason\":\"norecoglink\"}\n"},
    {SPEEDTEST_ERROR_UNRECOGFILE, "{\"info\":\"error\",\"reason\":\"unrecogfile\"}\n"},
    {SPEEDTEST_ERROR_NOCONNECTION, "{\"info\":\"error\",\"reason\":\"noconnection\",\"id\":?0?}\n"},
    {SPEEDTEST_ERROR_INVALIDSUB, "{\"info\":\"error\",\"reason\":\"invalidsub\"}\n"},
    {SPEEDTEST_ERROR_NONODES, "{\"info\":\"error\",\"reason\":\"nonodes\"}\n"},
    {SPEEDTEST_ERROR_NORESOLVE, "{\"info\":\"error\",\"reason\":\"noresolve\",\"id\":?0?}\n"},
    {SPEEDTEST_ERROR_RETEST, "{\"info\":\"error\",\"reason\":\"retest\",\"id\":?0?}\n"},
    {SPEEDTEST_ERROR_NOSPEED, "{\"info\":\"error\",\"reason\":\"nospeed\",\"id\":?0?}\n"},
    {SPEEDTEST_ERROR_SUBFETCHERR, "{\"info\":\"error\",\"reason\":\"subfetcherr\"}\n"},
    {SPEEDTEST_ERROR_GEOIPERR, "{\"info\":\"error\",\"reason\":\"geoiperr\",\"id\":?0?}\n"}
};

std::string lookUp(int index, LOOKUP_ITEM *items)
{
    int i = 0;
    while (0 <= items[i].index)
    {
        if (items[i].index == index)
            return items[i].info;
        i++;
    }
    return std::string("");
}

std::string lookUp(int index, bool rpcmode)
{
    if(rpcmode)
        return lookUp(index, SPEEDTEST_MESSAGES_RPC);
    else
        return lookUp(index, SPEEDTEST_MESSAGES);
}

/*
void printMsg(int index, nodeInfo &node, bool rpcmode)
{
    std::string printout;
    printout = lookUp(index, rpcmode);
    if(printout.size() == 0)
    {
        return;
    }
    printout = replace_all_distinct(printout, "?group?", trim(node.group));
    printout = replace_all_distinct(printout, "?remarks?", trim(node.remarks));
    printout = replace_all_distinct(printout, "?id?", std::to_string(node.id));
    printout = replace_all_distinct(printout, "?avgping?", node.avgPing);
    printout = replace_all_distinct(printout, "?pkloss?", node.pkLoss);
    printout = replace_all_distinct(printout, "?siteping?", node.sitePing);
    printout = replace_all_distinct(printout, "?speed?", node.avgSpeed);
    printout = replace_all_distinct(printout, "?maxspeed?", node.maxSpeed);
    printout = replace_all_distinct(printout, "?ulspeed?", node.ulSpeed);
    printout = replace_all_distinct(printout, "?traffic?", node.traffic);
    if(rpcmode)
        printout = replace_all_distinct(printout, "\\", "\\\\");
    std::cout<<printout;
    std::cout.clear();
    std::cout.flush();
}

void printMsgWithDict(int index, bool rpcmode, std::vector<std::string> dict, std::vector<std::string> trans)
{
    std::string printout;
    printout = lookUp(index, rpcmode);
    if(printout.size() == 0)
    {
        return;
    }
    for(unsigned int i = 0; i < dict.size(); i++)
    {
        printout = replace_all_distinct(printout, dict[i], trans[i]);
    }
    if(rpcmode)
        printout = replace_all_distinct(printout, "\\", "\\\\");
    std::cout<<printout;
    std::cout.clear();
    std::cout.flush();
}

void printMsgDirect(int index, bool rpcmode)
{
    std::cout<<lookUp(index, rpcmode);
    std::cout.clear();
    std::cout.flush();
}
*/
