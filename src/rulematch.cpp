#include <string>
#include <future>
#include <thread>

#include "rulematch.h"
#include "geoip.h"
#include "misc.h"
#include "logger.h"
#include "nodeinfo.h"

void getTestFile(nodeInfo &node, const std::string &proxy, const std::vector<downloadLink> &downloadFiles, const std::vector<linkMatchRule> &matchRules, const std::string &defaultTestFile)
{
    writeLog(LOG_TYPE_RULES, "Rule match started.");
    std::string def_test_file = defaultTestFile;
    geoIPInfo outbound = node.outboundGeoIP.get();

    //scan the URLs first to find the default one
    writeLog(LOG_TYPE_RULES, "Searching default rule.");
    auto iter = std::find_if(downloadFiles.begin(), downloadFiles.end(), [](auto x){ return x.tag == "Default"; });
    if(iter != downloadFiles.end())
        def_test_file = iter->url;
    writeLog(LOG_TYPE_RULES, "Using default rule: '" + def_test_file + "'.");

    //only need to match outbound address
    auto iterRule = matchRules.begin();
    while(iterRule != matchRules.end())
    {
        switch(hash_(iterRule->mode))
        {
            case "match_isp"_hash:
            {
                auto iterRuleDetail = iterRule->rules.begin();
                while(iterRuleDetail != iterRule->rules.end())
                {
                    if(outbound.organization == *iterRuleDetail)
                    {
                        auto iterFile = downloadFiles.begin();
                        while(iterFile != downloadFiles.end())
                        {
                            if(iterFile->tag == iterRule->tag)
                            {
                                node.testFile = iterFile->url;
                                writeLog(LOG_TYPE_RULES, "Node  " + node.group + " - " + node.remarks + "  matches ISP rule '" + iterRule->tag + "'.");
                                break;
                            }
                            iterFile++;
                        }
                    }
                    iterRuleDetail++;
                }
                break;
            }
            case "match_country"_hash:
            {
                auto iterRuleDetail = iterRule->rules.begin();
                while(iterRuleDetail != iterRule->rules.end())
                {
                    if(outbound.country == *iterRuleDetail || outbound.country_code == *iterRuleDetail)
                    {
                        auto iterFile = downloadFiles.begin();
                        while(iterFile != downloadFiles.end())
                        {
                            if(iterFile->tag == iterRule->tag)
                            {
                                node.testFile = iterFile->url;
                                writeLog(LOG_TYPE_RULES, "Node  " + node.group + " - " + node.remarks + "  matches country rule '" + iterRule->tag + "'.");
                                break;
                            }
                            iterFile++;
                        }
                    }
                    iterRuleDetail++;
                }
                break;
            }
            case "match_group"_hash:
            {
                auto iterRuleDetail = iterRule->rules.begin();
                while(iterRuleDetail != iterRule->rules.end())
                {
                    if(node.group == *iterRuleDetail)
                    {
                        auto iterFile = downloadFiles.begin();
                        while(iterFile != downloadFiles.end())
                        {
                            if(iterFile->tag == iterRule->tag)
                            {
                                node.testFile = iterFile->url;
                                writeLog(LOG_TYPE_RULES, "Node  " + node.group + " - " + node.remarks + "  matches group rule '" + iterRule->tag + "'.");
                                break;
                            }
                            iterFile++;
                        }
                    }
                    iterRuleDetail++;
                }
                break;
            }
        }
        iterRule++;
    }
    if(node.testFile.empty()) //no match rule
    {
        writeLog(LOG_TYPE_RULES, "Node  " + node.group + " - " + node.remarks + "  matches no rule. Using default rule.");
        node.testFile = def_test_file;
    }
    writeLog(LOG_TYPE_RULES, "Node  " + node.group + " - " + node.remarks + "  uses test file '" + node.testFile +"'.");
}
