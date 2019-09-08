#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <string>
#include <vector>

#include "misc.h"
#include "logger.h"

using namespace std;

struct color
{
    int red = 0;
    int green = 0;
    int blue = 0;
};

extern vector<color> colorgroup;
extern vector<int> bounds;

string exportRender(string resultpath, vector<nodeInfo> nodes, bool export_with_maxspeed, string export_sort_method, string export_color_style, bool export_as_new_style, int test_duration);

#endif // RENDERER_H_INCLUDED
