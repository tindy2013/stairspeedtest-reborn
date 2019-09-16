#ifndef PERF_TEST_H_INCLUDED
#define PERF_TEST_H_INCLUDED
#include "socket.h"
#include "misc.h"

void testTelegram(string localaddr, int localport, nodePerfInfo *node);
void testCloudflare(string localaddr, int localport, nodePerfInfo *node);

#endif // PERF_TEST_H_INCLUDED
