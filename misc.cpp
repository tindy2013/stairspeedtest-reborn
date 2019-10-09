#include <chrono>
#include <regex>
#include <fstream>
#include <thread>
#include <sstream>
#include <iosfwd>
#include <unistd.h>

#include <rapidjson/document.h>
#include <openssl/md5.h>

#include "misc.h"

#ifdef _WIN32
//#include <io.h>
#include <windows.h>
#include <winreg.h>
#else
#ifndef __hpux
#include <sys/select.h>
#endif /* __hpux */
#ifndef _access
#define _access access
#endif // _access
#include <sys/socket.h>
#endif // _WIN32

using namespace std::__cxx11;

void sleep(int interval)
{
/*
#ifdef _WIN32
    Sleep(interval);
#else
    // Portable sleep for platforms other than Windows.
    struct timeval wait = { 0, interval * 1000 };
    select(0, NULL, NULL, NULL, &wait);
#endif
*/
    //upgrade to c++11 standard
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

std::string GBKToUTF8(std::string str_src)
{
#ifdef _WIN32
    const char* strGBK = str_src.c_str();
    int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    std::string strTemp = str;
    if(wstr)
        delete[] wstr;
    if(str)
        delete[] str;
    return strTemp;
#else
    return str_src;
#endif // _WIN32
}

std::string UTF8ToGBK(std::string str_src)
{
#ifdef _WIN32
    const char* strUTF8 = str_src.data();
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    std::string strTemp(szGBK);
    if (wszGBK)
        delete[] wszGBK;
    if (szGBK)
        delete[] szGBK;
    return strTemp;
#else
    return str_src;
#endif
}

#ifdef _WIN32
// std::string to wstring
void StringToWstring(std::wstring& szDst, std::string str)
{
	std::string temp = str;
	int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, NULL,0);
	wchar_t* wszUtf8 = new wchar_t[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, (LPWSTR)wszUtf8, len);
	szDst = wszUtf8;
	std::wstring r = wszUtf8;
	delete[] wszUtf8;
}
#endif // _WIN32

unsigned char FromHex(unsigned char x)
{
    unsigned char y = '\0';
    if (x >= 'A' && x <= 'Z')
        y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        y = x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        y = x - '0';
    else
        assert(0);
    return y;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+')
            strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else
            strTemp += str[i];
    }
    return strTemp;
}

static inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(std::string string_to_encode)
{
    char const* bytes_to_encode = string_to_encode.data();
    unsigned int in_len = string_to_encode.size();

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--)
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string base64_decode(std::string encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i ==4)
        {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            ret += char_array_3[j];
    }

    return ret;
}

std::vector<std::string> split(const std::string &s, const std::string &seperator)
{
    std::vector<std::string> result;
    typedef std::string::size_type string_size;
    string_size i = 0;

    while(i != s.size())
    {
        int flag = 0;
        while(i != s.size() && flag == 0)
        {
            flag = 1;
            for(string_size x = 0; x < seperator.size(); ++x)
                if(s[i] == seperator[x])
                {
                    ++i;
                    flag = 0;
                    break;
                }
        }

        flag = 0;
        string_size j = i;
        while(j != s.size() && flag == 0)
        {
            for(string_size x = 0; x < seperator.size(); ++x)
                if(s[j] == seperator[x])
                {
                    flag = 1;
                    break;
                }
            if(flag == 0)
                ++j;
        }
        if(i != j)
        {
            result.push_back(s.substr(i, j-i));
            i = j;
        }
    }
    return result;
}

std::string getSystemProxy()
{
#ifdef _WIN32
    HKEY key;
    auto ret = RegOpenKeyEx(HKEY_CURRENT_USER, R"(Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings)", 0, KEY_ALL_ACCESS, &key);
    if(ret != ERROR_SUCCESS){
        //std::cout << "open failed: " << ret << std::endl;
        return std::string();
    }

    DWORD values_count, max_value_name_len, max_value_len;
    ret = RegQueryInfoKey(key, NULL, NULL, NULL, NULL, NULL, NULL,
        &values_count, &max_value_name_len, &max_value_len, NULL, NULL);
    if(ret != ERROR_SUCCESS){
        //std::cout << "query failed" << std::endl;
        return std::string();
    }

    std::vector<std::tuple<std::shared_ptr<char>, DWORD, std::shared_ptr<BYTE>>> values;
    for(DWORD i = 0; i < values_count; i++){
		std::shared_ptr<char> value_name(new char[max_value_name_len + 1],
			std::default_delete<char[]>());
        DWORD value_name_len = max_value_name_len + 1;
        DWORD value_type, value_len;
        RegEnumValue(key, i, value_name.get(), &value_name_len, NULL, &value_type, NULL, &value_len);
        std::shared_ptr<BYTE> value(new BYTE[value_len],
			std::default_delete<BYTE[]>());
        value_name_len = max_value_name_len + 1;
        RegEnumValue(key, i, value_name.get(), &value_name_len, NULL, &value_type, value.get(), &value_len);
        values.push_back(std::make_tuple(value_name, value_type, value));
    }

	DWORD ProxyEnable = 0;
	for (auto x : values) {
		if (strcmp(std::get<0>(x).get(), "ProxyEnable") == 0) {
			ProxyEnable = *(DWORD*)(std::get<2>(x).get());
		}
	}

	if (ProxyEnable) {
		for (auto x : values) {
			if (strcmp(std::get<0>(x).get(), "ProxyServer") == 0) {
				//std::cout << "ProxyServer: " << (char*)(std::get<2>(x).get()) << std::endl;
				return std::string((char*)(std::get<2>(x).get()));
			}
		}
	}
	/*
	else {
		//std::cout << "Proxy not Enabled" << std::endl;
	}
	*/
	//return 0;
	return std::string();
#else
    char* proxy = getenv("ALL_PROXY");
    if(proxy != NULL)
        return std::string(proxy);
    else
        return std::string();
#endif // _WIN32
}

std::string trim(const std::string& str)
{
    std::string::size_type pos = str.find_first_not_of(' ');
    if (pos == std::string::npos)
    {
        return str;
    }
    std::string::size_type pos2 = str.find_last_not_of(' ');
    if (pos2 != std::string::npos)
    {
        return str.substr(pos, pos2 - pos + 1);
    }
    return str.substr(pos);
}

std::string getUrlArg(std::string url, std::string request)
{
    smatch result;
    if (regex_search(url.cbegin(), url.cend(), result, regex(request + "=(.*?)&")))
    {
        return result[1];
    }
    else if (regex_search(url.cbegin(), url.cend(), result, regex(request + "=(.*)")))
    {
        return result[1];
    }
    else
    {
        return std::string();
    }
}

std::string replace_all_distinct(std::string str, std::string old_value, std::string new_value)
{
    for(std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())
    {
        if((pos = str.find(old_value, pos)) != std::string::npos)
            str.replace(pos, old_value.length(), new_value);
        else
            break;
    }
    return str;
}

bool regFind(std::string src, std::string target)
{
    regex reg(target);
    return regex_search(src, reg);
}

std::string regReplace(std::string src, std::string match, std::string rep)
{
    std::string result = "";
    regex reg(match);
    regex_replace(back_inserter(result), src.begin(), src.end(), reg, rep);
    return result;
}

bool regMatch(std::string src, std::string match)
{
    regex reg(match);
    return regex_match(src, reg);
}

std::string speedCalc(double speed)
{
    if(speed == 0.0)
        return std::string("0.00B");
    char str[10];
    std::string retstr;
    if(speed >= 1073741824.0)
        sprintf(str, "%.2fGB", speed / 1073741824.0);
    else if(speed >= 1048576.0)
        sprintf(str, "%.2fMB", speed / 1048576.0);
    else if(speed >= 1024.0)
        sprintf(str, "%.2fKB", speed / 1024.0);
    else
        sprintf(str, "%.2fB", speed);
    retstr = str;
    return retstr;
}

std::string urlsafe_base64_reverse(std::string encoded_string)
{
    return replace_all_distinct(replace_all_distinct(encoded_string, "-", "+"), "_", "/");
}

std::string urlsafe_base64_decode(std::string encoded_string)
{
    return base64_decode(urlsafe_base64_reverse(encoded_string));
}

std::string grabContent(std::string raw)
{
    /*
    std::string strTmp="";
    vector<std::string> content;
    content=split(raw,"\r\n\r\n");
    for(unsigned int i=1;i<content.size();i++) strTmp+=content[i];
    */
    return regReplace(raw.substr(raw.find("\r\n\r\n") + 4), "^\\d*?\\r\\n(.*)\\r\\n\\d", "$1");
    //return raw;
}

std::string getMD5(std::string data)
{
    MD5_CTX ctx;
    std::string result;
	unsigned int i = 0;
	unsigned char digest[16] = {};

	MD5_Init(&ctx);
    MD5_Update(&ctx, data.data(), data.size());
    MD5_Final((unsigned char *)&digest, &ctx);

    char tmp[3] = {};
    for(i = 0; i < 16; i++)
	{
		snprintf(tmp, 3, "%02x", digest[i]);
		result += tmp;
    }

    return result;
}

std::string fileGet(std::string path)
{
    std::ifstream infile;
    std::stringstream strstrm;

    infile.open(path, std::ios::binary);
    if(infile)
    {
        strstrm<<infile.rdbuf();
        infile.close();
        return strstrm.str();
    }
    return std::string();
}

bool fileExist(std::string path)
{
    return _access(path.data(), 4) != -1;
}

bool fileCopy(std::string source, std::string dest)
{
    std::ifstream infile;
    std::ofstream outfile;
    infile.open(source, std::ios::binary);
    if(!infile)
        return false;
    outfile.open(dest, std::ios::binary);
    if(!outfile)
        return false;
    try
    {
        outfile<<infile.rdbuf();
    }
    catch (std::exception &e)
    {
        return false;
    }
    infile.close();
    outfile.close();
    return true;
}

std::string fileToBase64(std::string filepath)
{
    return base64_encode(fileGet(filepath));
}

std::string fileGetMD5(std::string filepath)
{
    return getMD5(fileGet(filepath));
}

int fileWrite(std::string path, std::string content, bool overwrite)
{
    std::fstream outfile;
    std::ios::openmode mode = overwrite ? std::ios::out : std::ios::app;
    outfile.open(path, mode);
    outfile << content << std::endl;
    outfile.close();
    return 0;
}

bool isIPv4(std::string address)
{
    return regMatch(address, "^(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}$");
}

bool isIPv6(std::string address)
{
    int ret;
    std::vector<std::string> regLists = {"^(?:[0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$", "^((?:[0-9A-Fa-f]{1,4}(:[0-9A-Fa-f]{1,4})*)?)::((?:([0-9A-Fa-f]{1,4}:)*[0-9A-Fa-f]{1,4})?)$", "^(::(?:[0-9A-Fa-f]{1,4})(?::[0-9A-Fa-f]{1,4}){5})|((?:[0-9A-Fa-f]{1,4})(?::[0-9A-Fa-f]{1,4}){5}::)$"};
    for(unsigned int i = 0; i < regLists.size(); i++)
    {
        ret = regMatch(address, regLists[i]);
        if(ret)
            return true;
    }
    return false;
}

std::string rand_str(const int len)
{
    std::string retData;
    srand(time(NULL));
    int cnt = 0;
    while(cnt < len)
    {
        switch((rand() % 3))
        {
        case 1:
            retData += ('A' + rand() % 26);
            break;
        case 2:
            retData += ('a' + rand() % 26);
            break;
        default:
            retData += ('0' + rand() % 10);
            break;
        }
        cnt++;
    }
    return retData;
}

void urlParse(std::string url, std::string &host, std::string &path, int &port, bool &isTLS)
{
    std::vector<std::string> args;

    if(regMatch(url, "^https://(.*)"))
        isTLS = true;
    url = regReplace(url, "^(http|https)://", "");
    if(url.find("/") == url.npos)
    {
        host = url;
        path = "/";
    }
    else
    {
        host = url.substr(0, url.find("/"));
        path = url.substr(url.find("/"));
    }
    if(regFind(host, "\\[(.*)\\]")) //IPv6
    {
        args = split(regReplace(host, "\\[(.*)\\](.*)", "$1,$2"), ",");
        if(args.size() == 2) //with port
            port = stoi(args[1].substr(1));
        host = args[0];
    }
    else if(strFind(host, ":"))
    {
        port = stoi(host.substr(host.rfind(":") + 1));
        host = host.substr(0, host.rfind(":"));
    }
    if(port == 0)
    {
        if(isTLS)
            port = 443;
        else
            port = 80;
    }
}
