#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <string>
#include <vector>

#include "misc.h"
#include "logger.h"
#include "nodeinfo.h"

#define MAX_NODES_COUNT 1024

struct color
{
    int red = 0;
    int green = 0;
    int blue = 0;
};

extern std::vector<color> colorgroup;
extern std::vector<int> bounds;
extern bool export_as_ssrspeed;

std::string exportRender(std::string resultpath, std::vector<nodeInfo> &nodes, bool export_with_maxspeed, std::string export_sort_method, std::string export_color_style, bool export_as_new_style, bool export_nat_type = true);

#endif // RENDERER_H_INCLUDED
