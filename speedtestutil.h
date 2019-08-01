#ifndef SPEEDTESTUTIL_H_INCLUDED
#define SPEEDTESTUTIL_H_INCLUDED

#include <string>
#include <yaml-cpp/yaml.h>

#include "misc.h"

void explodevmess(string vmess, string custom_port, int local_port, nodeInfo *node);
void explodessr(string ssr, bool libev, string custom_port, int local_port, nodeInfo *node);
void explodess(string ss, bool libev, string custom_port, int local_port, nodeInfo *node);
void explodequan(string quan, string custom_port, int local_port, nodeInfo *node);
void explode(string link, bool sslibev, bool ssrlibev, string custom_port, int local_port, nodeInfo *node);
void explodessd(string link, bool libev, string custom_port, int local_port, vector<nodeInfo> *nodes);
void explodesub(string sub, bool sslibev, bool ssrlibev, string custom_port, int local_port, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks);
int explodeconf(string filepath, string custom_port, int local_port, bool sslibev, bool ssrlibev, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks);
int explodeconfcontent(string content, string custom_port, int local_port, bool sslibev, bool ssrlibev, vector<nodeInfo> *nodes, vector<string> *exclude_remarks, vector<string> *include_remarks);

#endif // SPEEDTESTUTIL_H_INCLUDED
