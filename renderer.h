#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <bits/stdc++.h>
#include <pngwriter.h>
#include <zlib.h>
#include <zconf.h>
#include <chrono>

#include "misc.h"
#include "logger.h"

using namespace std;

#define MAX_COLOR_COUNT 16

extern int colorgroup[MAX_COLOR_COUNT][3];
extern int bounds[MAX_COLOR_COUNT];
extern int color_count;

string exportRender(string resultpath, vector<nodeInfo> nodes, bool export_with_maxspeed, string export_sort_method);

#endif // RENDERER_H_INCLUDED
