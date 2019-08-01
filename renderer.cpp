#include "renderer.h"
#include "version.h"

using namespace std;
using namespace std::chrono;

string export_sort_method_render = "none";

struct color
{
    int red = 0;
    int green = 0;
    int blue = 0;
};

//original color
//vector<color> colorgroup = {{255 * 256, 255 * 256, 255 * 256}, {128 * 256, 255 * 256, 0}, {255 * 256, 255 * 256, 0}, {255 * 256, 128 * 256, 192 * 256}, {255 * 256, 0, 0}};
int colorgroup[][3] = {{255 * 256, 255 * 256, 255 * 256}, {128 * 256, 255 * 256, 0}, {255 * 256, 255 * 256, 0}, {255 * 256, 128 * 256, 192 * 256}, {255 * 256, 0, 0}};
int bounds[] = {0, 64 * 1024, 512 * 1024, 4 * 1024 * 1024, 16 * 1024 * 1024};
int color_count = 5;

/*
//rainbow color
int colorgroup[][3] = {{255 * 256, 255 * 256, 255 * 256}, {102 * 256, 255 * 256, 102 * 256}, {255 * 256, 255 * 256, 102 * 256}, {255 * 256, 178 * 256, 102 * 256}, {255 * 256, 102 * 256, 102 * 256}, {226 * 256, 140 * 256, 255 * 256}, {102 * 256, 204 * 256, 255 * 256}, {102 * 256, 102 * 256, 255 * 256}};
int bounds[] = {0, 64 * 1024, 512 * 1024, 4 * 1024 * 1024, 16 * 1024 * 1024, 24 * 1024 * 1024, 32 * 1024 * 1024, 40 * 1024 * 1024};
int color_count = 8;
*/

static inline void plot_text_utf8(pngwriter *png, string face_path, int fontsize, int x_start, int y_start, double angle, string text, double red, double green, double blue)
{
    png->plot_text_utf8(const_cast<char *>(face_path.data()), fontsize, x_start, y_start, angle, const_cast<char *>(text.data()), red, green, blue);
    return;
}

static inline int getTextWidth(string font, int fontsize, string text)
{
    return pngwriter().get_text_width_utf8(const_cast<char *>(font.data()), fontsize, const_cast<char *>(text.data()));
}

int getSpeed(string speed)
{
    if(speed == "")
        return 0;
    double speedval = 1.0;
    if(speed.find("MB") != string::npos)
        speedval = 1048576.0 * stof(speed.substr(0, speed.size() - 2));
    else if(speed.find("KB") != string::npos)
        speedval = 1024.0 * stof(speed.substr(0, speed.size() - 2));
    else if(speed.find("B") != string::npos)
        speedval = 1.0 * stof(speed.substr(0, speed.size() - 1));
    return (int)speedval;
}

/*
void useNewPalette()
{
    colorgroup = {{255, 255, 255}, {102, 255, 102}, {255, 255, 102}, {255, 178, 102}, {255, 102, 102}, {226, 140, 255}, {102, 204, 255}, {102, 102, 255}};
    bounds = {0, 64 * 1024, 512 * 1024, 4 * 1024 * 1024, 16 * 1024 * 1024, 24 * 1024 * 1024, 32 * 1024 * 1024, 40 * 1024 * 1024};
}
*/

bool comparer(nodeInfo &a, nodeInfo &b)
{
    if(export_sort_method_render == "speed")
        return getSpeed(a.avgspeed) < getSpeed(b.avgspeed);
    else if(export_sort_method_render == "rspeed")
        return getSpeed(a.avgspeed) > getSpeed(b.avgspeed);
    else if(export_sort_method_render == "ping")
        return stof(a.avgping) < stof(b.avgping);
    else if(export_sort_method_render == "rping")
        return stof(a.avgping) > stof(b.avgping);
    else
        return 0;
}

void getColor(color lc, color rc, float level, color *finalcolor)
{
    finalcolor->red = (int)((float)lc.red * (1.0 - level) + (float)rc.red * level);
    finalcolor->green = (int)((float)lc.green * (1.0 - level) + (float)rc.green * level);
    finalcolor->blue = (int)((float)lc.blue * (1.0 - level) + (float)rc.blue * level);
}

color array_to_color(int colors[3])
{
    color retcolor;
    retcolor.red = colors[0];
    retcolor.green = colors[1];
    retcolor.blue = colors[2];
    return retcolor;
}

void getSpeedColor(string speed, color *finalcolor)
{
    int speedval = getSpeed(speed);
    for(int i = 0; i < color_count - 1; i++)
    {
        if(speedval >= bounds[i] && speedval <= bounds[i + 1])
        {
            getColor(array_to_color(colorgroup[i]), array_to_color(colorgroup[i + 1]), ((float)speedval - (float)bounds[i]) / ((float)bounds[i + 1] - (float)bounds[i]), finalcolor);
            return;
        }
    }
    getColor(array_to_color(colorgroup[color_count - 1]), array_to_color(colorgroup[color_count - 1]), 1, finalcolor);
    return;
}

int calcLength(string data)
{
    int total = 0;
    for(unsigned int i = 0; i < data.size(); i++)
    {
        if(data[i] > 255 || data[i] < 0)
            total += 2;
        else
            total++;
    }
    return total;
}

int getTextLength(string str)
{
    return ((calcLength(str) - str.size()) / 3) * 2 + (str.size() * 2 - calcLength(str));
}

#ifndef _FAST_RENDER

string export_render(string resultpath, vector<nodeInfo> nodes, bool export_with_maxspeed, string export_sort_method)
{
    string pngname = replace_all_distinct(resultpath, ".log", ".png");
    nodeInfo node;
    int total_width = 0, total_height = 0, node_count = 0, total_line = 0;
    color bg_color;

    //predefined values
    const string font = "tools" PATH_SLASH "misc" PATH_SLASH "WenQuanYiMicroHei-01.ttf";
    const int height_line = 24, fontsize = 12, text_x_offset = 5, text_y_offset = 7, center_align_offset = 8, vertical_delim_align_offset = 2;
    const double border_red = 0.8, border_green = 0.8, border_blue = 0.8;
    const double text_red = 0.0, text_green = 0.0, text_blue = 0.0;
    //extra value for aligning to the center
    const int enableCenterAlign = 0;
    const int center_align_offset_side = center_align_offset/2;
    #define calcCenterOffset(item, total) ((((total - item) / 2) - center_align_offset_side) * enableCenterAlign)

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
    node.pkloss = "Pk.Loss";
    node.avgping = "TCP Ping";
    //node.siteping = "Bing Ping";
    node.avgspeed = "Avg.Speed";
    node.maxspeed = "Max.Speed";
    nodes.insert(nodes.begin(), node);

    //calculate the width of all columns
    int group_width = 0, remarks_width = 0, pkloss_width = 0, avgping_width = 0, avgspeed_width = 0, /* siteping_width = 0, */maxspeed_width = 0, onlines = 0, final_width = 0;
    int group_widths[node_count], remarks_widths[node_count], pkloss_widths[node_count], avgping_widths[node_count], avgspeed_widths[node_count], maxspeed_widths[node_count];
    long long total_traffic = 0;
    for(int i = 0; i <= node_count; i++)
    {
        //store them all into arrays first
        group_widths[i] = getTextWidth(font, fontsize, nodes[i].group);
        remarks_widths[i] = getTextWidth(font, fontsize, nodes[i].remarks);
        pkloss_widths[i] = getTextWidth(font, fontsize, nodes[i].pkloss);
        avgping_widths[i] = getTextWidth(font, fontsize, nodes[i].avgping);
        avgspeed_widths[i] = getTextWidth(font, fontsize, nodes[i].avgspeed);

        group_width = max(group_widths[i] + center_align_offset, group_width);
        remarks_width = max(remarks_widths[i] + center_align_offset, remarks_width);
        pkloss_width = max(pkloss_widths[i] + center_align_offset, pkloss_width);
        avgping_width = max(avgping_widths[i] + center_align_offset, avgping_width);
        //siteping_width = max(getTextWidth(font, fontsize, nodes[i].siteping) + center_align_offset, siteping_width);
        avgspeed_width = max(avgspeed_widths[i] + center_align_offset, avgspeed_width);
        if(export_with_maxspeed)
        {
            maxspeed_widths[i] = getTextWidth(font, fontsize, nodes[i].maxspeed);
            maxspeed_width = max(maxspeed_widths[i] + center_align_offset, maxspeed_width);
        }
        total_traffic += nodes[i].total_recv_bytes;
        if(nodes[i].online)
            onlines++;
    }
    //int width_all[8] =  {0, group_width, remarks_width, pkloss_width, avgping_width, siteping_width, avgspeed_width, maxspeed_width}; //put them into an array for reading
    int width_all[7] =  {0, group_width, remarks_width, pkloss_width, avgping_width, avgspeed_width, maxspeed_width}; //put them into an array for reading
    total_width = group_width + remarks_width + pkloss_width + avgping_width + /* siteping_width + */avgspeed_width;
    if(export_with_maxspeed)
        total_width += maxspeed_width;

    //generating information
    string gentime = "Generated at "+getTime(3);
    string traffic = "Traffic used : "+speedCalc((double)total_traffic)+". Working Node(s) : ["+to_string(onlines)+"/"+to_string(node_count)+"]";
    string about = "By Stair Speedtest Reborn " VERSION ".";

    final_width = max(getTextWidth(font, fontsize, gentime) + center_align_offset, total_width);
    final_width = max(getTextWidth(font, fontsize, traffic) + center_align_offset, total_width);
    if(final_width > total_width)
        width_all[2] += final_width - total_width;
    total_width = final_width;

    //initialize the file
    pngwriter png(total_width, total_height, 1.0, pngname.data());
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
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(group_widths[i], group_width), this_y_offset, 0.0, nodes[i].group, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //remarks
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(remarks_widths[i], remarks_width), this_y_offset, 0.0, nodes[i].remarks, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //packet loss
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(pkloss_widths[i], pkloss_width), this_y_offset, 0.0, nodes[i].pkloss, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //average ping
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(avgping_widths[i], avgping_width), this_y_offset, 0.0, nodes[i].avgping, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        /*
        //site ping, unused
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].siteping, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        */
        //draw color background
        if(i>0)
        {
            getSpeedColor(nodes[i].avgspeed, &bg_color);
            png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1]-1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
        }
        //average speed
        plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(avgspeed_widths[i], avgspeed_width), this_y_offset, 0.0, nodes[i].avgspeed, text_red, text_green, text_blue);
        if(export_with_maxspeed) //see if we want to draw max speed
        {
            j++;
            line_offset += width_all[j];
            png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
            this_x_offset += width_all[j];
            //draw color background
            if(i>0)
            {
                getSpeedColor(nodes[i].maxspeed, &bg_color);
                png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1] - 1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
            }
            //max speed
            plot_text_utf8(&png, font, fontsize, this_x_offset + calcCenterOffset(maxspeed_widths[i], maxspeed_width), this_y_offset, 0.0, nodes[i].maxspeed, text_red, text_green, text_blue);
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

#else

string export_render(string resultpath, vector<nodeInfo> nodes, bool export_with_maxspeed, string export_sort_method)
{
    string pngname = replace_all_distinct(resultpath, ".log", ".png");
    nodeInfo node;
    int total_width = 0, total_height = 0, node_count = 0, total_line = 0;
    color bg_color;

    //predefined values
    const string font = "tools" PATH_SLASH "misc" PATH_SLASH "WenQuanYiMicroHei-01.ttf";
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
    node.pkloss = "Pk.Loss";
    node.avgping = "TCP Ping";
    //node.siteping = "Bing Ping";
    node.avgspeed = "Avg.Speed";
    node.maxspeed = "Max.Speed";
    nodes.insert(nodes.begin(), node);

    //calculate the width of all columns
    int group_width = 0, remarks_width = 0, pkloss_width = 0, avgping_width = 0, avgspeed_width = 0, /* siteping_width = 0, */maxspeed_width = 0, onlines = 0, final_width = 0;
    string longest_group, longest_remarks, longest_pkloss, longest_avgping, longest_avgspeed, /*longest_siteping,*/longest_maxspeed;
    long long total_traffic = 0;
    for(int i = 0; i <= node_count; i++)
    {
        //find the longest string
        if(getTextLength(nodes[i].group) > getTextLength(longest_group))
            longest_group = nodes[i].group;
        if(getTextLength(nodes[i].remarks) > getTextLength(longest_remarks))
            longest_remarks = nodes[i].remarks;
        if(getTextLength(nodes[i].pkloss) > getTextLength(longest_pkloss))
            longest_pkloss = nodes[i].pkloss;
        if(getTextLength(nodes[i].avgping) > getTextLength(longest_avgping))
            longest_avgping = nodes[i].avgping;
        if(getTextLength(nodes[i].avgspeed) > getTextLength(longest_avgspeed))
            longest_avgspeed = nodes[i].avgspeed;
        if(export_with_maxspeed)
        {
            if(getTextLength(nodes[i].maxspeed) > getTextLength(longest_maxspeed))
                longest_maxspeed = nodes[i].maxspeed;
        }
        total_traffic += nodes[i].total_recv_bytes;
        if(nodes[i].online)
            onlines++;
    }
    //store them all into arrays first
    group_width = getTextWidth(font, fontsize, longest_group) + center_align_offset;
    remarks_width = getTextWidth(font, fontsize, longest_remarks) + center_align_offset;
    pkloss_width = getTextWidth(font, fontsize, longest_pkloss) + center_align_offset;
    avgping_width = getTextWidth(font, fontsize, longest_avgping) + center_align_offset;
    avgspeed_width = getTextWidth(font, fontsize, longest_avgspeed) + center_align_offset;
    if(export_with_maxspeed)
        maxspeed_width = getTextWidth(font, fontsize, longest_maxspeed) + center_align_offset;

    //int width_all[8] =  {0, group_width, remarks_width, pkloss_width, avgping_width, siteping_width, avgspeed_width, maxspeed_width}; //put them into an array for reading
    int width_all[7] =  {0, group_width, remarks_width, pkloss_width, avgping_width, avgspeed_width, maxspeed_width}; //put them into an array for reading
    total_width = group_width + remarks_width + pkloss_width + avgping_width + /* siteping_width + */avgspeed_width;
    if(export_with_maxspeed)
        total_width += maxspeed_width;

    //generating information
    string gentime = "Generated at "+getTime(3);
    string traffic = "Traffic used : "+speedCalc((double)total_traffic)+". Working Node(s) : ["+to_string(onlines)+"/"+to_string(node_count)+"]";
    string about = "By Stair Speedtest Reborn " VERSION ".";

    final_width = max(getTextWidth(font, fontsize, gentime) + center_align_offset, total_width);
    final_width = max(getTextWidth(font, fontsize, traffic) + center_align_offset, total_width);
    if(final_width > total_width)
        width_all[2] += final_width - total_width;
    total_width = final_width;

    //initialize the file
    pngwriter png(total_width, total_height, 1.0, pngname.data());
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
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].pkloss, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        //average ping
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].avgping, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        /*
        //site ping, unused
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].siteping, text_red, text_green, text_blue);
        j++;
        line_offset += width_all[j];
        png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
        this_x_offset += width_all[j];
        */
        //draw color background
        if(i>0)
        {
            getSpeedColor(nodes[i].avgspeed, &bg_color);
            png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1]-1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
        }
        //average speed
        plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].avgspeed, text_red, text_green, text_blue);
        if(export_with_maxspeed) //see if we want to draw max speed
        {
            j++;
            line_offset += width_all[j];
            png.line(line_offset, line_index * height_line + 1, line_offset, (line_index + 1) * height_line, border_red, border_green, border_blue);//right side
            this_x_offset += width_all[j];
            //draw color background
            if(i>0)
            {
                getSpeedColor(nodes[i].maxspeed, &bg_color);
                png.filledsquare(line_offset + 1, line_index * height_line + 2, line_offset + width_all[j + 1] - 1, (line_index + 1) * height_line, bg_color.red, bg_color.green, bg_color.blue);
            }
            //max speed
            plot_text_utf8(&png, font, fontsize, this_x_offset, this_y_offset, 0.0, nodes[i].maxspeed, text_red, text_green, text_blue);
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
