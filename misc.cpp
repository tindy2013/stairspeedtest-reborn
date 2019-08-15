#include "misc.h"

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
    this_thread::sleep_for(chrono::milliseconds(interval));
}

string GBKToUTF8(string str_src)
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
    string strTemp = str;
    if(wstr)
        delete[] wstr;
    if(str)
        delete[] str;
    return strTemp;
#else
    return str_src;
#endif // _WIN32
}

string UTF8ToGBK(string str_src)
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
    string strTemp(szGBK);
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
// string to wstring
void StringToWstring(wstring& szDst, string str)
{
	string temp = str;
	int len = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, NULL,0);
	wchar_t* wszUtf8 = new wchar_t[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)temp.c_str(), -1, (LPWSTR)wszUtf8, len);
	szDst = wszUtf8;
	wstring r = wszUtf8;
	delete[] wszUtf8;
}
#endif // _WIN32

string regReplace(string src, string match, string rep)
{
    string result = "";
    regex reg(match);
    regex_replace(back_inserter(result), src.begin(), src.end(), reg, rep);
    return result;
}

int regMatch(string src, string match)
{
    regex reg(match);
    return regex_match(src, reg);
}

string speedCalc(double speed)
{
    if(speed == 0.0)
        return string("0.00B");
    char str[10];
    string retstr;
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

unsigned char FromHex(unsigned char x)
{
    unsigned char y;
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

string UrlDecode(const string& str)
{
    string strTemp = "";
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
            strTemp += high*16 + low;
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

string base64_encode(string string_to_encode)
{
    char const* bytes_to_encode = string_to_encode.data();
    unsigned int in_len = string_to_encode.size();

    string ret;
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

string base64_decode(string encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    string ret;

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

vector<string> split(const string &s, const string &seperator)
{
    vector<string> result;
    typedef string::size_type string_size;
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

string getUrlArg(string url, string request)
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
        return string();
    }
}

string replace_all_distinct(string str, string old_value, string new_value)
{
    for(string::size_type pos(0); pos != string::npos; pos += new_value.length())
    {
        if((pos = str.find(old_value, pos)) != string::npos)
            str.replace(pos, old_value.length(), new_value);
        else
            break;
    }
    return str;
}

string urlsafe_base64_reverse(string encoded_string)
{
    return replace_all_distinct(replace_all_distinct(encoded_string, "-", "+"), "_", "/");
}

string urlsafe_base64_decode(string encoded_string)
{
    return base64_decode(urlsafe_base64_reverse(encoded_string));
}

bool strFind(string str,string target)
{
    return str.find(target) != str.npos;
}

string grabContent(string raw)
{
    /*
    string strTmp="";
    vector<string> content;
    content=split(raw,"\r\n\r\n");
    for(unsigned int i=1;i<content.size();i++) strTmp+=content[i];
    */
    return regReplace(raw.substr(raw.find("\r\n\r\n") + 4), "^\\d*?\\r\\n(.*)\\r\\n\\d", "$1");
    //return raw;
}

string trim(const string& str)
{
    string::size_type pos = str.find_first_not_of(' ');
    if (pos == string::npos)
    {
        return str;
    }
    string::size_type pos2 = str.find_last_not_of(' ');
    if (pos2 != string::npos)
    {
        return str.substr(pos, pos2 - pos + 1);
    }
    return str.substr(pos);
}

bool fileExist(string path)
{
    return _access(path.data(), 4) != -1;
}

bool fileCopy(string source, string dest)
{
    ifstream infile;
    ofstream outfile;
    infile.open(source, ios::binary);
    if(!infile)
        return false;
    outfile.open(dest, ios::binary);
    if(!outfile)
        return false;
    try
    {
        outfile<<infile.rdbuf();
    }
    catch (exception &e)
    {
        return false;
    }
    infile.close();
    outfile.close();
    return true;
}

string fileToBase64(string filepath)
{
    ifstream infile;
    string strdata;
    stringstream strstrm;
    infile.open(filepath, ios::binary);
    if(!infile)
        return string();
    strstrm<<infile.rdbuf();
    strdata = strstrm.str();
    infile.close();
    return base64_encode(strdata);
}
