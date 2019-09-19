#ifndef INI_READER_H_INCLUDED
#define INI_READER_H_INCLUDED

#include <map>
#include <algorithm>

#include "misc.h"

class INIReader
{
private:
    bool parsed = false;
    string current_section;
    map<string, multimap<string, string>> ini_content;
    vector<string> exclude_sections, include_sections, read_sections;

    string cached_section;
    multimap<string, string> cached_section_content;
public:
    bool do_utf8_to_gbk = false;

    INIReader()
    {
        parsed = false;
    }

    INIReader(string filePath)
    {
        parsed = false;
        ParseFile(filePath);
    }

    ~INIReader()
    {
        //nothing to do
    }

    void ExcludeSection(string section)
    {
        exclude_sections.push_back(section);
    }

    void IncludeSection(string section)
    {
        include_sections.push_back(section);
    }

    bool chkIgnore(string section)
    {
        bool excluded = false, included = false;
        if(count(exclude_sections.begin(), exclude_sections.end(), section) > 0)
                excluded = true;
        if(include_sections.size() != 0)
        {
            if(count(include_sections.begin(), include_sections.end(), section) > 0)
                included = true;
        }
        else
            included = true;

        return excluded || !included;
    }

    int Parse(string content) //parse content into mapped data
    {
        bool inExcludedSection = false;
        string strTemp, thisSection, curSection, itemName, itemVal;
        string regSection = "^\\[(.*?)\\]$", regItem = "^(.*?)=(.*?)$";
        multimap<string, string> itemGroup;
        stringstream strStrm;
        if(do_utf8_to_gbk)
            content = UTF8ToGBK(content);

        strStrm<<content;
        while(getline(strStrm, strTemp)) //get one line of ini content
        {
            if(strTemp.find(";") == 0 || strTemp.find("#") == 0)
                continue;
            strTemp = replace_all_distinct(strTemp, "\r", "");
            if(regMatch(strTemp, regItem)) //is an item
            {
                if(inExcludedSection) //this section is excluded
                    continue;
                if(curSection == "") //not in any section
                    return -1;
                itemName = trim(regReplace(strTemp, regItem, "$1"));
                itemVal = trim(regReplace(strTemp, regItem, "$2"));
                itemGroup.insert(pair<string, string>(itemName, itemVal)); //insert to current section
            }
            if(regMatch(strTemp, regSection)) //is a section title
            {
                thisSection = regReplace(strTemp, regSection, "$1"); //save section title
                inExcludedSection = chkIgnore(thisSection); //check if this section is excluded

                if(curSection != "" && itemGroup.size() != 0) //just finished reading a section
                {
                    if(ini_content.count(curSection) != 0) //a section with the same name has been inserted
                        return -1;
                    ini_content.insert(pair<string, multimap<string, string>>(curSection, itemGroup)); //insert previous section to content map
                    read_sections.push_back(curSection); //add to read sections list
                }
                eraseElements(&itemGroup); //reset section storage
                curSection = thisSection; //start a new section
            }
            if(include_sections.size() != 0 && include_sections == read_sections) //all included sections has been read
                break; //exit now
        }
        if(curSection != "" && itemGroup.size() != 0) //final section
        {
            if(ini_content.count(curSection) != 0) //a section with the same name has been inserted
                    return -1;
            ini_content.insert(pair<string, multimap<string, string>>(curSection, itemGroup)); //insert this section to content map
            read_sections.push_back(curSection); //add to read sections list
        }
        parsed = true;
        return 0; //all done
    }

    int ParseFile(string filePath)
    {
        return Parse(getFileContent(filePath));
    }

    bool SectionExist(string section)
    {
        return ini_content.find(section) != ini_content.end();
    }

    unsigned int SectionCount()
    {
        return ini_content.size();
    }

    int EnterSection(string section)
    {
        if(!SectionExist(section))
            return -1;
        current_section = cached_section = section;
        cached_section_content = ini_content.at(section);
        return 0;
    }

    bool ItemExist(string section, string itemName)
    {
        if(!SectionExist(section))
            return false;

        if(section != cached_section)
        {
            cached_section = section;
            cached_section_content= ini_content.at(section);
        }

        return cached_section_content.find(itemName) != cached_section_content.end();
    }

    bool ItemExist(string itemName)
    {
        return current_section != "" ? ItemExist(current_section, itemName) : false;
    }

    bool ItemPrefixExist(string section, string itemName)
    {
        if(!SectionExist(section))
            return false;

        if(section != cached_section)
        {
            cached_section = section;
            cached_section_content= ini_content.at(section);
        }

        for(auto &x : cached_section_content)
        {
            if(x.first.find(itemName) == 0)
                return true;
        }

        return false;
    }

    bool ItemPrefixExist(string itemName)
    {
        return current_section != "" ? ItemPrefixExist(current_section, itemName) : false;
    }

    unsigned int ItemCount(string section)
    {
        if(!parsed || !SectionExist(section))
            return 0;

        return ini_content.at(section).size();
    }

    void EraseAll()
    {
        eraseElements(&ini_content);
    }

    int GetItems(string section, multimap<string, string> *data)
    {
        if(!parsed || !SectionExist(section))
            return -1;

        if(cached_section != section)
        {
            cached_section = section;
            cached_section_content = ini_content.at(section);
        }

        *data = cached_section_content;
        return 0;
    }

    int GetItems(multimap<string, string> *data)
    {
        return current_section != "" ? GetItems(current_section, data) : -1;
    }

    int GetAll(string section, string itemName, vector<string> *results) //retrieve item(s) with the same itemName prefix
    {
        if(!parsed)
            return -1;

        multimap<string, string> mapTemp;

        if(GetItems(section, &mapTemp) != 0)
            return -1;

        for(auto &x : mapTemp)
        {
            if(x.first.find(itemName) == 0)
                results->push_back(x.second);
        }

        return 0;
    }

    int GetAll(string itemName, vector<string> *results)
    {
        return current_section != "" ? GetAll(current_section, itemName, results) : -1;
    }

    string Get(string section, string itemName) //retrieve one item with the exact same itemName
    {
        if(!parsed)
            return string();

        multimap<string, string> mapTemp;

        if(GetItems(section, &mapTemp) != 0)
            return string();

        for(auto &x : mapTemp)
        {
            if(x.first == itemName)
                return x.second;
        }

        return string();
    }

    string Get(string itemName)
    {
        return current_section != "" ? Get(current_section, itemName) : string();
    }

    string GetFirst(string section, string itemName) //return the first item value found in section
    {
        if(!parsed)
            return string();
        vector<string> result;
        if(GetAll(section, itemName, &result) != -1)
            return result[0];
        else
            return string();
    }

    string GetFirst(string itemName)
    {
        return current_section != "" ? GetFirst(current_section, itemName) : string();
    }
};

#endif // INI_READER_H_INCLUDED
