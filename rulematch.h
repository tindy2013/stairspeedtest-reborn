#ifndef RULEMATCH_H_INCLUDED
#define RULEMATCH_H_INCLUDED

#include <string>
#include <vector>

using namespace std;

struct downloadLink
{
    string url;
    string tag;
};

struct linkMatchRule
{
    string mode;
    vector<string> rules;
    string tag;
};

#endif // RULEMATCH_H_INCLUDED
