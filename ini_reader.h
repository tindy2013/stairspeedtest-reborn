#ifndef INI_READER_H_INCLUDED
#define INI_READER_H_INCLUDED

#include <map>
#include <algorithm>

#include "misc.h"

#define MAX_LINE_LENGTH 512

    /**
    *  @brief A simple INI reader which utilize map and vector
    *  to store sections and items, allowing access in logarithmic time.
    */
class INIReader
{
private:
    bool parsed = false;
    string current_section;
    map<string, multimap<string, string>> ini_content;
    vector<string> exclude_sections, include_sections, read_sections;

    string cached_section;
    multimap<string, string> cached_section_content;

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
public:
    /**
    *  @brief Set this flag to true to do a UTF8-To-GBK conversion before parsing data. Only useful in Windows.
    */
    bool do_utf8_to_gbk = false;

    /**
    *  @brief Set this flag to true so any line within the section will be stored even it doesn't follow the "name=value" format.
    * These line will store as the name "{NONAME}".
    */
    bool store_any_line = false;

    /**
    *  @brief Initialize the reader.
    */
    INIReader()
    {
        parsed = false;
    }

    /**
    *  @brief Parse a file during initialization.
    */
    INIReader(string filePath)
    {
        parsed = false;
        ParseFile(filePath);
    }

    ~INIReader()
    {
        //nothing to do
    }

    /**
    *  @brief Exclude a section with the given name.
    */
    void ExcludeSection(string section)
    {
        exclude_sections.push_back(section);
    }

    /**
    *  @brief Include a section with the given name.
    */
    void IncludeSection(string section)
    {
        include_sections.push_back(section);
    }

    /**
    *  @brief Parse INI content into mapped data structure.
    * If exclude sections are set, these sections will not be stored.
    * If include sections are set, only these sections will be stored.
    */
    int Parse(string content) //parse content into mapped data
    {
        bool inExcludedSection = false;
        string strLine, thisSection, curSection, itemName, itemVal;
        string regSection = "^\\[(.*?)\\]$", regItem = "^(.*?)=(.*?)$";
        multimap<string, string> itemGroup;
        stringstream strStrm;
        char delimiter = count(content.begin(), content.end(), '\n') <= 1 ? '\r' : '\n';

        EraseAll(); //first erase all data
        if(do_utf8_to_gbk)
            content = UTF8ToGBK(content); //do conversion if flag is set

        strStrm<<content;
        while(getline(strStrm, strLine, delimiter)) //get one line of content
        {
            strLine = replace_all_distinct(strLine, "\r", ""); //remove line break
            if(!strLine.size() || strLine.size() > MAX_LINE_LENGTH || strLine.find(";") == 0 || strLine.find("#") == 0) //empty lines, lines longer than MAX_LINE_LENGTH and comments are ignored
                continue;
            if(regMatch(strLine, regItem)) //is an item
            {
                if(inExcludedSection) //this section is excluded
                    continue;
                if(!curSection.size()) //not in any section
                    return -1;
                itemName = trim(regReplace(strLine, regItem, "$1"));
                itemVal = trim(regReplace(strLine, regItem, "$2"));
                itemGroup.insert(pair<string, string>(itemName, itemVal)); //insert to current section
            }
            else if(regMatch(strLine, regSection)) //is a section title
            {
                thisSection = regReplace(strLine, regSection, "$1"); //save section title
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
            else if(store_any_line && !inExcludedSection && curSection.size()) //store a line without name
            {
                itemGroup.insert(pair<string, string>("{NONAME}", strLine));
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

    /**
    *  @brief Parse an INI file into mapped data structure.
    */
    int ParseFile(string filePath)
    {
        return Parse(getFileContent(filePath));
    }

    /**
    *  @brief Check whether a section exist.
    */
    bool SectionExist(string section)
    {
        return ini_content.find(section) != ini_content.end();
    }

    /**
    *  @brief Count of sections in the whole INI.
    */
    unsigned int SectionCount()
    {
        return ini_content.size();
    }

    /**
    *  @brief Enter a section with the given name. Section name and data will be cached to speed up the following reading process.
    */
    int EnterSection(string section)
    {
        if(!SectionExist(section))
            return -1;
        current_section = cached_section = section;
        cached_section_content = ini_content.at(section);
        return 0;
    }

    /**
    *  @brief Check whether an item exist in the given section. Return false if the section does not exist.
    */
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

    /**
    *  @brief Check whether an item exist in current section. Return false if the section does not exist.
    */
    bool ItemExist(string itemName)
    {
        return current_section != "" ? ItemExist(current_section, itemName) : false;
    }

    /**
    *  @brief Check whether an item with the given name prefix exist in the given section. Return false if the section does not exist.
    */
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

    /**
    *  @brief Check whether an item with the given name prefix exist in current section. Return false if the section does not exist.
    */
    bool ItemPrefixExist(string itemName)
    {
        return current_section != "" ? ItemPrefixExist(current_section, itemName) : false;
    }

    /**
    *  @brief Count of items in the given section. Return 0 if the section does not exist.
    */
    unsigned int ItemCount(string section)
    {
        if(!parsed || !SectionExist(section))
            return 0;

        return ini_content.at(section).size();
    }

    /**
    *  @brief Erase all data from the data structure and reset parser status.
    */
    void EraseAll()
    {
        eraseElements(&ini_content);
        parsed = false;
    }

    /**
    *  @brief Retrieve all items in the given section.
    */
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

    /**
    *  @brief Retrieve all items in current section.
    */
    int GetItems(multimap<string, string> *data)
    {
        return current_section != "" ? GetItems(current_section, data) : -1;
    }

    /**
    * @brief Retrieve item(s) with the same name prefix in the given section.
    */
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

    /**
    * @brief Retrieve item(s) with the same name prefix in current section.
    */
    int GetAll(string itemName, vector<string> *results)
    {
        return current_section != "" ? GetAll(current_section, itemName, results) : -1;
    }

    /**
    * @brief Retrieve one item with the exact same name in the given section.
    */
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

    /**
    * @brief Retrieve one item with the exact same name in current section.
    */
    string Get(string itemName)
    {
        return current_section != "" ? Get(current_section, itemName) : string();
    }

    /**
    * @brief Retrieve the first item found in the given section.
    */
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

    /**
    * @brief Retrieve the first item found in current section.
    */
    string GetFirst(string itemName)
    {
        return current_section != "" ? GetFirst(current_section, itemName) : string();
    }
};

#endif // INI_READER_H_INCLUDED
