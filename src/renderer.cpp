#include <algorithm>
#include <chrono>

#include <pngwriter.h>
#include <zlib.h>
#include <zconf.h>

#include "renderer.h"
#include "version.h"
#include "nodeinfo.h"

using namespace std::chrono;
using namespace std::__cxx11;

std::string export_sort_method_render = "none";
bool export_as_ssrspeed = false;

std::vector<color> colorgroup;
std::vector<int> bounds;

//renderer values
int widNumber = 0, widNA = 0, widB = 0, widKB = 0, widMB = 0, widGB = 0, widPercent = 0, widDot = 0;

//original color
const int def_colorgroup[5][3] = {{65535, 65535, 65535}, {32768, 65535, 0}, {65535, 65535, 0}, {65535, 32768, 49152}, {65535, 0, 0}};
const int def_bounds[5] = {0, 64 * 1024, 512 * 1024, 4 * 1024 * 1024, 16 * 1024 * 1024};

//rainbow color
const int rainbow_colorgroup[8][3] = {{65535, 65535, 65535}, {26112, 65535, 26112}, {65535, 65535, 26112}, {65535, 45568, 26112}, {65535, 26112, 26112}, {57856, 35840, 65535}, {26112, 52224, 65535}, {26112, 26112, 65535}};
const int rainbow_bounds[8] = {0, 64 * 1024, 512 * 1024, 4 * 1024 * 1024, 16 * 1024 * 1024, 24 * 1024 * 1024, 32 * 1024 * 1024, 40 * 1024 * 1024};

int calcLength(std::string data)
{
    int total = 0;
    for(unsigned int i = 0; i < data.size(); i++)
    {
        if(int(data[i]) > 255 || int(data[i]) < 0)
            total += 2;
        else
            total++;
    }
    return total;
}

int getTextLength(std::string str)
{
    return ((calcLength(str) - str.size()) / 3) * 2 + (str.size() * 2 - calcLength(str)) - count(str.begin(), str.end(), ' ') / 2;
}

/*
static inline int calcCharCount(std::string data, int type)
{
    int uBound, lBound, total = 0;
    switch(type)
    {
    case 0: //number
        uBound = 57;
        lBound = 48;
        break;
    case 3: //percent
        uBound = 37;
        lBound = 37;
        break;
    case 4: //dot
        uBound = 56;
        lBound = 56;
        break;
    default: //all basic chars
        uBound = 255;
        lBound = 0;
        break;
    }

    for(unsigned int i = 0; i < data.size(); i++)
    {
        if(int(data[i]) >= lBound && int(data[i]) <= uBound)
            total++;
    }
    return total;
}
*/

static inline int getWidth(pngwriter *png, std::string font, int fontsize, std::string text)
{
    return png->get_text_width_utf8(const_cast<char *>(font.data()), fontsize, const_cast<char *>(text.data()));
    //const int widChnChar = 17, widEngChar = 9;
    //return ((calcLength(text) - text.size()) / 3) * widChnChar + ((text.size() * 2 - calcLength(text)) - count(text.begin(), text.end(), ' ') / 2) * widEngChar;
}

void rendererInit(std::string font, int fontsize)
{
    pngwriter png;
    writeLog(LOG_TYPE_RENDER, "Start calculating basic string widths for font '" + font + "' at size " + to_string(fontsize) + ".");
    widNumber = getWidth(&png, font, fontsize, "1");
    widNA = getWidth(&png, font, fontsize, "N/A");
    widB = getWidth(&png, font, fontsize, "B");
    widKB = getWidth(&png, font, fontsize, "KB");
    widMB = getWidth(&png, font, fontsize, "MB");
    widGB = getWidth(&png, font, fontsize, "GB");
    widPercent = getWidth(&png, font, fontsize, "%");
    widDot = getWidth(&png, font, fontsize, ".");
    writeLog(LOG_TYPE_RENDER, "Calculated basic string widths: Number=" + to_string(widNumber) + " N/A=" + to_string(widNA) + " KB=" + to_string(widKB) \
             + " MB=" + to_string(widMB) + " GB=" + to_string(widGB) + " Percent=" + to_string(widPercent) + " Dot=" + to_string(widDot));
}

static inline int getTextWidth(pngwriter *png, std::string font, int fontsize, std::string text)
{
    int cntNumber = 0, total_width = 0;

    if(text == "N/A")
        return widNA;

    for(unsigned int i = 0; i < text.size(); i++)
    {
        if(int(text[i]) >= 48 && int(text[i]) <= 57)
            cntNumber++;
    }
    total_width = cntNumber * widNumber + widDot;
    //if(strFind(text, "."))
    //total_width += widDot;

    if(strFind(text, "%"))
        total_width += widPercent;
    else
    {
        if(strFind(text, "MB"))
            total_width += widMB;
        else if(strFind(text, "KB"))
            total_width += widKB;
        else if(strFind(text, "GB"))
            total_width += widGB;
        else if(strFind(text, "B"))
            total_width += widB;
    }

    return total_width;
}

/*
static inline int getTextWidth(pngwriter *png, std::string font, int fontsize, std::string text)
{
    return png->get_text_width_utf8(const_cast<char *>(font.data()), fontsize, const_cast<char *>(text.data()));
}
*/
/*
static inline int getTextWidth(pngwriter *png, std::string font, int fontsize, std::string text)
{
    return calcCharCount(text, 0) * widNumber + calcCharCount(text, 1) * widUpperLetter + calcCharCount(text, 2) * widLowerLetter + calcCharCount(text, 3) * widPercent \
    + calcCharCount(text, 4) * widDot + calcCharCount(text, 5) * widSpace;
}
*/

static inline void plot_text_utf8(pngwriter *png, std::string face_path, int fontsize, int x_start, int y_start, double angle, std::string text, double red, double green, double blue)
{
    png->plot_text_utf8(const_cast<char *>(face_path.data()), fontsize, x_start, y_start, angle, const_cast<char *>(text.data()), red, green, blue);
    return;
}

std::string secondToString(int duration)
{
    int intHrs = duration / 3600;
    int intMin = (duration % 3600) / 60;
    int intSec = duration % 60;
    std::string strHrs = intHrs > 9 ? to_string(intHrs) : "0" + to_string(intHrs);
    std::string strMin = intMin > 9 ? to_string(intMin) : "0" + to_string(intMin);
    std::string strSec = intSec > 9 ? to_string(intSec) : "0" + to_string(intSec);
    return strHrs + ":" + strMin + ":" + strSec;
}

int getSpeed(std::string speed)
{
    if(speed == "")
        return 0;
    double speedval = 1.0;
    if(speed.find("MB") != std::string::npos)
        speedval = 1048576.0 * stof(speed.substr(0, speed.size() - 2));
    else if(speed.find("KB") != std::string::npos)
        speedval = 1024.0 * stof(speed.substr(0, speed.size() - 2));
    else if(speed.find("B") != std::string::npos)
        speedval = 1.0 * stof(speed.substr(0, speed.size() - 1));
    return (int)speedval;
}

bool comparer(nodeInfo &a, nodeInfo &b)
{
    if(export_sort_method_render == "speed")
        return getSpeed(a.avgSpeed) < getSpeed(b.avgSpeed);
    else if(export_sort_method_render == "rspeed")
        return getSpeed(a.avgSpeed) > getSpeed(b.avgSpeed);
    else if(export_sort_method_render == "ping")
        return stof(a.avgPing) < stof(b.avgPing);
    else if(export_sort_method_render == "rping")
        return stof(a.avgPing) > stof(b.avgPing);
    else
        return a.groupID < b.groupID || a.id < b.id;
}

void getColor(color lc, color rc, float level, color *finalcolor)
{
    finalcolor->red = (int)((float)lc.red * (1.0 - level) + (float)rc.red * level);
    finalcolor->green = (int)((float)lc.green * (1.0 - level) + (float)rc.green * level);
    finalcolor->blue = (int)((float)lc.blue * (1.0 - level) + (float)rc.blue * level);
}

color arrayToColor(const int colors[3])
{
    color retcolor;
    retcolor.red = colors[0];
    retcolor.green = colors[1];
    retcolor.blue = colors[2];
    return retcolor;
}

void getSpeedColor(std::string speed, color *finalcolor)
{
    int speedval = getSpeed(speed);
    unsigned int color_count = colorgroup.size();
    for(unsigned int i = 0; i < color_count - 1; i++)
    {
        if(speedval >= bounds[i] && speedval <= bounds[i + 1])
        {
            getColor(colorgroup[i], colorgroup[i + 1], ((float)speedval - (float)bounds[i]) / ((float)bounds[i + 1] - (float)bounds[i]), finalcolor);
            return;
        }
    }
    getColor(colorgroup[color_count - 1], colorgroup[color_count - 1], 1, finalcolor);
    return;
}

void loadDefaultColor(std::string type)
{
    if(type == "rainbow")
    {
        std::vector<color>().swap(colorgroup);
        std::vector<int>().swap(bounds);
        for(int i = 0; i < 8; i++)
        {
            colorgroup.push_back(arrayToColor(rainbow_colorgroup[i]));
            bounds.push_back(rainbow_bounds[i]);
        }
    }
    else if(type == "original")
    {
        std::vector<color>().swap(colorgroup);
        std::vector<int>().swap(bounds);
        for(int i = 0; i < 5; i++)
        {
            colorgroup.push_back(arrayToColor(def_colorgroup[i]));
            bounds.push_back(def_bounds[i]);
        }
    }
}

/*
void test()
{
    pngwriter png;
    rendererInit("tools\\misc\\WenQuanYiMicroHei-01.ttf", 12);
    cout<<getWidth(&png, "tools\\misc\\WenQuanYiMicroHei-01.ttf", 12, "KB")<<" "<<getTextWidth(&png, "tools\\misc\\WenQuanYiMicroHei-01.ttf", 12, "KB")<<endl;
}
*/
#ifndef _FAST_RENDER

std::string exportRender(std::string resultpath, std::vector<nodeInfo> nodes, bool export_with_maxSpeed, std::string export_sort_method, std::string export_color_style, bool export_as_new_style)
{
    std::string pngname = replace_all_distinct(resultpath, ".log", ".png");
    nodeInfo node;
    int total_width = 0, total_height = 0, node_count = 0, total_line = 0;
    color bg_color;
    pngwriter png;
    loadDefaultColor(export_color_style);

    //predefined values
    std::string font = "tools" PATH_SLASH "misc" PATH_SLASH "WenQuanYiMicroHei-01.ttf";

    int fontsize = 12, text_x_offset = 5, height_line = 24, text_y_offset = 7;
    double border_red = 0.8, border_green = 0.8, border_blue = 0.8;
    if(export_as_new_style)
    {
        height_line = 30;
        text_y_offset = 10;
    }
    const int center_align_offset = 8, vertical_delim_align_offset = 2;
    const double text_red = 0.0, text_green = 0.0, text_blue = 0.0;
    //extra value for aligning to the center
    const int enableCenterAlign = export_as_new_style ? 1 : 0;
    const int center_align_offset_side = center_align_offset / 2;
#define calcCenterOffset(item, total) ((((total - item) / 2) - center_align_offset_side) * enableCenterAlign)

    //SSRSpeed style
    if(export_as_ssrspeed)
    {
        export_as_new_style = true;
        font = "tools" PATH_SLASH "misc" PATH_SLASH "SourceHanSansCN-Medium.otf";
        fontsize = 13;
        height_line = 30;
        text_y_offset = 7;
        border_red = 0.5;
        border_green = 0.5;
        border_blue = 0.5;
    }

    //initialize all values
    rendererInit(font, fontsize);
    export_sort_method_render = export_sort_method;
    node_count = nodes.size();
    total_line = node_count + 4;
    total_height = height_line * total_line;
    std::sort(nodes.begin(), nodes.end(), comparer); //sort by export_sort_method

    //add title line into the list
    node.group = "Group";
    node.remarks = "Remarks";
    if(export_as_new_style)
    {
        node.pkLoss = "     Loss     ";
        node.avgPing = "     Ping     ";
        node.sitePing = "  Google Ping  ";
        node.avgSpeed = "  AvgSpeed  ";
        node.maxSpeed = "  MaxSpeed  ";
    }
    else
    {
        node.pkLoss = "Pk.Loss";
        node.avgPing = "TCP Ping";
        node.sitePing = "Google Ping";
        node.avgSpeed = "Avg.Speed";
        node.maxSpeed = "Max.Speed";
    }
    nodes.insert(nodes.begin(), node);

    //calculate the width of all columns
    int group_width = 0, remarks_width = 0, pkLoss_width = 0, avgPing_width = 0, avgSpeed_width = 0,  sitePing_width = 0, maxSpeed_width = 0, onlines = 0, final_width = 0, test_duration = 0;
    int group_widths[MAX_NODES_COUNT], remarks_widths[MAX_NODES_COUNT], pkLoss_widths[MAX_NODES_COUNT], avgPing_widths[MAX_NODES_COUNT], avgSpeed_widths[MAX_NODES_COUNT], sitePing_widths[MAX_NODES_COUNT], maxSpeed_widths[MAX_NODES_COUNT];
    long long total_traffic = 0;
    std::string longest_group, longest_remarks;
    int longest_group_len = 0, longest_remarks_len = 0;

    for(int i = 0; i <= node_count; i++)
    {
        //store them all into arrays first
        //don't calculate all remarks and group widths
        //instead we use longest group/remarks to calculate width
        //group_widths[i] = getTextWidth(&png, font, fontsize, nodes[i].group);
        //remarks_widths[i] = getTextWidth(&png, font, fontsize, nodes[i].remarks);
        if(getTextLength(nodes[i].group) > longest_group_len)
        {
            longest_group = nodes[i].group;
            longest_group_len = getTextLength(longest_group);
        }
        if(getTextLength(nodes[i].remarks) > longest_remarks_len)
        {
            longest_remarks = nodes[i].remarks;
            longest_remarks_len = getTextLength(longest_remarks);
        }
        if(i == 0)
        {
            pkLoss_widths[i] = getWidth(&png, font, fontsize, nodes[i].pkLoss);
            avgPing_widths[i] = getWidth(&png, font, fontsize, nodes[i].avgPing);
            avgSpeed_widths[i] = getWidth(&png, font, fontsize, nodes[i].avgSpeed);
            if(export_as_new_style)
                sitePing_widths[i] = getWidth(&png, font, fontsize, nodes[i].sitePing);
            if(export_with_maxSpeed)
                maxSpeed_widths[i] = getWidth(&png, font, fontsize, nodes[i].maxSpeed);
        }
        else
        {
            pkLoss_widths[i] = getTextWidth(&png, font, fontsize, nodes[i].pkLoss);
            avgPing_widths[i] = getTextWidth(&png, font, fontsize, nodes[i].avgPing);
            avgSpeed_widths[i] = getTextWidth(&png, font, fontsize, nodes[i].avgSpeed);
            if(export_as_new_style)
                sitePing_widths[i] = getTextWidth(&png, font, fontsize, nodes[i].sitePing);
            if(export_with_maxSpeed)
                maxSpeed_widths[i] = getTextWidth(&png, font, fontsize, nodes[i].maxSpeed);
        }
        //group_width = max(group_widths[i] + center_align_offset, group_width);
        //remarks_width = max(remarks_widths[i] + center_align_offset, remarks_width);
        pkLoss_width = std::max(pkLoss_widths[i] + center_align_offset, pkLoss_width);
        avgPing_width = std::max(avgPing_widths[i] + center_align_offset, avgPing_width);
        if(export_as_new_style)
            sitePing_width = std::max(sitePing_widths[i] + center_align_offset, sitePing_width);
        avgSpeed_width = std::max(avgSpeed_widths[i] + center_align_offset, avgSpeed_width);
        if(export_with_maxSpeed)
            maxSpeed_width = std::max(maxSpeed_widths[i] + center_align_offset, maxSpeed_width);

        total_traffic += nodes[i].totalRecvBytes;
        test_duration += nodes[i].duration;
        if(nodes[i].online)
            onlines++;
    }
    //only calculate the width of the group/remark title line
    remarks_widths[0] = getWidth(&png, font, fontsize, node.remarks);
    remarks_width = std::max(getWidth(&png, font, fontsize, longest_remarks) + center_align_offset, remarks_widths[0] + center_align_offset) + 4;
    group_widths[0] = getWidth(&png, font, fontsize, node.group);
    group_width = std::max(getWidth(&png, font, fontsize, longest_group) + center_align_offset, group_widths[0] + center_align_offset) + 4;

    int width_all[8] = {0, group_width, remarks_width, pkLoss_width, avgPing_width, sitePing_width, avgSpeed_width, maxSpeed_width}; //put them into an array for reading
    //int width_all[7] = {0, group_width, remarks_width, pkLoss_width, avgPing_width, avgSpeed_width, maxSpeed_width}; //put them into an array for reading
    total_width = group_width + remarks_width + pkLoss_width + avgPing_width + sitePing_width + avgSpeed_width;
    if(export_with_maxSpeed)
        total_width += maxSpeed_width;

    //generating information
    std::string gentime = "Generated at " + getTime(3);
    std::string traffic = "Traffic used : " + speedCalc((double)total_traffic) + ". ";
    std::string about = "By Stair Speedtest Reborn " VERSION ".";
    std::string title = "  Stair Speedtest Reborn Result Table ( " VERSION " )  ";
    //SSRSpeed style
    if(export_as_ssrspeed)
    {
        traffic += "Time used: " + secondToString(test_duration) + ". Online Node(s) : [" + to_string(onlines) + "/" + to_string(node_count) + "]";
        title = "  SSRSpeed Result Table ( v2.6.2 )  ";
    }
    else
    {
        if(export_as_new_style)
            traffic += "Time used : " + secondToString(test_duration) + ". ";
        traffic += "Working Node(s) : [" + to_string(onlines) + "/" + to_string(node_count) + "]";
    }

    final_width = total_width;
    final_width = std::max(getWidth(&png, font, fontsize, gentime) + center_align_offset, final_width);
    final_width = std::max(getWidth(&png, font, fontsize, traffic) + center_align_offset, final_width);
    if(export_as_new_style)
        final_width = std::max(getWidth(&png, font, fontsize, title) + center_align_offset, final_width);
    if(final_width > total_width)
        width_all[2] += final_width - total_width;
    total_width = final_width;

    //initialize the file
    //pngwriter png(total_width, total_height, 1.0, pngname.data());
    png = pngwriter(total_width, total_height, 1.0, pngname.data());
    //then draw from the bottom
    int line_index = 0;
    if(!export_as_new_style)
    {
        //about message
        plot_text_utf8(&png, font, fontsize, text_x_offset, text_y_offset + line_index * height_line, 0.0, about, text_red, text_green, text_blue);
        line_index++;
        png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter
    }
    //generate time
    plot_text_utf8(&png, font, fontsize, text_x_offset, text_y_offset + line_index * height_line, 0.0, gentime, text_red, text_green, text_blue);
    line_index++;
    png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter
    //traffic and online nodes
    plot_text_utf8(&png, font, fontsize, text_x_offset, text_y_offset + line_index * height_line, 0.0, traffic, text_red, text_green, text_blue);
    line_index++;
    png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter

    //now draw all the nodes
    int this_x_offset = 0, this_y_offset = 0, line_offset = vertical_delim_align_offset, j = 0;
    for(int i = node_count; i >= 0; i--)
    {
        this_x_offset = text_x_offset + width_all[j];
        this_y_offset = line_index * height_line + text_y_offset;
        //group name
        //don't align group except title
        if(i > 0)
            plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].group, text_red, text_green, text_blue);
        else
            plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(group_widths[i], group_width) - (export_as_ssrspeed ? 4 : 0), this_y_offset, 0.0, nodes[i].group, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //remarks
        //don't align remarks except title
        if(i > 0)
            plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].remarks, text_red, text_green, text_blue);
        else
            plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(remarks_widths[i], remarks_width), this_y_offset, 0.0, nodes[i].remarks, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //packet loss
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(pkLoss_widths[i], pkLoss_width), this_y_offset, 0.0, nodes[i].pkLoss, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //average ping
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(avgPing_widths[i], avgPing_width), this_y_offset, 0.0, nodes[i].avgPing, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        if(export_as_new_style)
        {
            //site ping
            plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(sitePing_widths[i], sitePing_width), this_y_offset, 0.0, nodes[i].sitePing, text_red, text_green, text_blue);
            j++;
            line_offset += width_all[j];
            png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
            this_x_offset += width_all[j];
        }
        else
            j++;

        //draw color background
        if(i > 0)
        {
            getSpeedColor(nodes[i].avgSpeed, &bg_color);
            png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1] - 1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
        }
        //average speed
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(avgSpeed_widths[i], avgSpeed_width), (i > 0 && export_as_ssrspeed) ? this_y_offset + 3 : this_y_offset, 0.0, nodes[i].avgSpeed, text_red, text_green, text_blue);
        if(export_with_maxSpeed) //see if we want to draw max speed
        {
            j++;
            line_offset += width_all[j];
            png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
            this_x_offset += width_all[j];
            //draw color background
            if(i > 0)
            {
                getSpeedColor(nodes[i].maxSpeed, &bg_color);
                png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1] - 1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
            }
            //max speed
            plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(maxSpeed_widths[i], maxSpeed_width), (i > 0 && export_as_ssrspeed) ? this_y_offset + 3 : this_y_offset, 0.0, nodes[i].maxSpeed, text_red, text_green, text_blue);
        }
        line_index++; //one line completed,  moving up
        png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter
        j = 0;
        line_offset = vertical_delim_align_offset;
    }
    if(export_as_new_style)
    {
        //title
        plot_text_utf8(&png, font, fontsize, text_x_offset + calcCenterOffset(getWidth(&png, font, fontsize, title), total_width), text_y_offset + line_index * height_line, 0.0, title, text_red, text_green, text_blue);
    }
    //basic border
    png.line(1, 1, total_width, 1, border_red, border_green, border_blue);//bottom
    png.line(1, total_height, total_width, total_height, border_red, border_green, border_blue);//top
    if(export_as_ssrspeed)
    {
        png.line(2, 1, 2, total_height, border_red, border_green, border_blue);//left, offset +1 pixel
    }
    else
    {
        png.line(1, 1, 1, total_height, border_red, border_green, border_blue);//left
    }

    png.line(total_width, 1, total_width, total_height, border_red, border_green, border_blue);//right
    png.close(); //save picture
    //all done!
    return pngname;
}

#else

// old style only since we cannot align to the center
std::string exportRender(std::string resultpath, vector<nodeInfo> nodes, bool export_with_maxSpeed, std::string export_sort_method, std::string export_color_style, bool export_as_new_style, int test_duration)
{
    std::string pngname = replace_all_distinct(resultpath, ".log", ".png");
    nodeInfo node;
    int total_width = 0, total_height = 0, node_count = 0, total_line = 0;
    color bg_color;
    pngwriter png;
    loadDefaultColor(export_color_style);

    //predefined values
    const std::string font = "tools" PATH_SLASH "misc" PATH_SLASH "WenQuanYiMicroHei-01.ttf";
    const int height_line = 24, fontsize = 12, text_x_offset = 5, text_y_offset = 7, center_align_offset = 8, vertical_delim_align_offset = 2;
    const double border_red = 0.8, border_green = 0.8, border_blue = 0.8;
    const double text_red = 0.0, text_green = 0.0, text_blue = 0.0;

    //initialize all values
    export_sort_method_render = export_sort_method;
    node_count = nodes.size();
    total_line = node_count + 4;
    total_height = height_line * total_line;
    if(export_sort_method != "none")
        sort(nodes.begin(), nodes.end(), comparer);//sort by export_sort_method

    //add title line into the list
    node.group = "Group";
    node.remarks = "Remarks";
    node.pkLoss = "Pk.Loss";
    node.avgPing = "TCP Ping";
    //node.sitePing = "Bing Ping";
    node.avgSpeed = "Avg.Speed";
    node.maxSpeed = "Max.Speed";
    nodes.insert(nodes.begin(), node);

    //calculate the width of all columns
    int group_width = 0, remarks_width = 0, pkLoss_width = 0, avgPing_width = 0, avgSpeed_width = 0, /* sitePing_width = 0, */maxSpeed_width = 0, onlines = 0, final_width = 0;
    std::string longest_group, longest_remarks, longest_pkLoss, longest_avgPing, longest_avgSpeed, /*longest_sitePing,*/longest_maxSpeed;
    long long total_traffic = 0;
    for(int i = 0; i <= node_count; i++)
    {
        //find the longest string
        if(getTextLength(nodes[i].group) > getTextLength(longest_group))
            longest_group = nodes[i].group;
        if(getTextLength(nodes[i].remarks) > getTextLength(longest_remarks))
            longest_remarks = nodes[i].remarks;
        if(getTextLength(nodes[i].pkLoss) > getTextLength(longest_pkLoss))
            longest_pkLoss = nodes[i].pkLoss;
        if(getTextLength(nodes[i].avgPing) > getTextLength(longest_avgPing))
            longest_avgPing = nodes[i].avgPing;
        if(getTextLength(nodes[i].avgSpeed) > getTextLength(longest_avgSpeed))
            longest_avgSpeed = nodes[i].avgSpeed;
        if(export_with_maxSpeed)
        {
            if(getTextLength(nodes[i].maxSpeed) > getTextLength(longest_maxSpeed))
                longest_maxSpeed = nodes[i].maxSpeed;
        }
        total_traffic += nodes[i].totalRecvBytes;
        if(nodes[i].online)
            onlines++;
    }
    //calculate the width of the longest string
    group_width = getWidth(&png, font, fontsize, longest_group) + center_align_offset;
    remarks_width = getWidth(&png, font, fontsize, longest_remarks) + center_align_offset;
    pkLoss_width = getWidth(&png, font, fontsize, longest_pkLoss) + center_align_offset;
    avgPing_width = getWidth(&png, font, fontsize, longest_avgPing) + center_align_offset;
    avgSpeed_width = getWidth(&png, font, fontsize, longest_avgSpeed) + center_align_offset;
    if(export_with_maxSpeed)
        maxSpeed_width = getWidth(&png, font, fontsize, longest_maxSpeed) + center_align_offset;

    //int width_all[8] =  {0, group_width, remarks_width, pkLoss_width, avgPing_width, sitePing_width, avgSpeed_width, maxSpeed_width}; //put them into an array for reading
    int width_all[7] =  {0, group_width, remarks_width, pkLoss_width, avgPing_width, avgSpeed_width, maxSpeed_width}; //put them into an array for reading
    total_width = group_width + remarks_width + pkLoss_width + avgPing_width + /* sitePing_width + */avgSpeed_width;
    if(export_with_maxSpeed)
        total_width += maxSpeed_width;

    //generating information
    std::string gentime = "Generated at "+getTime(3);
    std::string traffic = "Traffic used : "+speedCalc((double)total_traffic)+". Working Node(s) : ["+to_string(onlines)+"/"+to_string(node_count)+"]";
    std::string about = "By Stair Speedtest Reborn " VERSION ".";

    final_width = max(getWidth(&png, font, fontsize, gentime) + center_align_offset, total_width);
    final_width = max(getWidth(&png, font, fontsize, traffic) + center_align_offset, total_width);
    if(final_width > total_width)
        width_all[2] += final_width - total_width;
    total_width = final_width;

    //initialize the file
    png = pngwriter(total_width, total_height, 1.0, pngname.data());
    //then draw from the bottom
    int line_index = 0;
    //about message
    plot_text_utf8(&png, font, fontsize, text_x_offset, text_y_offset + line_index * height_line, 0.0, about, text_red, text_green, text_blue);
    line_index++;
    png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter
    //generate time
    plot_text_utf8(&png, font, fontsize, text_x_offset, text_y_offset + line_index * height_line, 0.0, gentime, text_red, text_green, text_blue);
    line_index++;
    png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter
    //traffic and online nodes
    plot_text_utf8(&png, font, fontsize, text_x_offset, text_y_offset + line_index * height_line, 0.0, traffic, text_red, text_green, text_blue);
    line_index++;
    png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter

    //now draw all the nodes
    int this_x_offset = 0, this_y_offset = 0, line_offset = vertical_delim_align_offset, j = 0;
    for(int i = node_count; i >= 0; i--)
    {
        this_x_offset = text_x_offset + width_all[j];
        this_y_offset = line_index * height_line + text_y_offset;
        //group name
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].group, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //remarks
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].remarks, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //packet loss
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].pkLoss, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //average ping
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].avgPing, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        /*
        //site ping, unused
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].sitePing, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        */
        //draw color background
        if(i>0)
        {
            getSpeedColor(nodes[i].avgSpeed, &bg_color);
            png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1]-1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
        }
        //average speed
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].avgSpeed, text_red, text_green, text_blue);
        if(export_with_maxSpeed) //see if we want to draw max speed
        {
            j++;
            line_offset += width_all[j];
            png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
            this_x_offset += width_all[j];
            //draw color background
            if(i>0)
            {
                getSpeedColor(nodes[i].maxSpeed, &bg_color);
                png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1] - 1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
            }
            //max speed
            plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].maxSpeed, text_red, text_green, text_blue);
        }
        line_index++; //one line completed,  moving up
        png.line(1, line_index * height_line + 1, total_width, line_index * height_line + 1, border_red, border_green, border_blue);//delimiter
        j = 0;
        line_offset = vertical_delim_align_offset;
    }
    //basic border
    png.line(1, 1, total_width, 1, border_red, border_green, border_blue);//bottom
    png.line(1, total_height, total_width, total_height, border_red, border_green, border_blue);//top
    png.line(1, 1, 1, total_height, border_red, border_green, border_blue);//left
    png.line(total_width, 1, total_width, total_height, border_red, border_green, border_blue);//right
    png.close(); //save picture
    //all done!
    return pngname;
}

#endif
