#ifndef SPEEDTESTUTIL_H_INCLUDED
#define SPEEDTESTUTIL_H_INCLUDED

#include <string>
#include <yaml-cpp/yaml.h>

#include "misc.h"

string vmessConstruct(string add, string port, string type, string id, string aid, string net, string cipher, string path, string host, string tls, int local_port);
string ssrConstruct(string group, string remarks, string remarks_base64, string server, string port, string protocol, string method, string obfs, string password, string obfsparam, string protoparam, int local_port, bool libev);
string ssConstruct(string server, string port, string password, string method, string plugin, string pluginopts, string remarks, int local_port, bool libev);
void explodeVmess(string vmess, string custom_port, int local_port, nodeInfo *node);
void explodeSSR(string ssr, bool libev, string custom_port, int local_port, nodeInfo *node);
void explodeSS(string ss, bool libev, string custom_port, int local_port, nodeInfo *node);
void explodeQuan(string quan, string custom_port, int local_port, nodeInfo *node);
void explode(string link, bool sslibev, bool ssrlibev, string custom_port, int local_port, nodeInfo *node);
void explodeSSD(string link, bool libev, string custom_port, int local_port, vector<nodeInfo> *nodes);
void explodeSub(string sub, bool sslibev, bool ssrlibev, string custom_port, int local_port, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks);
int explodeConf(string filepath, string custom_port, int local_port, bool sslibev, bool ssrlibev, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks);
int explodeConfContent(string content, string custom_port, int local_port, bool sslibev, bool ssrlibev, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks);

#endif // SPEEDTESTUTIL_H_INCLUDED
