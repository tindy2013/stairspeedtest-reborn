#ifndef MULTITHREAD_TEST_H_INCLUDED
#define MULTITHREAD_TEST_H_INCLUDED

#include <string>
#include "misc.h"

using namespace std;

int perform_test(nodeInfo *node, string localaddr, int localport, string username, string password, int thread_count);
int upload_test(nodeInfo *node, string localaddr, int localport, string username, string password);
int upload_test_curl(nodeInfo *node, string localaddr, int localport, string username, string password);
int sitePing(nodeInfo *node, string localaddr, int localport, string username, string password, string target);

#endif // MULTITHREAD_TEST_H_INCLUDED
