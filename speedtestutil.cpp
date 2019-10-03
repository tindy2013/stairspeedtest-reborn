#include <fstream>
#include <algorithm>

#include <rapidjson/document.h>

#include "misc.h"
#include "printout.h"
#include "logger.h"
#include "speedtestutil.h"
#include "webget.h"
#include "rapidjson_extra.h"
#include "ini_reader.h"

using namespace std;
using namespace rapidjson;
using namespace YAML;

string base_ss_win = "{\"version\":\"4.1.6\",\"configs\":[?config?],\"strategy\":null,\"index\":0,\"global\":false,\"enabled\":false,\"shareOverLan\":true,\"isDefault\":false,\"localPort\":?localport?,\"portableMode\":true,\"pacUrl\":null,\"useOnlinePac\":false,\"secureLocalPac\":true,\"availabilityStatistics\":false,\"autoCheckUpdate\":true,\"checkPreRelease\":false,\"isVerboseLogging\":false,\"logViewer\":{\"topMost\":false,\"wrapText\":false,\"toolbarShown\":false,\"Font\":\"Consolas, 8pt\",\"BackgroundColor\":\"Black\",\"TextColor\":\"White\"},\"proxy\":{\"useProxy\":false,\"proxyType\":0,\"proxyServer\":\"\",\"proxyPort\":0,\"proxyTimeout\":3,\"useAuth\":false,\"authUser\":\"\",\"authPwd\":\"\"},\"hotkey\":{\"SwitchSystemProxy\":\"\",\"SwitchSystemProxyMode\":\"\",\"SwitchAllowLan\":\"\",\"ShowLogs\":\"\",\"ServerMoveUp\":\"\",\"ServerMoveDown\":\"\",\"RegHotkeysAtStartup\":false}}";
string config_ss_win = "{\"server\":\"?server?\",\"server_port\":?port?,\"password\":\"?password?\",\"method\":\"?method?\",\"plugin\":\"?plugin?\",\"plugin_opts\":\"?plugin_opts?\",\"plugin_args\":\"\",\"remarks\":\"?remarks?\",\"timeout\":5}";
string config_ss_libev = "{\"server\":\"?server?\",\"server_port\":?port?,\"password\":\"?password?\",\"method\":\"?method?\",\"plugin\":\"?plugin?\",\"plugin_opts\":\"?plugin_opts?\",\"plugin_args\":\"\",\"local_address\":\"127.0.0.1\",\"local_port\":?localport?}";
string base_ssr_win = "{\"configs\":[?config?],\"index\":0,\"random\":true,\"sysProxyMode\":1,\"shareOverLan\":false,\"localPort\":?localport?,\"localAuthPassword\":null,\"localDnsServer\":\"\",\"dnsServer\":\"\",\"reconnectTimes\":2,\"balanceAlgorithm\":\"LowException\",\"randomInGroup\":false,\"TTL\":0,\"connectTimeout\":5,\"proxyRuleMode\":2,\"proxyEnable\":false,\"pacDirectGoProxy\":false,\"proxyType\":0,\"proxyHost\":null,\"proxyPort\":0,\"proxyAuthUser\":null,\"proxyAuthPass\":null,\"proxyUserAgent\":null,\"authUser\":null,\"authPass\":null,\"autoBan\":false,\"checkSwitchAutoCloseAll\":false,\"logEnable\":false,\"sameHostForSameTarget\":false,\"keepVisitTime\":180,\"isHideTips\":false,\"nodeFeedAutoUpdate\":true,\"serverSubscribes\":[],\"token\":{},\"portMap\":{}}";
string config_ssr_win = "{\"remarks\":\"?remarks?\",\"id\":\"18C4949EBCFE46687AE4A7645725D35F\",\"server\":\"?server?\",\"server_port\":?port?,\"server_udp_port\":0,\"password\":\"?password?\",\"method\":\"?method?\",\"protocol\":\"?protocol?\",\"protocolparam\":\"?protoparam?\",\"obfs\":\"?obfs?\",\"obfsparam\":\"?obfsparam?\",\"remarks_base64\":\"?remarks_base64?\",\"group\":\"?group?\",\"enable\":true,\"udp_over_tcp\":false}";
string config_ssr_libev = "{\"server\":\"?server?\",\"server_port\":?port?,\"protocol\":\"?protocol?\",\"method\":\"?method?\",\"obfs\":\"?obfs?\",\"password\":\"?password?\",\"obfs_param\":\"?obfsparam?\",\"protocol_param\":\"?protoparam?\",\"local_address\":\"127.0.0.1\",\"local_port\":?localport?}";
string base_vmess = "{\"inbounds\":[{\"port\":?localport?,\"listen\":\"0.0.0.0\",\"protocol\":\"socks\"}],\"outbounds\":[{\"tag\":\"proxy\",\"protocol\":\"vmess\",\"settings\":{\"vnext\":[{\"address\":\"?add?\",\"port\":?port?,\"users\":[{\"id\":\"?id?\",\"alterId\":?aid?,\"email\":\"t@t.tt\",\"security\":\"?cipher?\"}]}]},\"streamSettings\":{\"network\":\"?net?\",\"security\":\"?tls?\",\"tlsSettings\":?tlsset?,\"tcpSettings\":?tcpset?,\"wsSettings\":?wsset?},\"mux\":{\"enabled\":true}}],\"routing\":{\"domainStrategy\":\"IPIfNonMatch\"}}";
string wsset_vmess = "{\"connectionReuse\":true,\"path\":\"?path?\",\"headers\":{\"Host\":\"?host?\"}}";
string tcpset_vmess = "{\"connectionReuse\":true,\"header\":{\"type\":\"?type?\",\"request\":{\"version\":\"1.1\",\"method\":\"GET\",\"path\":[\"?path?\"],\"headers\":{\"Host\":[\"?host?\"],\"User-Agent\":[\"Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.75 Safari/537.36\",\"Mozilla/5.0 (iPhone; CPU iPhone OS 10_0_2 like Mac OS X) AppleWebKit/601.1 (KHTML, like Gecko) CriOS/53.0.2785.109 Mobile/14A456 Safari/601.1.46\"],\"Accept-Encoding\":[\"gzip, deflate\"],\"Connection\":[\"keep-alive\"],\"Pragma\":\"no-cache\"}}}}";
string tlsset_vmess = "{\"serverName\":\"?serverName?\",\"allowInsecure\":false,\"allowInsecureCiphers\":false}";

vector<string> ss_ciphers = {"rc4-md5","aes-128-gcm","aes-192-gcm","aes-256-gcm","aes-128-cfb","aes-192-cfb","aes-256-cfb","aes-128-ctr","aes-192-ctr","aes-256-ctr","camellia-128-cfb","camellia-192-cfb","camellia-256-cfb","bf-cfb","chacha20-ietf-poly1305","xchacha20-ietf-poly1305","salsa20","chacha20","chacha20-ietf"};

map<string, string> parsedMD5;
string modSSMD5 = "f7653207090ce3389115e9c88541afe0";

//remake from speedtestutil

template <typename T> void operator >> (const YAML::Node& node,  T& i)
{
    i = node.as<T>();
};

string vmessConstruct(string add, string port, string type, string id, string aid, string net, string cipher, string path, string host, string tls, int local_port)
{
    string base = base_vmess;
    string wsset = wsset_vmess;
    string tcpset = tcpset_vmess;
    string tlsset = tlsset_vmess;
    base = replace_all_distinct(base, "?localport?", to_string(local_port));
    base = replace_all_distinct(base, "?add?", add);
    base = replace_all_distinct(base, "?port?", port);
    base = replace_all_distinct(base, "?id?", id);
    base = replace_all_distinct(base, "?aid?", aid);
    base = replace_all_distinct(base, "?net?", net);
    base = replace_all_distinct(base, "?cipher?", cipher);
    if(net == "ws")
    {
        wsset = replace_all_distinct(wsset, "?host?", host == "" ? add : host);
        wsset = replace_all_distinct(wsset, "?path?", path == "" ? "/" : path);
        base = replace_all_distinct(base, "?wsset?", wsset);
    }
    if(type == "http")
    {
        tcpset = replace_all_distinct(tcpset, "?host?", host == "" ? add : host);
        tcpset = replace_all_distinct(tcpset, "?type?", type);
        tcpset = replace_all_distinct(tcpset, "?path?", path == "" ? "/" : path);
        base = replace_all_distinct(base, "?tcpset?", tcpset);
    }
    if(host == "" && !isIPv4(add) && !isIPv6(add))
        host = add;
    if(host != "")
    {
        tlsset = replace_all_distinct(tlsset, "?serverName?", host);
        base = replace_all_distinct(base, "?tlsset?", tlsset);
    }

    base = replace_all_distinct(base, "?tls?", tls);
    base = replace_all_distinct(base, "?tcpset?", "null");
    base = replace_all_distinct(base, "?wsset?", "null");
    base = replace_all_distinct(base, "?tlsset?", "null");

    return base;
}

string ssrConstruct(string group, string remarks, string remarks_base64, string server, string port, string protocol, string method, string obfs, string password, string obfsparam, string protoparam, int local_port, bool libev)
{
    string base = base_ssr_win;
    string config = config_ssr_win;
    string config_libev = config_ssr_libev;
    if(libev == true)
        config = config_libev;
    if(isIPv6(server))
        server = "[" + server + "]";
    config = replace_all_distinct(config, "?group?", group);
    config = replace_all_distinct(config, "?remarks?", remarks);
    config = replace_all_distinct(config, "?remarks_base64?", remarks_base64);
    config = replace_all_distinct(config, "?server?", server);
    config = replace_all_distinct(config, "?port?", port);
    config = replace_all_distinct(config, "?protocol?", protocol);
    config = replace_all_distinct(config, "?method?", method);
    config = replace_all_distinct(config, "?obfs?", obfs);
    config = replace_all_distinct(config, "?password?", password);
    config = replace_all_distinct(config, "?obfsparam?", obfsparam);
    config = replace_all_distinct(config, "?protoparam?", protoparam);
    if(libev == true)
        base = config;
    else
        base = replace_all_distinct(base, "?config?", config);
    base = replace_all_distinct(base, "?localport?", to_string(local_port));

    return base;
}

string ssConstruct(string server, string port, string password, string method, string plugin, string pluginopts, string remarks, int local_port, bool libev)
{
    string base = base_ss_win;
    string config = config_ss_win;
    string config_libev = config_ss_libev;
    if(plugin == "obfs-local")
        plugin = "simple-obfs";
    if(libev == true)
        config = config_libev;
    if(isIPv6(server))
        server = "[" + server + "]";
    config = replace_all_distinct(config, "?server?", server);
    config = replace_all_distinct(config, "?port?", port);
    config = replace_all_distinct(config, "?password?", password);
    config = replace_all_distinct(config, "?method?", method);
    config = replace_all_distinct(config, "?plugin?", plugin);
    config = replace_all_distinct(config, "?plugin_opts?", pluginopts);
    config = replace_all_distinct(config, "?remarks?", remarks);
    if(libev == true)
        base = config;
    else
        base = replace_all_distinct(base, "?config?", config);
    base = replace_all_distinct(base, "?localport?", to_string(local_port));

    return base;
}

void explodeVmess(string vmess, string custom_port, int local_port, nodeInfo *node)
{
    string version, ps, add, port, type, id, aid, net, path, host, tls;
    Document jsondata;
    vector<string> vArray;
    vmess = vmess.substr(8);
    vmess = base64_decode(vmess);
    if(regMatch(vmess, "(.*?) = (.*)"))
    {
        explodeQuan(vmess, custom_port, local_port, node);
        return;
    }
    jsondata.Parse(vmess.data());

    version = "1"; //link without version will treat as version 1
    GetMember(jsondata, "v", &version); //try to get version

    jsondata["ps"] >> ps;
    jsondata["add"] >> add;
    jsondata["type"] >> type;
    jsondata["id"] >> id;
    jsondata["aid"] >> aid;
    jsondata["net"] >> net;
    jsondata["tls"] >> tls;
    if(custom_port != "")
        port = custom_port;
    else
        jsondata["port"] >> port;

    jsondata["host"] >> host;
    if(version == "1")
    {
        if(host != "")
        {
            vArray = split(host, ";");
            if(vArray.size() == 2)
            {
                host = vArray[0];
                path = vArray[1];
            }
        }
    }
    else if(version == "2")
    {
        path = GetMember(jsondata, "path");
    }

    node->linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
    node->group = V2RAY_DEFAULT_GROUP;
    node->remarks = ps;
    node->server = add;
    node->port = stoi(port);
    node->proxyStr = vmessConstruct(add, port, type, id, aid, net, "auto", path, host, tls, local_port);
}

void explodeVmessConf(string content, string custom_port, int local_port, bool libev, vector<nodeInfo> *nodes)
{
    nodeInfo node;
    Document json;
    string group, ps, add, port, type, id, aid, net, path, host, tls, cipher, subid;
    int configType;
    map<string, string> subdata;
    map<string, string>::iterator iter;

    json.Parse(content.data());
    //read all subscribe remark as group name
    for(unsigned int i = 0; i < json["subItem"].Size(); i++)
    {
        subdata.insert(pair<string, string>(json["subItem"][i]["id"].GetString(), json["subItem"][i]["remarks"].GetString()));
    }

    for(unsigned int i = 0; i < json["vmess"].Size(); i++)
    {
        if(json["vmess"][i]["address"].IsNull() || json["vmess"][i]["port"].IsNull() || json["vmess"][i]["id"].IsNull())
        {
            continue;
        }
        //common info
        json["vmess"][i]["remarks"] >> ps;
        json["vmess"][i]["address"] >> add;
        if(custom_port != "")
            port = custom_port;
        else
            json["vmess"][i]["port"] >>port;
        json["vmess"][i]["subid"] >> subid;

        json["vmess"][i]["configType"] >> configType;
        switch(configType)
        {
        case 1: //vmess config
            json["vmess"][i]["headerType"] >> type;
            json["vmess"][i]["id"] >> id;
            json["vmess"][i]["alterId"] >> aid;
            json["vmess"][i]["network"] >> net;
            json["vmess"][i]["path"] >> path;
            json["vmess"][i]["requestHost"] >> host;
            json["vmess"][i]["streamSecurity"] >> tls;
            json["vmess"][i]["security"] >> cipher;
            group = V2RAY_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
            node.proxyStr = vmessConstruct(add, port, type, id, aid, net, cipher, path, host, tls, local_port);
            break;
        case 3: //ss config
            json["vmess"][i]["id"] >> id;
            json["vmess"][i]["security"] >> cipher;
            group = SS_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
            node.proxyStr = ssConstruct(add, port, id, cipher, "", "", ps, local_port, true);
            break;
        case 4: //socks config
            group = SOCKS_DEFAULT_GROUP;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
            node.proxyStr = "user=&pass=";
        default:
            continue;
        }
        if(subid != "")
        {
            iter = subdata.find(subid);
            if(iter != subdata.end())
            {
                group = iter->second;
            }
        }
        if(ps == "")
        {
            ps = add + ":" + port;
        }
        node.group = group;
        node.remarks = ps;
        node.server = add;
        node.port = stoi(port);
        nodes->push_back(node);
    }
    return;
}

void explodeSSR(string ssr, bool libev, string custom_port, int local_port, nodeInfo *node)
{
    string strobfs;
    vector<string> strcfg;
    string remarks, group, server, port, method, password, protocol, protoparam, obfs, obfsparam, remarks_base64;
    ssr = replace_all_distinct(ssr.substr(6), "\r", "");
    ssr = urlsafe_base64_decode(ssr);
    if(strFind(ssr, "/?"))
    {
        strobfs = ssr.substr(ssr.find("/?") + 2);
        ssr = ssr.substr(0, ssr.find("/?"));
        group = urlsafe_base64_decode(getUrlArg(strobfs, "group"));
        remarks = urlsafe_base64_decode(getUrlArg(strobfs, "remarks"));
        remarks_base64 = urlsafe_base64_reverse(getUrlArg(strobfs, "remarks"));
        obfsparam = urlsafe_base64_decode(getUrlArg(strobfs, "obfsparam"));
        protoparam = urlsafe_base64_decode(getUrlArg(strobfs, "protoparam"));
    }

    ssr = regReplace(ssr, "(.*):(.*?):(.*?):(.*?):(.*?):(.*)", "$1,$2,$3,$4,$5,$6");
    strcfg = split(ssr, ",");

    if(strcfg.size() != 6)
        return;

    server = strcfg[0];
    port = custom_port == "" ? strcfg[1] : custom_port;
    protocol = strcfg[2];
    method = strcfg[3];
    obfs = strcfg[4];
    password = base64_decode(strcfg[5]);

    if(group == "")
        group = SSR_DEFAULT_GROUP;
    if(remarks == "")
    {
        remarks = server + ":" + port;
        remarks_base64 = base64_encode(remarks);
    }

    node->linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
    node->group = group;
    node->remarks = remarks;
    node->server = server;
    node->port = stoi(port);
    node->proxyStr = ssrConstruct(group, remarks, remarks_base64, server, port, protocol, method, obfs, password, obfsparam, protoparam, local_port, libev);
}

void explodeSSRConf(string content, string custom_port, int local_port, bool libev, vector<nodeInfo> *nodes)
{
    nodeInfo node;
    Document json;
    string base, config, remarks, remarks_base64, group, server, port, method, password, protocol, protoparam, obfs, obfsparam;

    json.Parse(content.data());
    for(unsigned int i = 0; i < json["configs"].Size(); i++)
    {
        config = config_ssr_libev;
        json["configs"][i]["group"] >> group;
        if(group == "")
            group = SSR_DEFAULT_GROUP;
        json["configs"][i]["remarks"] >> remarks;
        json["configs"][i]["remarks_base64"] >> remarks_base64;
        json["configs"][i]["password"] >> password;
        json["configs"][i]["method"] >> method;
        json["configs"][i]["server"] >> server;
        if(custom_port != "")
            port = custom_port;
        else
            json["configs"][i]["server_port"] >> port;
        json["configs"][i]["protocol"] >> protocol;
        json["configs"][i]["protocolparam"] >> protoparam;
        json["configs"][i]["obfs"] >> obfs;
        json["configs"][i]["obfsparam"] >> obfsparam;

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
        node.group = group;
        node.remarks = remarks;
        node.server = server;
        node.port = stoi(port);
        node.proxyStr = ssrConstruct(group, remarks, remarks_base64, server, port, protocol, method, obfs, password, obfsparam, protoparam, local_port, libev);
        nodes->push_back(node);
    }
    return;
}

void explodeSS(string ss, bool libev, string custom_port, int local_port, nodeInfo *node)
{
    string ps, password, method, server, port, plugins, plugin, pluginopts, addition, group = SS_DEFAULT_GROUP;
    vector<string> args, secret;
    ss = replace_all_distinct(replace_all_distinct(ss.substr(5), "\r", ""), "/?", "?");
    if(strFind(ss, "#"))
    {
        ps = UrlDecode(ss.substr(ss.find("#") + 1));
        ss = ss.substr(0, ss.find("#"));
    }

    if(strFind(ss, "?"))
    {
        addition = ss.substr(ss.find("?") + 1);
        plugins = UrlDecode(getUrlArg(addition, "plugin"));
        plugin = plugins.substr(0, plugins.find(";"));
        pluginopts = plugins.substr(plugins.find(";") + 1);
        if(getUrlArg(addition, "group") != "")
            group = base64_decode(getUrlArg(addition, "group"));
        ss = ss.substr(0, ss.find("?"));
    }
    if(strFind(ss, "@"))
    {
        ss = regReplace(ss, "(.*?)@(.*):(.*)", "$1,$2,$3");
        args = split(ss, ",");
        secret = split(urlsafe_base64_decode(args[0]), ":");
        method = secret[0];
        password = secret[1];
        server = args[1];
        port = custom_port == "" ? args[2] : custom_port;
    }
    else
    {
        ss = regReplace(urlsafe_base64_decode(ss), "(.*?):(.*?)@(.*):(.*)", "$1,$2,$3,$4");
        args = split(ss, ",");
        method = args[0];
        password = args[1];
        server = args[2];
        port = custom_port == "" ? args[3] : custom_port;
    }
    if(ps == "")
        ps = server + ":" + port;

    node->linkType = SPEEDTEST_MESSAGE_FOUNDSS;
    node->group = group;
    node->remarks = ps;
    node->server = server;
    node->port = stoi(port);
    node->proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, ps, local_port, libev);
}

void explodeSSD(string link, bool libev, string custom_port, int local_port, vector<nodeInfo> *nodes)
{
    Document jsondata;
    nodeInfo node;
    unsigned int index = nodes->size();
    string group, port, method, password, server, remarks;
    string plugin, pluginopts;
    link = base64_decode(link.substr(6));
    jsondata.Parse(link.c_str());
    for(unsigned int i = 0; i < jsondata["servers"].Size(); i++)
    {
        jsondata["airport"] >> group;
        jsondata["port"] >> port;
        jsondata["encryption"] >> method;
        jsondata["password"] >> password;
        jsondata["servers"][i]["remarks"] >> remarks;
        jsondata["servers"][i]["server"] >> server;
        GetMember(jsondata["servers"][i], "plugin", &plugin);
        GetMember(jsondata["servers"][i], "plugin_options", &pluginopts);
        GetMember(jsondata["servers"][i], "encryption", &method);
        GetMember(jsondata["servers"][i], "port", &port);
        GetMember(jsondata["servers"][i], "password", &password);

        if(custom_port != "")
            port = custom_port;

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.group = group;
        node.remarks = remarks;
        node.server = server;
        node.port = stoi(port);
        node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, remarks, local_port, libev);
        node.id = index;
        nodes->push_back(node);
        index++;
        writeLog(LOG_TYPE_INFO, "Node  " + node.group + " - " + node.remarks + "  has been added.");
    }
    return;
}

void explodeSSAndroid(string ss, bool libev, string custom_port, int local_port, vector<nodeInfo> *nodes)
{
    Document json;
    nodeInfo node;
    string ps, password, method, server, port, group = SS_DEFAULT_GROUP;
    //first add some extra data before parsing
    ss = "{\"nodes\":" + ss + "}";
    json.Parse(ss.data());

    for(unsigned int i = 0; i < json["nodes"].Size(); i++)
    {
        json["nodes"][i]["server"] >> server;
        if(custom_port != "")
            port = custom_port;
        else
            json["nodes"][i]["server_port"] >> port;
        json["nodes"][i]["password"] >> password;
        json["nodes"][i]["method"] >> method;
        json["nodes"][i]["remarks"] >> ps;

        if(ps == "")
            ps = server + ":" + port;

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.group = group;
        node.remarks = ps;
        node.server = server;
        node.port = stoi(port);
        node.proxyStr = ssConstruct(server, port, password, method, "", "", ps, local_port, libev);
        nodes->push_back(node);
    }
}

void explodeSSConf(string content, string custom_port, int local_port, bool libev, vector<nodeInfo> *nodes)
{
    nodeInfo node;
    Document json;
    string config, ps, password, method, server, port, plugin, pluginopts, group = SS_DEFAULT_GROUP;

    json.Parse(content.data());
    for(unsigned int i = 0; i < json["configs"].Size(); i++)
    {
        config = config_ss_libev;
        json["configs"][i]["remarks"] >> ps;
        json["configs"][i]["password"] >> password;
        json["configs"][i]["method"] >> method;
        json["configs"][i]["server"] >> server;
        if(custom_port != "")
            port = custom_port;
        else
            json["configs"][i]["server_port"] >> port;
        json["configs"][i]["plugin"] >> plugin;
        json["configs"][i]["plugin_opts"] >> pluginopts;
        if(ps == "")
        {
            ps = server + ":" + port;
        }

        node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
        node.group = group;
        node.remarks = ps;
        node.server = server;
        node.port = stoi(port);
        node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, ps, local_port, libev);
        nodes->push_back(node);
    }
    return;
}

void explodeSocks(string link, string custom_port, nodeInfo *node)
{
    string remarks, server, port, username, password;
    if(strFind(link, "socks://")) //v2rayn socks link
    {
        vector<string> arguments;
        if(strFind(link, "#"))
        {
            remarks = UrlDecode(link.substr(link.find("#") + 1));
            link = link.substr(0, link.find("#"));
        }
        link = base64_decode(link.substr(8));
        arguments = split(link, ":");
        server = arguments[0];
        port = arguments[1];
    }
    else if(strFind(link, "https://t.me/socks") || strFind(link, "tg://socks"))
    {
        server = getUrlArg(link, "server");
        port = getUrlArg(link, "port");
        username = getUrlArg(link, "user");
        password = getUrlArg(link, "pass");
    }
    if(remarks == "")
    {
        remarks = server + ":" + port;
    }
    if(custom_port != "")
    {
        port = custom_port;
    }

    node->linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
    node->group = SOCKS_DEFAULT_GROUP;
    node->remarks = remarks;
    node->server = server;
    node->port = stoi(port);
    node->proxyStr = "user=" + username + "&pass=" + password;
}

void explodeClash(Node yamlnode, string custom_port, int local_port, vector<nodeInfo> *nodes, bool libev)
{
    nodeInfo node;
    unsigned int index = nodes->size();
    string proxytype, strTemp, ps, server, port, cipher, group; //common
    string type = "none", id, aid = "0", net = "tcp", path, host, tls; //vmess
    string password, plugin, pluginopts, pluginopts_mode, pluginopts_host; //ss
    string user, pass; //socks
    for(unsigned int i = 0; i < yamlnode["Proxy"].size(); i++)
    {
        yamlnode["Proxy"][i]["type"] >> proxytype;
        yamlnode["Proxy"][i]["name"] >> ps;
        yamlnode["Proxy"][i]["server"] >> server;
        port = custom_port == "" ? yamlnode["Proxy"][i]["port"].as<string>() : custom_port;
        if(proxytype == "vmess")
        {
            group = V2RAY_DEFAULT_GROUP;

            yamlnode["Proxy"][i]["uuid"] >> id;
            yamlnode["Proxy"][i]["alterId"] >> aid;
            yamlnode["Proxy"][i]["cipher"] >> cipher;
            net = yamlnode["Proxy"][i]["network"].IsDefined() ? yamlnode["Proxy"][i]["network"].as<string>() : "tcp";
            path = yamlnode["Proxy"][i]["ws-path"].IsDefined() ? yamlnode["Proxy"][i]["ws-path"].as<string>() : "/";
            if(yamlnode["Proxy"][i]["tls"].IsDefined())
                tls = yamlnode["Proxy"][i]["tls"].as<string>() == "true" ? "tls" : "";
            else
                tls = "";
            if(yamlnode["Proxy"][i]["ws-headers"].IsDefined())
                yamlnode["Proxy"][i]["ws-headers"]["Host"] >> host;
            else
                host = "";


            node.linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
            node.proxyStr = vmessConstruct(server, port, type, id, aid, net, cipher, path, host, tls, local_port);
        }
        else if(proxytype == "ss")
        {
            group = SS_DEFAULT_GROUP;

            yamlnode["Proxy"][i]["cipher"] >> cipher;
            yamlnode["Proxy"][i]["password"] >> password;
            if(yamlnode["Proxy"][i]["plugin"].IsDefined())
            {
                if(yamlnode["Proxy"][i]["plugin"].as<string>() == "obfs")
                {
                    plugin = "simple-obfs";
                    if(yamlnode["Proxy"][i]["plugin-opts"].IsDefined())
                    {
                        yamlnode["Proxy"][i]["plugin-opts"]["mode"] >> pluginopts_mode;
                        if(yamlnode["Proxy"][i]["plugin-opts"]["host"].IsDefined())
                            yamlnode["Proxy"][i]["plugin-opts"]["host"] >> pluginopts_host;
                    }
                }
            }
            else if(yamlnode["Proxy"][i]["obfs"].IsDefined())
            {
                plugin = "simple-obfs";
                yamlnode["Proxy"][i]["obfs"] >> pluginopts_mode;
                if(yamlnode["Proxy"][i]["obfs-host"].IsDefined())
                {
                    yamlnode["Proxy"][i]["obfs-host"] >> pluginopts_host;
                }
            }
            else
                plugin = "";

            if(plugin != "")
            {
                pluginopts = "obfs=" + pluginopts_mode;
                pluginopts += pluginopts_host == "" ? "" : ";obfs-host=" + pluginopts_host;
            }

            //support for go-shadowsocks2
            if(cipher == "AEAD_CHACHA20_POLY1305")
                cipher = "chacha20-ietf-poly1305";
            else if(strFind(cipher, "AEAD"))
            {
                cipher = replace_all_distinct(replace_all_distinct(cipher, "AEAD_", ""), "_", "-");
                transform(cipher.begin(), cipher.end(), cipher.begin(), ::tolower);
            }

            node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
            node.proxyStr = ssConstruct(server, port, password, cipher, plugin, pluginopts, ps, local_port, libev);
        }
        else if(proxytype == "socks")
        {
            group = SOCKS_DEFAULT_GROUP;

            if(yamlnode["Proxy"][i]["username"].IsDefined() && yamlnode["Proxy"][i]["password"].IsDefined())
            {
                yamlnode["Proxy"][i]["username"] >> user;
                yamlnode["Proxy"][i]["password"] >> pass;
            }
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
            node.proxyStr = "user=" + user + "&pass=" + pass;
        }
        else
            continue;
        node.group = group;
        node.remarks = ps;
        node.server = server;
        node.port = stoi(port);
        node.id = index;
        nodes->push_back(node);
        writeLog(LOG_TYPE_INFO, "Node  " + node.group + " - " + node.remarks + "  has been added.");
        index++;
    }
    return;
}

void explodeQuan(string quan, string custom_port, int local_port, nodeInfo *node)
{
    string strTemp, itemName, itemVal;
    string group = V2RAY_DEFAULT_GROUP, ps, add, port, cipher = "auto", type = "none", id, aid = "0", net = "tcp", path, host, tls;
    vector<string> configs, vArray;
    strTemp = regReplace(quan, "(.*?) = (.*)", "$1,$2");
    configs = split(strTemp, ",");
    if(configs[1] == "vmess")
    {
        ps = trim(configs[0]);
        add = trim(configs[2]);
        port = trim(configs[3]);
        cipher = trim(configs[4]);
        id = trim(replace_all_distinct(configs[5], "\"", ""));

        //read link
        for(unsigned int i = 6; i < configs.size(); i++)
        {
            vArray = split(configs[i], "=");
            if(vArray.size() < 2)
                continue;
            itemName = trim(vArray[0]);
            itemVal = trim(vArray[1]);
            if(itemName == "group")
                group = itemVal;
            else if(itemName == "over-tls")
                tls = itemVal == "true" ? "tls" : "";
            else if(itemName == "tls-host")
                host = itemVal;
            else if(itemName == "obfs-path")
                path = replace_all_distinct(itemVal, "\"", "");
            else if(itemName == "obfs-header")
            {
                vector<string> headers = split(replace_all_distinct(replace_all_distinct(itemVal, "\"", ""), "[Rr][Nn]", "|"), "|");
                for(unsigned int j = 0; j < headers.size(); j++)
                {
                    if(strFind(headers[j], "Host: "))
                        host = headers[j].substr(6);
                }
            }
            else if(itemName == "obfs" && itemVal == "ws")
                net = "ws";
        }
        if(path == "")
            path = "/";

        node->linkType = SPEEDTEST_MESSAGE_FOUNDVMESS;
        node->group = group;
        node->remarks = ps;
        node->server = add;
        node->port = stoi(port);
        node->proxyStr = vmessConstruct(add, port, type, id, aid, net, cipher, path, host, tls, local_port);
    }
}

bool explodeSurge(string surge, string custom_port, int local_port, vector<nodeInfo> *nodes, bool libev)
{
    string line, remarks, server, port, method, username, password, plugin, pluginopts, pluginopts_mode, pluginopts_host = "cloudfront.net", mod_url, mod_md5;
    stringstream data;
    vector<string> configs, vArray;
    multimap<string, string> proxies;
    nodeInfo node;
    unsigned int i, index = nodes->size();
    INIReader ini;

    if(!strFind(surge, "[Proxy]"))
        return false;

    ini.IncludeSection("Proxy");
    ini.Parse(surge);

    if(!ini.SectionExist("Proxy"))
        return false;
    ini.EnterSection("Proxy");
    ini.GetItems(&proxies);

    for(auto &x : proxies)
    {
        remarks = x.first;
        configs = split(x.second, ",");
        if(configs[0] == "direct")
            continue;
        else if(configs[0] == "custom") //surge 2 style custom proxy
        {
                mod_url = trim(configs[5]);
                if(parsedMD5.count(mod_url) > 0)
                {
                    mod_md5 = parsedMD5[mod_url]; //read calculated MD5 from map
                }
                else
                {
                    mod_md5 = getMD5(webGet(mod_url)); //retrieve module and calculate MD5
                    parsedMD5.insert(pair<string, string>(mod_url, mod_md5)); //save unrecognized module MD5 to map
                }

                if(mod_md5 == modSSMD5) //is SSEncrypt module
                {
                    server = trim(configs[1]);
                    port = custom_port == "" ? trim(configs[2]) : custom_port;
                    method = trim(configs[3]);
                    password = trim(configs[4]);
                    plugin = "";

                    for(i = 6; i < configs.size(); i++)
                    {
                        vArray = split(trim(configs[i]), "=");
                        if(vArray.size() < 2)
                            continue;
                        else if(vArray[0] == "obfs")
                        {
                            plugin = "simple-obfs";
                            pluginopts_mode = vArray[1];
                        }
                        else if(vArray[0] == "obfs-host")
                            pluginopts_host = vArray[1];
                    }
                    if(plugin != "")
                    {
                        pluginopts = "obfs=" + pluginopts_mode;
                        pluginopts += pluginopts_host == "" ? "" : ";obfs-host=" + pluginopts_host;
                    }

                    node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
                    node.group = SS_DEFAULT_GROUP;
                    node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, remarks, local_port, libev);
                }
                else
                    continue;
            }
            else if(configs[0] == "ss") //surge 3 style ss proxy
            {
                server = trim(configs[1]);
                port = custom_port == "" ? trim(configs[2]) : custom_port;
                plugin = "";

                for(i = 3; i < configs.size(); i++)
                {
                    vArray = split(trim(configs[i]), "=");
                    if(vArray.size() < 2)
                        continue;
                    else if(vArray[0] == "encrypt-method")
                        method = vArray[1];
                    else if(vArray[0] == "password")
                        password = vArray[1];
                    else if(vArray[0] == "obfs")
                    {
                        plugin = "simple-obfs";
                        pluginopts_mode = vArray[1];
                    }
                    else if(vArray[0] == "obfs-host")
                        pluginopts_host = vArray[1];
                }
                if(plugin != "")
                {
                    pluginopts = "obfs=" + pluginopts_mode;
                    pluginopts += pluginopts_host == "" ? "" : ";obfs-host=" + pluginopts_host;
                }

                node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
                node.group = SS_DEFAULT_GROUP;
                node.proxyStr = ssConstruct(server, port, password, method, plugin, pluginopts, remarks, local_port, libev);
            }
            else if(configs[0] == "socks5") //surge 3 style socks5 proxy
            {
                node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
                node.group = SOCKS_DEFAULT_GROUP;
                if(configs.size() >= 5)
                {
                    username = trim(configs[2]);
                    password = trim(configs[3]);
                }
                node.proxyStr = "user=" + username + "&pass=" + password;
            }
            else
                    continue;
            node.remarks = remarks;
            node.server = server;
            node.port = stoi(port);
            node.id = index;
            nodes->push_back(node);
            writeLog(LOG_TYPE_INFO, "Node  " + node.group + " - " + node.remarks + "  has been added.");
            index++;
    }
    return true;
}

void explodeSSTap(string sstap, string custom_port, int local_port, vector<nodeInfo> *nodes, bool ss_libev, bool ssr_libev)
{
    Document json;
    nodeInfo node;
    string configType, group, remarks, server, port;
    string cipher;
    string user, pass;
    string protocol, protoparam, obfs, obfsparam;
    json.Parse(sstap.data());

    for(unsigned int i = 0; i < json["configs"].Size(); i++)
    {
        json["configs"][i]["group"] >> group;
        json["configs"][i]["remarks"] >> remarks;
        json["configs"][i]["server"] >> server;
        json["configs"][i]["server_port"] >> port;
        json["configs"][i]["password"] >> pass;
        json["configs"][i]["type"] >> configType;
        if(configType == "5") //socks 5
        {
            json["configs"][i]["username"] >> user;
            node.linkType = SPEEDTEST_MESSAGE_FOUNDSOCKS;
            node.proxyStr = "user=" + user + "&pass=" + pass;
        }
        else if(configType == "6") //ss/ssr
        {
            json["configs"][i]["protocol"] >> protocol;
            json["configs"][i]["obfs"] >> obfs;
            json["configs"][i]["method"] >> cipher;
            if(find(ss_ciphers.begin(), ss_ciphers.end(), cipher) != ss_ciphers.end() && protocol == "origin" && obfs == "plain") //is ss
            {
                node.linkType = SPEEDTEST_MESSAGE_FOUNDSS;
                node.proxyStr = ssConstruct(server, port, pass, cipher, "", "", remarks, local_port, ss_libev);
            }
            else //is ssr cipher
            {
                json["configs"][i]["obfsparam"] >> obfsparam;
                json["configs"][i]["protocolparam"] >> protoparam;
                node.linkType = SPEEDTEST_MESSAGE_FOUNDSSR;
                node.proxyStr = ssrConstruct(group, remarks, base64_encode(remarks), server, port, protocol, cipher, obfs, pass, obfsparam, protoparam, local_port, ssr_libev);
            }
        }
        else
            continue;

        if(remarks == "")
            remarks = server + ":" + port;

        node.group = group;
        node.remarks = remarks;
        node.server = server;
        node.port = stoi(port);
        nodes->push_back(node);
    }
}

int explodeLog(string log, vector<nodeInfo> *nodes)
{
    INIReader ini;
    vector<string> nodeList, vArray;
    string strTemp;
    int index;
    nodeInfo node;
    ini.Parse(log);

    if(!ini.SectionExist("Basic"))
        return -1;

    nodeList = ini.GetSections();
    node.proxyStr = "LOG";
    for(auto &x : nodeList)
    {
        if(x == "Basic")
            continue;
        ini.EnterSection(x);
        vArray = split(x, "^");
        node.group = vArray[0];
        node.remarks = vArray[1];
        node.avgPing = ini.Get("AvgPing");
        node.avgSpeed = ini.Get("AvgSpeed");
        node.groupID = stoi(ini.Get("GroupID"));
        node.id = stoi(ini.Get("ID"));
        node.maxSpeed = ini.Get("MaxSpeed");
        node.online = ini.GetBool("Online");
        node.pkLoss = ini.Get("PkLoss");
        vArray = split(ini.Get("RawPing"), ",");
        index = 0;
        for(auto &y : vArray)
        {
            node.rawPing[index] = stoi(y);
            index++;
        }
        vArray = split(ini.Get("RawSitePing"), ",");
        index = 0;
        for(auto &y : vArray)
        {
            node.rawSitePing[index] = stoi(y);
            index++;
        }
        vArray = split(ini.Get("RawSpeed"), ",");
        index = 0;
        for(auto &y : vArray)
        {
            node.rawSpeed[index] = stoi(y);
            index++;
        }
        node.sitePing = ini.Get("SitePing");
        node.totalRecvBytes = stoi(ini.Get("UsedTraffic"));
        node.ulSpeed = ini.Get("ULSpeed");
        nodes->push_back(node);
    }

    return 0;
}

bool chkIgnore(nodeInfo *node, vector<string> *exclude_remarks, vector<string> *include_remarks)
{
    bool excluded = false, included = false;
    string remarks = UTF8ToGBK(node->remarks);
    writeLog(LOG_TYPE_INFO, "Comparing exclude remarks...");
    for(auto &x : *exclude_remarks)
    {
        if(strFind(remarks, x))
            excluded = true;
    }
    if(include_remarks->size() != 0)
    {
        writeLog(LOG_TYPE_INFO, "Comparing include remarks...");
        for(auto &x : *include_remarks)
        {
            if(strFind(remarks, x))
                included = true;
        }
    }
    else
    {
        included = true;
    }

    return excluded || !included;
}

int explodeConf(string filepath, string custom_port, int local_port, bool sslibev, bool ssrlibev, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks)
{
    ifstream infile;
    stringstream contentstrm;
    infile.open(filepath);

    contentstrm << infile.rdbuf();
    infile.close();

    return explodeConfContent(contentstrm.str(), custom_port, local_port, sslibev, ssrlibev, nodes, exclude_remarks, include_remarks);
}

int explodeConfContent(string content, string custom_port, int local_port, bool sslibev, bool ssrlibev, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks)
{
    unsigned int index = 0;
    int filetype = -1;
    vector<nodeInfo>::iterator iter;

    if(strFind(content, "\"version\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSS;
    else if(strFind(content, "\"serverSubscribes\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSSR;
    else if(strFind(content, "\"uiItem\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDVMESS;
    else if(strFind(content, "\"proxy_apps\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSSCONF;
    else if(strFind(content, "\"idInUse\""))
        filetype = SPEEDTEST_MESSAGE_FOUNDSSTAP;

    switch(filetype)
    {
    case SPEEDTEST_MESSAGE_FOUNDSS:
        explodeSSConf(content, custom_port, local_port, sslibev, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSR:
        explodeSSRConf(content, custom_port, local_port, ssrlibev, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDVMESS:
        explodeVmessConf(content, custom_port, local_port, sslibev, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSCONF:
        explodeSSAndroid(content, sslibev, custom_port, local_port, nodes);
        break;
    case SPEEDTEST_MESSAGE_FOUNDSSTAP:
        explodeSSTap(content, custom_port, local_port, nodes, sslibev, ssrlibev);
        break;
    default:
        //try to parse as a local subscription
        explodeSub(content, sslibev, ssrlibev, custom_port, local_port, nodes, exclude_remarks, include_remarks);
        if(nodes->size() == 0)
            return SPEEDTEST_ERROR_UNRECOGFILE;
        else
            return SPEEDTEST_ERROR_NONE;
    }

    iter = nodes->begin();
    while(iter != nodes->end())
    {
        if(chkIgnore(&*iter, exclude_remarks, include_remarks))
        {
            writeLog(LOG_TYPE_INFO, "Node  " + iter->group + " - " + iter->remarks + "  has been ignored and will not be tested.");
            nodes->erase(iter);
        }
        else
        {
            writeLog(LOG_TYPE_INFO, "Node  " + iter->group + " - " + iter->remarks + "  has been added.");
            iter->id = index;
            index++;
            iter++;
        }
    }

    return SPEEDTEST_ERROR_NONE;
}

void explode(string link, bool sslibev, bool ssrlibev, string custom_port, int local_port, nodeInfo *node)
{
    if(strFind(link, "ssr://"))
        explodeSSR(link, ssrlibev, custom_port, local_port, node);
    else if(strFind(link, "vmess://"))
        explodeVmess(link, custom_port, local_port, node);
    else if(strFind(link, "ss://"))
        explodeSS(link, sslibev, custom_port, local_port, node);
    else if(strFind(link, "socks://") || strFind(link, "https://t.me/socks") || strFind(link, "tg://socks"))
        explodeSocks(link, custom_port, node);
}

void explodeSub(string sub, bool sslibev, bool ssrlibev, string custom_port, int local_port, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks)
{
    stringstream strstream;
    string strLink;
    nodeInfo node;

    //try to parse as surge configuration
    if(explodeSurge(sub, custom_port, local_port, nodes, sslibev))
    {
        return;
    }

    //try to parse as exported log
    if(explodeLog(sub, nodes) == 0)
    {
        return;
    }

    //try to parse as clash configuration
    try
    {
        if(strFind(sub, "Proxy"))
        {
            Node yamlnode = Load(sub);
            if(yamlnode.size() && yamlnode["Proxy"])
            {
                explodeClash(yamlnode, custom_port, local_port, nodes, sslibev);
                return;
            }
        }
    }
    catch (exception &e)
    {
        //ignore
    }

    //try to parse as SSD configuration
    if(strFind(sub, "ssd://"))
    {
        explodeSSD(sub, sslibev, custom_port, local_port, nodes);
        return;
    }

    //try to parse as normal subscription
    sub = base64_decode(sub);
    strstream << sub;
    unsigned int index = nodes->size();
    char delimiter = split(sub, "\n").size() <= 1 ? split(sub, "\r").size() <= 1 ? ' ' : '\r' : '\n';
    while(getline(strstream, strLink, delimiter))
    {
        explode(strLink, sslibev, ssrlibev, custom_port, local_port, &node);
        if(strLink.size() == 0 || node.linkType == -1)
        {
            continue;
        }
        if(chkIgnore(&node, exclude_remarks, include_remarks))
        {
            writeLog(LOG_TYPE_INFO, "Node  " + node.group + " - " + node.remarks + "  has been ignored and will not be tested.");
            continue;
        }
        else
            writeLog(LOG_TYPE_INFO, "Node  " + node.group + " - " + node.remarks + "  has been added.");
        node.id = index;
        nodes->push_back(node);
        index++;
        //nodes->insert(nodes->end(), node);
    }
}
