#include <string>
#include <vector>

#include "ini_reader.h"
#include "misc.h"
#include "nodeinfo.h"

extern int socksport;

std::string base_ss_win = R"({"version":"4.1.6","configs":[?config?],"strategy":null,"index":0,"global":false,"enabled":false,"shareOverLan":true,"isDefault":false,"localPort":?localport?,"portableMode":true,"pacUrl":null,"useOnlinePac":false,"secureLocalPac":true,"availabilityStatistics":false,"autoCheckUpdate":true,"checkPreRelease":false,"isVerboseLogging":false,"logViewer":{"topMost":false,"wrapText":false,"toolbarShown":false,"Font":"Consolas, 8pt","BackgroundColor":"Black","TextColor":"White"},"proxy":{"useProxy":false,"proxyType":0,"proxyServer":"","proxyPort":0,"proxyTimeout":3,"useAuth":false,"authUser":"","authPwd":""},"hotkey":{"SwitchSystemProxy":"","SwitchSystemProxyMode":"","SwitchAllowLan":"","ShowLogs":"","ServerMoveUp":"","ServerMoveDown":"","RegHotkeysAtStartup":false}})";
std::string config_ss_win = R"({"server":"?server?","server_port":?port?,"password":"?password?","method":"?method?","plugin":"?plugin?","plugin_opts":"?plugin_opts?","plugin_args":"","remarks":"?remarks?","timeout":5})";
std::string config_ss_libev = R"({"server":"?server?","server_port":?port?,"password":"?password?","method":"?method?","plugin":"?plugin?","plugin_opts":"?plugin_opts?","plugin_args":"","local_address":"127.0.0.1","local_port":?localport?,"reuse_port":true})";
std::string base_ssr_win = R"({"configs":[?config?],"index":0,"random":true,"sysProxyMode":1,"shareOverLan":false,"localPort":?localport?,"localAuthPassword":null,"localDnsServer":"","dnsServer":"","reconnectTimes":2,"balanceAlgorithm":"LowException","randomInGroup":false,"TTL":0,"connectTimeout":5,"proxyRuleMode":2,"proxyEnable":false,"pacDirectGoProxy":false,"proxyType":0,"proxyHost":null,"proxyPort":0,"proxyAuthUser":null,"proxyAuthPass":null,"proxyUserAgent":null,"authUser":null,"authPass":null,"autoBan":false,"checkSwitchAutoCloseAll":false,"logEnable":false,"sameHostForSameTarget":false,"keepVisitTime":180,"isHideTips":false,"nodeFeedAutoUpdate":true,"serverSubscribes":[],"token":{},"portMap":{}})";
std::string config_ssr_win = R"({"remarks":"?remarks?","id":"18C4949EBCFE46687AE4A7645725D35F","server":"?server?","server_port":?port?,"server_udp_port":0,"password":"?password?","method":"?method?","protocol":"?protocol?","protocolparam":"?protoparam?","obfs":"?obfs?","obfsparam":"?obfsparam?","remarks_base64":"?remarks_base64?","group":"?group?","enable":true,"udp_over_tcp":false})";
std::string config_ssr_libev = R"({"server":"?server?","server_port":?port?,"protocol":"?protocol?","method":"?method?","obfs":"?obfs?","password":"?password?","obfs_param":"?obfsparam?","protocol_param":"?protoparam?","local_address":"127.0.0.1","local_port":?localport?,"reuse_port":true})";
std::string base_vmess = R"({"inbounds":[{"port":?localport?,"listen":"127.0.0.1","protocol":"socks","settings":{"udp":true}}],"outbounds":[{"tag":"proxy","protocol":"vmess","settings":{"vnext":[{"address":"?add?","port":?port?,"users":[{"id":"?id?","alterId":?aid?,"email":"t@t.tt","security":"?cipher?"}]}]},"streamSettings":{"network":"?net?","security":"?tls?","tlsSettings":?tlsset?,"tcpSettings":?tcpset?,"wsSettings":?wsset?},"mux":{"enabled":false}}],"routing":{"domainStrategy":"IPIfNonMatch"}})";
std::string wsset_vmess = R"({"connectionReuse":true,"path":"?path?","headers":{"Host":"?host?"?edge?}})";
std::string tcpset_vmess = R"({"connectionReuse":true,"header":{"type":"?type?","request":{"version":"1.1","method":"GET","path":["?path?"],"headers":{"Host":["?host?"],"User-Agent":["Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.75 Safari/537.36","Mozilla/5.0 (iPhone; CPU iPhone OS 10_0_2 like Mac OS X) AppleWebKit/601.1 (KHTML, like Gecko) CriOS/53.0.2785.109 Mobile/14A456 Safari/601.1.46"],"Accept-Encoding":["gzip, deflate"],"Connection":["keep-alive"],"Pragma":"no-cache"}}}})";
std::string tlsset_vmess = R"({"serverName":"?serverName?","allowInsecure":?verify?,"allowInsecureCiphers":true})";
std::string base_trojan = R"({"run_type":"client","local_addr":"127.0.0.1","local_port":?localport?,"remote_addr":"?server?","remote_port":?port?,"password":["?password?"],"ssl":{"verify":?verify?,"verify_hostname":?verify?,"sni":"?host?"},"tcp":{"reuse_port":true}})";

int explodeLog(std::string log, std::vector<nodeInfo> &nodes)
{
    INIReader ini;
    std::vector<std::string> nodeList, vArray;
    std::string strTemp;
    nodeInfo node;
    if(!startsWith(log, "[Basic]"))
        return -1;
    ini.Parse(log);

    if(!ini.SectionExist("Basic") || !ini.ItemExist("Basic", "GenerationTime") || !ini.ItemExist("Basic", "Tester"))
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
        node.groupID = ini.GetInt("GroupID");
        node.id = ini.GetInt("ID");
        node.maxSpeed = ini.Get("MaxSpeed");
        node.online = ini.GetBool("Online");
        node.pkLoss = ini.Get("PkLoss");
        ini.GetIntArray("RawPing", ",", node.rawPing);
        ini.GetIntArray("RawSitePing", ",", node.rawSitePing);
        ini.GetIntArray("RawSpeed", ",", node.rawSpeed);
        node.sitePing = ini.Get("SitePing");
        node.totalRecvBytes = ini.GetInt("UsedTraffic");
        node.ulSpeed = ini.Get("ULSpeed");
        nodes.push_back(node);
    }

    return 0;
}

std::string vmessConstruct(std::string group, std::string remarks, std::string add, std::string port, std::string type, std::string id, std::string aid, std::string net, std::string cipher, std::string path, std::string host, std::string edge, std::string tls, tribool udp = tribool(), tribool tfo = tribool(), tribool scv = tribool())
{
    std::string base = base_vmess;
    std::string wsset = wsset_vmess;
    std::string tcpset = tcpset_vmess;
    std::string tlsset = tlsset_vmess;
    host = trim(host);
    path = trim(path);
    base = replace_all_distinct(base, "?localport?", std::to_string(socksport));
    base = replace_all_distinct(base, "?add?", add);
    base = replace_all_distinct(base, "?port?", port);
    base = replace_all_distinct(base, "?id?", id);
    base = replace_all_distinct(base, "?aid?", aid);
    base = replace_all_distinct(base, "?net?", net);
    base = replace_all_distinct(base, "?cipher?", cipher);
    if(net == "ws")
    {
        wsset = replace_all_distinct(wsset, "?host?", host.empty() ? add : host);
        wsset = replace_all_distinct(wsset, "?path?", path.empty() ? "/" : path);
        wsset = replace_all_distinct(wsset, "?edge?", edge.empty() ? "" : ",\"Edge\":\"" + edge + "\"");
        base = replace_all_distinct(base, "?wsset?", wsset);
    }
    if(type == "http")
    {
        tcpset = replace_all_distinct(tcpset, "?host?", host.empty() ? add : host);
        tcpset = replace_all_distinct(tcpset, "?type?", type);
        tcpset = replace_all_distinct(tcpset, "?path?", path.empty() ? "/" : path);
        base = replace_all_distinct(base, "?tcpset?", tcpset);
    }
    if(host.empty() && !isIPv4(add) && !isIPv6(add))
        host = add;
    if(host.size())
    {
        tlsset = replace_all_distinct(tlsset, "?serverName?", host);
        scv.define(true);
        tlsset = replace_all_distinct(tlsset, "?verify?", scv ? "true" : "false");
        base = replace_all_distinct(base, "?tlsset?", tlsset);
    }

    base = replace_all_distinct(base, "?tls?", tls);
    base = replace_all_distinct(base, "?tcpset?", "null");
    base = replace_all_distinct(base, "?wsset?", "null");
    base = replace_all_distinct(base, "?tlsset?", "null");

    return base;
}

std::string ssrConstruct(std::string group, std::string remarks, std::string remarks_base64, std::string server, std::string port, std::string protocol, std::string method, std::string obfs, std::string password, std::string obfsparam, std::string protoparam, bool libev, tribool udp = tribool(), tribool tfo = tribool(), tribool scv = tribool())
{
    std::string base = base_ssr_win;
    std::string config = config_ssr_win;
    std::string config_libev = config_ssr_libev;
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
    base = replace_all_distinct(base, "?localport?", std::to_string(socksport));

    return base;
}

std::string ssConstruct(std::string group, std::string remarks, std::string server, std::string port, std::string password, std::string method, std::string plugin, std::string pluginopts, bool libev, tribool udp = tribool(), tribool tfo = tribool(), tribool scv = tribool())
{
    std::string base = base_ss_win;
    std::string config = config_ss_win;
    std::string config_libev = config_ss_libev;
    if(plugin == "obfs-local")
        plugin = "simple-obfs";
    if(plugin.size())
        plugin = "./" + plugin;
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
    base = replace_all_distinct(base, "?localport?", std::to_string(socksport));

    return base;
}

std::string socksConstruct(std::string group, std::string remarks, std::string server, std::string port, std::string username, std::string password, tribool udp = tribool(), tribool tfo = tribool(), tribool scv = tribool())
{
    return "user=" + username + "&pass=" + password;
}

std::string httpConstruct(std::string group, std::string remarks, std::string server, std::string port, std::string username, std::string password, bool tls, tribool scv = tribool())
{
    return "user=" + username + "&pass=" + password;
}

std::string trojanConstruct(std::string group, std::string remarks, std::string server, std::string port, std::string password, std::string host, bool tlssecure, tribool udp = tribool(), tribool tfo = tribool(), tribool scv = tribool())
{
    std::string base = base_trojan;
    scv.define(true);
    base = replace_all_distinct(base, "?server?", server);
    base = replace_all_distinct(base, "?port?", port);
    base = replace_all_distinct(base, "?password?", password);
    base = replace_all_distinct(base, "?verify?", scv ? "false" : "true");
    base = replace_all_distinct(base, "?host?", host);
    base = replace_all_distinct(base, "?localport?", std::to_string(socksport));
    return base;
}

std::string snellConstruct(std::string group, std::string remarks, std::string server, std::string port, std::string password, std::string obfs, std::string host, tribool udp = tribool(), tribool tfo = tribool(), tribool scv = tribool())
{
    //no clients available, ignore
    return std::string();
}
