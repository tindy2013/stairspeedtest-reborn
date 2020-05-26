#ifndef PRINTMSG_H_INCLUDED
#define PRINTMSG_H_INCLUDED

#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>

#include "misc.h"
#include "nodeinfo.h"

std::string lookUp(int index, bool rpcmode);

template <typename... Ts> void printMsg(int index, bool rpcmode, Ts&&... args)
{
    const std::vector<std::string> args_vec = {args...};
    std::string printout = lookUp(index, rpcmode);
    if(printout.size() == 0)
        return;
    for(size_t i = 0; i < args_vec.size(); i++)
        printout = replace_all_distinct(printout, "?" + std::to_string(i) + "?", args_vec[i]);
    if(rpcmode)
        printout = replace_all_distinct(printout, "\\", "\\\\");
    std::cout<<printout;
    std::cout.clear();
    std::cout.flush();
}

/*
void printMsg(int index, nodeInfo &node, bool rpcmode);
void printMsgWithDict(int index, bool rpcmode, std::vector<std::string> dict, std::vector<std::string> trans);
void printMsgDirect(int index, bool rpcmode);
*/

#endif // PRINTMSG_H_INCLUDED
