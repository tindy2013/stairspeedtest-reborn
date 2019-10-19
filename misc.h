#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "//"
#endif // _WIN32

#include "geoip.h"

typedef std::vector<std::string> string_array;

struct nodeInfo
{
    int linkType = -1;
    int id = 0;
    int groupID = 0;
    bool online = false;
    std::string group;
    std::string remarks;
    std::string server;
    int port = 0;
    std::string proxyStr;
    long long rawSpeed[20] = {};
    long long totalRecvBytes = 0;
    int duration = 0;
    std::string avgSpeed = "N/A";
    std::string maxSpeed = "N/A";
    std::string ulSpeed = "N/A";
    std::string pkLoss = "100.00%";
    int rawPing[6] = {};
    std::string avgPing = "0.00";
    int rawSitePing[10] = {};
    std::string sitePing = "0.00";
    std::string traffic;
    geoIPInfo inboundGeoIP;
    geoIPInfo outboundGeoIP;
    std::string testFile;
    std::string ulTarget;
};

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string UrlDecode(const std::string& str);
std::string base64_decode(std::string encoded_string);
std::string base64_encode(std::string string_to_encode);

std::vector<std::string> split(const std::string &s, const std::string &seperator);
std::string getUrlArg(std::string url, std::string request);
std::string replace_all_distinct(std::string str, std::string old_value, std::string new_value);
std::string urlsafe_base64_reverse(std::string encoded_string);
std::string urlsafe_base64_decode(std::string encoded_string);
std::string UTF8ToGBK(std::string str_src);
std::string GBKToUTF8(std::string str_src);
std::string trim(const std::string& str);
std::string getSystemProxy();
std::string rand_str(const int len);
bool is_str_utf8(std::string data);

void sleep(int interval);
bool regFind(std::string src, std::string target);
std::string regReplace(std::string src, std::string match, std::string rep);
bool regMatch(std::string src, std::string match);
std::string speedCalc(double speed);
std::string grabContent(std::string raw);
std::string getMD5(std::string data);
bool isIPv4(std::string address);
bool isIPv6(std::string address);
void urlParse(std::string url, std::string &host, std::string &path, int &port, bool &isTLS);

std::string fileGet(std::string path);
int fileWrite(std::string path, std::string content, bool overwrite);
bool fileExist(std::string path);
bool fileCopy(std::string source,std::string dest);
std::string fileToBase64(std::string filepath);
std::string fileGetMD5(std::string filepath);

static inline bool strFind(std::string str, std::string target)
{
    return str.find(target) != str.npos;
}

template <typename T> static inline void eraseElements(T &target)
{
    T().swap(target);
}

#ifdef _MACOS
namespace std
{
    namespace __cxx11
    {
        template <typename T> std::string to_string(const T& n)
        {
            std::ostringstream ss;
            ss << n ;
            return ss.str();
        }
    }
}
#endif // _MACOS

#ifdef _WIN32
void StringToWstring(std::wstring& szDst, std::string str);
#endif // _WIN32

#endif // MISC_H_INCLUDED
