#include "rapidjson_extra.h"

void operator >> (const rapidjson::Value& value, std::string& i)
{
    if(value.IsNull())
        i = std::string();
    else if(value.IsInt64())
        i = std::to_string(value.GetInt64());
    else if(value.IsDouble())
        i = std::to_string(value.GetDouble());
    else if(value.IsString())
        i = std::string(value.GetString());
    else
        i = std::string();
}

void operator >> (const rapidjson::Value& value, int& i)
{
    if(value.IsNull())
        i = 0;
    else if(value.IsInt())
        i = value.GetInt();
    else if(value.IsString())
        i = std::stoi(value.GetString());
    else
        i = 0;
}

std::string GetMember(const rapidjson::Value& value, std::string member)
{
    std::string retStr;
    if(value.HasMember(member.data()))
        value[member.data()] >> retStr;
    return retStr;
}

void GetMember(const rapidjson::Value& value, std::string member, std::string* target)
{
    std::string retStr = GetMember(value, member);
    if(retStr.size())
        target->assign(retStr);
}
