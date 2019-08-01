#ifndef WEBGET_H_INCLUDED
#define WEBGET_H_INCLUDED

#include <bits/stdc++.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#endif // _WIN32

#ifndef CURLINFO_TOTAL_TIME_T
#define CURLINFO_TOTAL_TIME_T CURLINFO_TOTAL_TIME
#endif // CURLINFO_TOTAL_TIME_T

#include "misc.h"
#include "socket.h"

using namespace std;

string webGet(string url);
string httpGet(string host, string addr, string uri);
string httpsGet(string host, string addr, string uri);
int website_ping(nodeInfo *node, string url, string local_addr, int local_port, string user, string pass);

#endif // WEBGET_H_INCLUDED
