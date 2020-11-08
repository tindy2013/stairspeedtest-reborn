#include <string>
#include <vector>
#include <numeric>

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
std::string base_vmess = R"({"inbounds":[{"port":?localport?,"listen":"127.0.0.1","protocol":"socks","settings":{"udp":true}}],"outbounds":[{"tag":"proxy","protocol":"vmess","settings":{"vnext":[{"address":"?add?","port":?port?,"users":[{"id":"?id?","alterId":?aid?,"email":"t@t.tt","security":"?cipher?"}]}]},"streamSettings":{"network":"?net?","security":"?tls?","tlsSettings":?tlsset?,"tcpSettings":?tcpset?,"wsSettings":?wsset?,"kcpSettings":?kcpset?,"httpSettings":?h2set?,"quicSettings":?quicset?},"mux":{"enabled":false}}],"routing":{"domainStrategy":"IPIfNonMatch"}})";
std::string wsset_vmess = R"({"connectionReuse":true,"path":"?path?","headers":{"Host":"?host?"?edge?}})";
std::string tcpset_vmess = R"({"connectionReuse":true,"header":{"type":"?type?","request":{"version":"1.1","method":"GET","path":["?path?"],"headers":{"Host":["?host?"],"User-Agent":["Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.75 Safari/537.36","Mozilla/5.0 (iPhone; CPU iPhone OS 10_0_2 like Mac OS X) AppleWebKit/601.1 (KHTML, like Gecko) CriOS/53.0.2785.109 Mobile/14A456 Safari/601.1.46"],"Accept-Encoding":["gzip, deflate"],"Connection":["keep-alive"],"Pragma":"no-cache"}}}})";
std::string tlsset_vmess = R"({"serverName":"?serverName?","allowInsecure":?verify?,"allowInsecureCiphers":true})";
std::string kcpset_vmess = R"({"mtu":1350,"tti":50,"uplinkCapacity":12,"downlinkCapacity":100,"congestion":false,"readBufferSize":2,"writeBufferSize":2,"header":{"type":"?type?"}})";
std::string h2set_vmess = R"({"path":"?path?","host":[?host?]})";
std::string quicset_vmess = R"({"security":"?host?","key":"?path?","header":{"type":"?type?"}})";
std::string base_trojan = R"({"run_type":"client","local_addr":"127.0.0.1","local_port":?localport?,"remote_addr":"?server?","remote_port":?port?,"password":["?password?"],"ssl":{"verify":?verify?,"verify_hostname":?verifyhost?,"sni":"?host?"},"tcp":{"reuse_port":true}})";

int explodeLog(const std::string &log, std::vector<nodeInfo> &nodes)
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
        node.groupID = ini.GetNumber<int>("GroupID");
        node.id = ini.GetNumber<int>("ID");
        node.maxSpeed = ini.Get("MaxSpeed");
        node.online = ini.GetBool("Online");
        node.pkLoss = ini.Get("PkLoss");
        ini.GetNumberArray<int>("RawPing", ",", node.rawPing);
        ini.GetNumberArray<int>("RawSitePing", ",", node.rawSitePing);
        ini.GetNumberArray<unsigned long long>("RawSpeed", ",", node.rawSpeed);
        node.sitePing = ini.Get("SitePing");
        node.totalRecvBytes = ini.GetNumber<unsigned long long>("UsedTraffic");
        node.ulSpeed = ini.Get("ULSpeed");
        nodes.push_back(node);
    }

    return 0;
}

std::string replace_first(std::string str, const std::string &old_value, const std::string &new_value)
{
    string_size pos = str.find(old_value);
    if(pos == str.npos)
        return str;
    return str.replace(pos, old_value.size(), new_value);
}

std::string vmessConstruct(const std::string &group, const std::string &remarks, const std::string &add, const std::string &port, const std::string &type, const std::string &id, const std::string &aid, const std::string &net, const std::string &cipher, const std::string &path, const std::string &host, const std::string &edge, const std::string &tls, tribool udp, tribool tfo, tribool scv, tribool tls13)
{
    std::string base = base_vmess;
    base = replace_first(base, "?localport?", std::to_string(socksport));
    base = replace_first(base, "?add?", add);
    base = replace_first(base, "?port?", port);
    base = replace_first(base, "?id?", id);
    base = replace_first(base, "?aid?", aid.empty() ? "0" : aid);
    base = replace_first(base, "?net?", net.empty() ? "tcp" : net);
    base = replace_first(base, "?cipher?", cipher);
    switch(hash_(net))
    {
        case "ws"_hash:
        {
            std::string wsset = wsset_vmess;
            wsset = replace_first(wsset, "?host?", (host.empty() && !isIPv4(add) && !isIPv6(add)) ? add : trim(host));
            wsset = replace_first(wsset, "?path?", path.empty() ? "/" : path);
            wsset = replace_first(wsset, "?edge?", edge.empty() ? "" : ",\"Edge\":\"" + edge + "\"");
            base = replace_first(base, "?wsset?", wsset);
            break;
        }
        case "kcp"_hash:
        {
            std::string kcpset = kcpset_vmess;
            kcpset = replace_first(kcpset, "?type?", type);
            base = replace_first(base, "?kcpset?", kcpset);
            break;
        }
        case "h2"_hash:
        case "http"_hash:
        {
            std::string h2set = h2set_vmess;
            h2set = replace_first(h2set, "?path?", path);
            string_array hosts = split(host, ",");
            h2set = replace_first(h2set, "?host?", std::accumulate(std::next(hosts.begin()), hosts.end(), std::string("\"" + hosts[0] + "\""), [](auto before, auto current){ return before + ",\"" + current + "\""; }));
            base = replace_first(base, "?h2set?", h2set);
            break;
        }
        case "quic"_hash:
        {
            std::string quicset = quicset_vmess;
            quicset = replace_first(quicset, "?host?", host);
            quicset = replace_first(quicset, "?path?", path);
            quicset = replace_first(quicset, "?type?", type);
            base = replace_first(base, "?quicset?", quicset);
            break;
        }
        case "tcp"_hash:
            break;
    }
    if(type == "http")
    {
        std::string tcpset = tcpset_vmess;
        tcpset = replace_first(tcpset, "?host?", (host.empty() && !isIPv4(add) && !isIPv6(add)) ? add : trim(host));
        tcpset = replace_first(tcpset, "?type?", type);
        tcpset = replace_first(tcpset, "?path?", path.empty() ? "/" : path);
        base = replace_first(base, "?tcpset?", tcpset);
    }
    if(host.size())
    {
        std::string tlsset = tlsset_vmess;
        tlsset = replace_first(tlsset, "?serverName?", host);
        scv.define(true);
        tlsset = replace_first(tlsset, "?verify?", scv ? "true" : "false");
        base = replace_first(base, "?tlsset?", tlsset);
    }

    base = replace_first(base, "?tls?", tls);
    base = replace_first(base, "?tcpset?", "null");
    base = replace_first(base, "?wsset?", "null");
    base = replace_first(base, "?tlsset?", "null");
    base = replace_first(base, "?kcpset?", "null");
    base = replace_first(base, "?h2set?", "null");
    base = replace_first(base, "?quicset?", "null");

    return base;
}

std::string ssrConstruct(const std::string &group, const std::string &remarks, const std::string &remarks_base64, const std::string &server, const std::string &port, const std::string &protocol, const std::string &method, const std::string &obfs, const std::string &password, const std::string &obfsparam, const std::string &protoparam, bool libev, tribool udp, tribool tfo, tribool scv)
{
    std::string base = base_ssr_win;
    std::string config = config_ssr_win;
    std::string config_libev = config_ssr_libev;
    if(libev == true)
        config = config_libev;
    config = replace_first(config, "?group?", group);
    config = replace_first(config, "?remarks?", remarks);
    if(remarks_base64.empty())
        config = replace_first(config, "?remarks_base64?", base64_encode(remarks));
    else
        config = replace_first(config, "?remarks_base64?", remarks_base64);
    config = replace_first(config, "?server?", isIPv6(server) ? "[" + server + "]" : server);
    config = replace_first(config, "?port?", port);
    config = replace_first(config, "?protocol?", protocol);
    config = replace_first(config, "?method?", method);
    config = replace_first(config, "?obfs?", obfs);
    config = replace_first(config, "?password?", password);
    config = replace_first(config, "?obfsparam?", obfsparam);
    config = replace_first(config, "?protoparam?", protoparam);
    if(libev == true)
        base = config;
    else
        base = replace_first(base, "?config?", config);
    base = replace_first(base, "?localport?", std::to_string(socksport));

    return base;
}

std::string ssConstruct(const std::string &group, const std::string &remarks, const std::string &server, const std::string &port, const std::string &password, const std::string &method, const std::string &plugin, const std::string &pluginopts, bool libev, tribool udp, tribool tfo, tribool scv, tribool tls13)
{
    std::string base = base_ss_win;
    std::string config = config_ss_win;
    std::string config_libev = config_ss_libev;
    if(libev == true)
        config = config_libev;
    config = replace_first(config, "?server?", isIPv6(server) ? "[" + server + "]" : server);
    config = replace_first(config, "?port?", port);
    config = replace_first(config, "?password?", password);
    config = replace_first(config, "?method?", method);
    config = replace_first(config, "?plugin?", plugin.size() ? "./" + (plugin == "obfs-local" ? "simple-obfs" : plugin) : "");
    config = replace_first(config, "?plugin_opts?", pluginopts);
    config = replace_first(config, "?remarks?", remarks);
    if(libev == true)
        base = config;
    else
        base = replace_first(base, "?config?", config);
    base = replace_first(base, "?localport?", std::to_string(socksport));

    return base;
}

std::string socksConstruct(const std::string &group, const std::string &remarks, const std::string &server, const std::string &port, const std::string &username, const std::string &password, tribool udp, tribool tfo, tribool scv)
{
    return "user=" + username + "&pass=" + password;
}

std::string httpConstruct(const std::string &group, const std::string &remarks, const std::string &server, const std::string &port, const std::string &username, const std::string &password, bool tls, tribool tfo, tribool scv, tribool tls13)
{
    return "user=" + username + "&pass=" + password;
}

std::string trojanConstruct(const std::string &group, const std::string &remarks, const std::string &server, const std::string &port, const std::string &password, const std::string &host, bool tlssecure, tribool udp, tribool tfo, tribool scv, tribool tls13)
{
    std::string base = base_trojan;
    scv.define(true);
    base = replace_first(base, "?server?", server);
    base = replace_first(base, "?port?", port);
    base = replace_first(base, "?password?", password);
    base = replace_first(base, "?verify?", scv ? "false" : "true");
    base = replace_first(base, "?verifyhost?", scv ? "false" : "true");
    base = replace_first(base, "?host?", host);
    base = replace_first(base, "?localport?", std::to_string(socksport));
    return base;
}

std::string snellConstruct(const std::string &group, const std::string &remarks, const std::string &server, const std::string &port, const std::string &password, const std::string &obfs, const std::string &host, tribool udp, tribool tfo, tribool scv)
{
    //no clients available, ignore
    return std::string();
}
