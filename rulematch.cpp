#include <string>

#include "rulematch.h"
#include "geoip.h"
#include "misc.h"
#include "logger.h"

using namespace std;

void getTestFile(nodeInfo *node, socks5Proxy proxy, vector<downloadLink> *downloadFiles, vector<linkMatchRule> *matchRules, string defaultTestFile)
{
    writeLog(LOG_TYPE_RULES, "Rule match started.");
    string def_test_file = defaultTestFile;
    vector<downloadLink>::iterator iterFile = downloadFiles->begin();
    vector<linkMatchRule>::iterator iterRule = matchRules->begin();
    vector<string>::iterator iterRuleDetail;

    //first retrieve all GeoIP info, ignore the inbound one for now
    //node->inboundGeoIP = getGeoIPInfo(node->server, "");
    writeLog(LOG_TYPE_RULES, "Fetching inbound GeoIP info.");
    node->outboundGeoIP = getGeoIPInfo("", proxy);
    //if(node->outboundGeoIP.organization == "") //something went wrong, try again
        //node->outboundGeoIP = getGeoIPInfo("", proxy);

    //scan the URLs first to find the default one
    writeLog(LOG_TYPE_RULES, "Searching default rule.");
    while(iterFile != downloadFiles->end())
    {
        if(iterFile->tag == "Default")
        {
            def_test_file = iterFile->url;
        }
        iterFile++;
    }
    writeLog(LOG_TYPE_RULES, "Using default rule: '" + def_test_file + "'.");

    //only need to match outbound address
    while(iterRule != matchRules->end())
    {
        if(iterRule->mode == "match_isp")
        {
            if(node->outboundGeoIP.organization == iterRule->rules[0])
            {
                iterFile = downloadFiles->begin();
                while(iterFile != downloadFiles->end())
                {
                    if(iterFile->tag == iterRule->tag)
                    {
                        node->testFile = iterFile->url;
                        writeLog(LOG_TYPE_RULES, "Node  " + node->group + " - " + node->remarks + "  matches ISP rule '" + iterRule->tag + "'.");
                        break;
                    }
                    iterFile++;
                }
            }
        }
        else if(iterRule->mode == "match_country")
        {
            iterRuleDetail = iterRule->rules.begin();
            while(iterRuleDetail != iterRule->rules.end())
            {
                if(node->outboundGeoIP.country == *iterRuleDetail || node->outboundGeoIP.country_code == *iterRuleDetail)
                {
                    iterFile = downloadFiles->begin();
                    while(iterFile != downloadFiles->end())
                    {
                        if(iterFile->tag == iterRule->tag)
                        {
                            node->testFile = iterFile->url;
                            writeLog(LOG_TYPE_RULES, "Node  " + node->group + " - " + node->remarks + "  matches country rule '" + iterRule->tag + "'.");
                            break;
                        }
                        iterFile++;
                    }
                }
                iterRuleDetail++;
            }
        }
        iterRule++;
    }
    if(node->testFile == "") //no match rule
    {
        writeLog(LOG_TYPE_RULES, "Node  " + node->group + " - " + node->remarks + "  matches no rule. Using default rule.");
        node->testFile = def_test_file;
    }
    writeLog(LOG_TYPE_RULES, "Node  " + node->group + " - " + node->remarks + "  uses test file '" + node->testFile +"'.");
}
