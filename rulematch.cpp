#include <string>

#include "rulematch.h"
#include "geoip.h"
#include "misc.h"

using namespace std;

void getTestFile(nodeInfo *node, socks5Proxy proxy, vector<downloadLink> *downloadFiles, vector<linkMatchRule> *matchRules, string defaultTestFile)
{
    string def_test_file = defaultTestFile;
    vector<downloadLink>::iterator iterFile = downloadFiles->begin();
    vector<linkMatchRule>::iterator iterRule = matchRules->begin();
    vector<string>::iterator iterRuleDetail;

    //first retrieve all GeoIP info, ignore the inbound one for now
    //node->inboundGeoIP = getGeoIPInfo(node->server, "");
    node->outboundGeoIP = getGeoIPInfo("", proxy);
    //if(node->outboundGeoIP.organization == "") //something went wrong, try again
        //node->outboundGeoIP = getGeoIPInfo("", proxy);

    //scan the URLs first to find the default one
    while(iterFile != downloadFiles->end())
    {
        if(iterFile->tag == "Default")
        {
            def_test_file = iterFile->url;
        }
        iterFile++;
    }

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
        node->testFile = def_test_file;
    }
}
