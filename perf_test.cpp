#include "misc.h"
#include "socket.h"
#include "webget.h"
#include "printout.h"
#include "perf_test.h"

void testTelegram(string localaddr, int localport, nodePerfInfo *node)
{
    string telegram_host = "http://91.108.56.180:80";
    nodeInfo tmpNode;
    if(node->linkType == SPEEDTEST_MESSAGE_FOUNDSOCKS)
        websitePing(&tmpNode, telegram_host, node->server, node->port, UrlDecode(getUrlArg(node->proxyStr, "user")), UrlDecode(getUrlArg(node->proxyStr, "pass")));
    else
        websitePing(&tmpNode, telegram_host, localaddr, localport, "", "");
    copy(begin(tmpNode.rawSitePing), end(tmpNode.rawSitePing), begin(node->rawTelegramPing));
    node->telegramPing = tmpNode.sitePing;
}

void testCloudflare(string localaddr, int localport, nodePerfInfo *node)
{
    string cloudflare_host = "https://1.1.1.1";
    nodeInfo tmpNode;
    if(node->linkType == SPEEDTEST_MESSAGE_FOUNDSOCKS)
        websitePing(&tmpNode, cloudflare_host, node->server, node->port, UrlDecode(getUrlArg(node->proxyStr, "user")), UrlDecode(getUrlArg(node->proxyStr, "pass")));
    else
        websitePing(&tmpNode, cloudflare_host, localaddr, localport, "", "");
    copy(begin(tmpNode.rawSitePing), end(tmpNode.rawSitePing), begin(node->rawCloudflarePing));
    node->cloudflarePing = tmpNode.sitePing;
}

