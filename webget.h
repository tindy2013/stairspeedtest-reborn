#ifndef WEBGET_H_INCLUDED
#define WEBGET_H_INCLUDED

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif // _WIN32

#ifndef CURLINFO_TOTAL_TIME_T
#define CURLINFO_TOTAL_TIME_T CURLINFO_TOTAL_TIME
#endif // CURLINFO_TOTAL_TIME_T

#include "misc.h"
#include "nodeinfo.h"

std::string webGet(std::string url, std::string proxy = "");
std::string httpGet(std::string host, std::string addr, std::string uri);
std::string httpsGet(std::string host, std::string addr, std::string uri);
long curlPost(std::string url, std::string data, std::string proxy);
int websitePing(nodeInfo *node, std::string url, std::string local_addr, int local_port, std::string user, std::string pass);
std::string buildSocks5ProxyString(std::string addr, int port, std::string username, std::string password);

#endif // WEBGET_H_INCLUDED
