#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>

#include "webserver.h"
#include "misc.h"
#include "webget.h"
#include "version.h"
#include "nodeinfo.h"
#include "rapidjson_extra.h"
#include "printout.h"
#include "renderer.h"
#include "speedtestutil.h"
#include "version.h"

std::atomic<bool> start_flag = false;
std::atomic<time_t> done_time = 0;

//variables from main
extern std::vector<nodeInfo> allNodes;
extern int cur_node_id, socksport;
extern std::string speedtest_mode, export_sort_method, export_color_style, custom_group, override_conf_port;
extern bool ssr_libev, ss_libev;
extern std::vector<color> custom_color_groups;
extern std::vector<int> custom_color_bounds;
extern string_array custom_exclude_remarks, custom_include_remarks;
extern unsigned int node_count;

//functions from main
void addNodes(std::string link, bool multilink);
void rewriteNodeID(std::vector<nodeInfo> &nodes);
void batchTest(std::vector<nodeInfo> &nodes);

//webui variables
std::vector<nodeInfo> targetNodes, testedNodes;
std::string server_status = "stopped";
nodeInfo current_node;

nodeInfo find_node(std::string &group, std::string &remarks, std::string &server, int &server_port)
{
    auto iter = std::find_if(allNodes.begin(), allNodes.end(), [&](auto x){ return x.group == group && x.remarks == remarks && x.server == server && x.port == server_port; });
    if(iter != allNodes.end())
        return *iter;
    return nodeInfo();
}

void ssrspeed_regenerate_node_list(rapidjson::Document &json)
{
    nodeInfo node;
    std::string group, remarks, server;
    int server_port;

    eraseElements(targetNodes);
    eraseElements(testedNodes);

    for(unsigned int i = 0; i < json["configs"].Size(); i++)
    {
        group = GetMember(json["configs"][i]["config"], "group");
        remarks = GetMember(json["configs"][i]["config"], "remarks");
        server = GetMember(json["configs"][i]["config"], "server");
        server_port = stoi(GetMember(json["configs"][i]["config"], "server_port"));
        auto iter = std::find_if(allNodes.begin(), allNodes.end(), [&](auto x){ return x.group == group && x.remarks == remarks && x.server == server && x.port == server_port; });
        if(iter != allNodes.end())
            targetNodes.push_back(*iter);
    }
    node_count = json["configs"].Size();
    rewriteNodeID(targetNodes);
}

double ssrspeed_get_speed_number(const std::string &speed)
{
    if(speed == "N/A")
        return 0;

    return streamToInt(speed);
}

void json_write_node(rapidjson::Writer<rapidjson::StringBuffer> &writer, nodeInfo &node)
{
    geoIPInfo inbound = node.inboundGeoIP.get(), outbound = node.outboundGeoIP.get();
    int counter = 0, total = 0;
    writer.Key("group");
    writer.String(node.group.data());
    writer.Key("remarks");
    writer.String(node.remarks.data());
    writer.Key("loss");
    writer.Double(stod(node.pkLoss.substr(0, node.pkLoss.size() - 1)) / 100.0);
    writer.Key("ping");
    writer.Double(stod(node.avgPing) / 1000.0);
    writer.Key("gPing");
    writer.Double(stod(node.sitePing) / 1000.0);
    writer.Key("rawSocketSpeed");
    writer.StartArray();
    for(auto &y : node.rawSpeed)
    {
        writer.Int(y);
    }
    writer.EndArray();
    writer.Key("rawTcpPingStatus");
    writer.StartArray();
    for(auto &y : node.rawPing)
    {
        writer.Double(y / 1000.0);
    }
    writer.EndArray();
    writer.Key("rawGooglePingStatus");
    writer.StartArray();
    counter = total = 0;
    for(auto &y : node.rawSitePing)
    {
        total++;
        writer.Double(y / 1000.0);
        if(y == 0)
            counter++;
    }
    writer.EndArray();
    writer.Key("gPingLoss");
    writer.Double(counter / total * 1.0);
    writer.Key("webPageSimulation");
    writer.String("N/A");
    writer.Key("geoIP");
    writer.StartObject();
    writer.Key("inbound");
    writer.StartObject();
    writer.Key("address");
    writer.String(std::string(node.server + ":" + std::to_string(node.port)).data());
    writer.Key("info");
    writer.String(std::string((inbound.country.size() ? inbound.country : std::string("N/A")) + " " + \
                              (inbound.city.size() ? inbound.city : std::string("N/A")) + ", " + \
                              (inbound.organization.size() ? inbound.organization : std::string("N/A"))).data());
    writer.EndObject();
    writer.Key("outbound");
    writer.StartObject();
    writer.Key("address");
    writer.String(outbound.ip.data());
    writer.Key("info");
    writer.String(std::string((outbound.country.size() ? outbound.country : std::string("N/A")) + " " + \
                              (outbound.city.size() ? outbound.city : std::string("N/A")) + ", " + \
                              (outbound.organization.size() ? outbound.organization : std::string("N/A"))).data());
    writer.EndObject();
    writer.EndObject();
    writer.Key("dspeed");
    writer.Double(ssrspeed_get_speed_number(node.avgSpeed));
    writer.Key("trafficUsed");
    writer.Int(node.totalRecvBytes);
}

std::string ssrspeed_generate_results(std::vector<nodeInfo> &nodes)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    nodeInfo *node = nullptr;

    writer.StartObject();
    writer.Key("status");
    writer.String(start_flag ? "running" : "stopped");
    writer.Key("current");
    writer.StartObject();
    for(nodeInfo &x : nodes)
    {
        if(x.id == cur_node_id)
        {
            node = &x;
        }
        if(x.id == current_node.id)
        {
            current_node = x;
        }
    }
    if(node && node->linkType != -1)
        json_write_node(writer, *node);
    if(node && (current_node.groupID != node->groupID || current_node.id != node->id))
    {
        if(current_node.linkType != -1)
        {
            testedNodes.push_back(current_node);
        }
        current_node = *node;
    }
    writer.EndObject();

    writer.Key("results");
    writer.StartArray();
    for(nodeInfo &x : testedNodes)
    {
        writer.StartObject();
        json_write_node(writer, x);
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    return sb.GetString();
}

std::string ssrspeed_generate_web_configs(std::vector<nodeInfo> &nodes)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartArray();
    for(nodeInfo &x : nodes)
    {
        writer.StartObject();
        writer.Key("type");
        switch(x.linkType)
        {
        case SPEEDTEST_MESSAGE_FOUNDSS:
            writer.String("Shadowsocks");
            break;
        case SPEEDTEST_MESSAGE_FOUNDSSR:
            writer.String("ShadowsocksR");
            break;
        case SPEEDTEST_MESSAGE_FOUNDVMESS:
            writer.String("V2Ray");
            break;
        case SPEEDTEST_MESSAGE_FOUNDTROJAN:
            writer.String("Trojan");
            break;
        default:
            writer.String("Unknown");
            writer.EndObject();
            continue;
        }
        writer.Key("config");
        writer.StartObject();
        writer.Key("group");
        writer.String(x.group.data());
        writer.Key("remarks");
        writer.String(x.remarks.data());
        writer.Key("server_port");
        writer.Int(x.port);
        writer.Key("server");
        writer.String(x.server.data());
        writer.EndObject();
        writer.EndObject();
    }
    writer.EndArray();
    return sb.GetString();
}

std::string ssrspeed_generate_color()
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    writer.StartArray();
    writer.StartObject();
    writer.Key("name");
    writer.String("original");
    writer.Key("colors");
    writer.StartObject();
    writer.EndObject();
    writer.EndObject();
    writer.StartObject();
    writer.Key("name");
    writer.String("rainbow");
    writer.Key("colors");
    writer.StartObject();
    writer.EndObject();
    writer.EndObject();
    if(custom_color_bounds.size() && custom_color_groups.size())
    {
        writer.StartObject();
        writer.Key("name");
        writer.String("custom");
        writer.Key("colors");
        writer.StartObject();
        writer.EndObject();
        writer.EndObject();
    }
    writer.EndArray();
    return sb.GetString();
}

void ssrspeed_webserver_routine(const std::string &listen_address, int listen_port)
{
    listener_args args = {listen_address, listen_port, 10, 4};
    extern bool gServeFile;
    extern std::string gServeFileRoot;
    gServeFile = true;
    gServeFileRoot = "webui/";

    append_response("GET", "/status", "text/plain", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        return start_flag ? "running" : "stopped";
    });

    /*append_response("GET", "/", "REDIRECT", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        return "http://web1.ospf.in/";
    });*/

    append_response("GET", "/favicon.ico", "x-icon", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        return fileGet("tools/gui/favicon.ico", true);
    });

    append_response("GET", "/getversion", "text/plain", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        return "{\"main\":\"" VERSION "\",\"webapi\":\"0.6.1\"}";
    });

    append_response("GET", "/getcolors", "text/plain", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        return ssrspeed_generate_color();
    });

    append_response("POST", "/readsubscriptions", "text/plain;charset=utf-8", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        if(start_flag)
            return "running";
        rapidjson::Document json;
        std::string suburl;
        json.Parse(request.postdata.data());
        suburl = GetMember(json, "url");
        eraseElements(allNodes);
        addNodes(suburl, false);
        return ssrspeed_generate_web_configs(allNodes);
    });

    append_response("POST", "/readfileconfig", "text/plain", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        eraseElements(allNodes);
        //fileWrite("received.txt", getFormData(postdata), true);
        if(start_flag)
            return "running";
        else
        {
            if(explodeConfContent(getFormData(request.postdata), override_conf_port, ss_libev, ssr_libev, allNodes) == SPEEDTEST_ERROR_UNRECOGFILE)
                return "error";
            else
                return ssrspeed_generate_web_configs(allNodes);
        }
    });

    append_response("POST", "/start", "text/plain", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        if(start_flag)
            return "running";
        time_t cur_time = time(NULL);
        if(cur_time - done_time < 5)
            return "done";
        start_flag = true;
        rapidjson::Document json;
        json.Parse(request.postdata.data());
        std::string test_mode = GetMember(json, "testMode"), sort_method = GetMember(json, "sortMethod"), group = GetMember(json, "group"), exp_color = GetMember(json, "colors");

        if(test_mode == "ALL")
            speedtest_mode = "all";
        else if(test_mode == "TCP_PING")
            speedtest_mode = "pingonly";
        std::transform(sort_method.begin(), sort_method.end(), sort_method.begin(), ::tolower);
        export_sort_method = replace_all_distinct(sort_method, "reverse_", "r");
        custom_group = group;
        if(exp_color.size())
            export_color_style = exp_color;

        ssrspeed_regenerate_node_list(json);
        batchTest(targetNodes);
        done_time = time(NULL);
        start_flag = false;
        return "done";
    });

    append_response("GET", "/getresults", "text/plain;charset=utf-8", [](RESPONSE_CALLBACK_ARGS) -> std::string
    {
        return ssrspeed_generate_results(targetNodes);
    });

    std::cerr << "Stair Speedtest " VERSION " Web server running @ http://" << listen_address << ":" << listen_port << std::endl;
    start_web_server_multi(&args);
}
