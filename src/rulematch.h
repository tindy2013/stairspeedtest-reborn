#ifndef RULEMATCH_H_INCLUDED
#define RULEMATCH_H_INCLUDED

#include <string>
#include <vector>

struct downloadLink
{
    std::string url;
    std::string tag;
};

struct linkMatchRule
{
    std::string mode;
    std::vector<std::string> rules;
    std::string tag;
};

#endif // RULEMATCH_H_INCLUDED
