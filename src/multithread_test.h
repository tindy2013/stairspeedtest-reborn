#ifndef MULTITHREAD_TEST_H_INCLUDED
#define MULTITHREAD_TEST_H_INCLUDED

#include <string>

#include "misc.h"
#include "nodeinfo.h"

int perform_test(nodeInfo &node, std::string localaddr, int localport, std::string username, std::string password, int thread_count);
int upload_test(nodeInfo &node, std::string localaddr, int localport, std::string username, std::string password);
int upload_test_curl(nodeInfo &node, std::string localaddr, int localport, std::string username, std::string password);
int sitePing(nodeInfo &node, std::string localaddr, int localport, std::string username, std::string password, std::string target);

#endif // MULTITHREAD_TEST_H_INCLUDED
