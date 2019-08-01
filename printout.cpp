#include "printout.h"

//define print-out messages
struct LOOKUP_ITEM
{
    int index;
    string info;
};

LOOKUP_ITEM SPEEDTEST_MESSAGES[] = {
    {SPEEDTEST_MESSAGE_EOF, "\nSpeedtest done. Press any key to exit..."},
    {SPEEDTEST_MESSAGE_WELCOME, "Welcome to Stair Speedtest!\nWhich stair do you want to test today? (Supports single ss/ssd/ssr/v2ray link and their subscribe links)\nLink: "},
    {SPEEDTEST_MESSAGE_FOUNDVMESS, "Found single v2ray link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDSS, "Found single ss link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDSSR, "Found single ssr link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDSUB, "Found subscribe link.\n"},
    {SPEEDTEST_MESSAGE_FOUNDLOCAL, "Found local configure file.\n"},
    {SPEEDTEST_MESSAGE_GROUP, "If you have imported an v2ray subscribe link which doesn't contain a Group Name, you can write a custom name below.\nIf you have imported an ss/ssr link which contains a Group Name, press Enter to skip.\nCustom Group Name: "},
    {SPEEDTEST_MESSAGE_GOTSERVER, "\nCurrent Server Group: ?group? Remarks: ?remarks?\n"},
    {SPEEDTEST_MESSAGE_STARTPING, "Now performing tcping...\n"},
    {SPEEDTEST_MESSAGE_STARTSPEED, "Now performing speedtest...\n"},
    {SPEEDTEST_MESSAGE_GOTRESULT, "Result: DL.Speed: ?speed? Max.Speed: ?maxspeed? Pk.Loss: ?pkloss? Avg.Ping: ?avgping?\n"},
    {SPEEDTEST_MESSAGE_TRAFFIC, "Traffic used: ?traffic?\n"},
    {SPEEDTEST_MESSAGE_PICSAVING, "Now exporting png...\n"},
    {SPEEDTEST_MESSAGE_PICSAVED, "Result png saved to \"?picpath?\".\n"},
    {SPEEDTEST_MESSAGE_FETCHSUB, "Downloading subscription data...\n"},
    {SPEEDTEST_MESSAGE_PARSING, "Parsing configuration file...\n"},
    {SPEEDTEST_MESSAGE_BEGIN, "Speedtest will now begin.\n"},
    {SPEEDTEST_ERROR_UNDEFINED, "Undefined error!\n"},
    {SPEEDTEST_ERROR_WSAERR, "WSA Startup error!\n"},
    {SPEEDTEST_ERROR_SOCKETERR, "Socket error!\n"},
    {SPEEDTEST_ERROR_NORECOGLINK, "No valid link found. Please check your link.\n"},
    {SPEEDTEST_ERROR_UNRECOGFILE, "This configure file is invalid. Please make sure this is an ss/ssr gui-config.json file.\n"},
    {SPEEDTEST_ERROR_NOCONNECTION, "Cannot connect to server.\n"},
    {SPEEDTEST_ERROR_INVALIDSUB, "Nothing returned from subscribe link. Please check your subscribe link.\n"},
    {SPEEDTEST_ERROR_NONODES, "No nodes found. Please check your subscribe link.\n"},
    {SPEEDTEST_ERROR_NORESOLVE, "Cannot resolve server address.\n"},
    {SPEEDTEST_ERROR_RETEST, "Speedtest returned no speed. Retesting...\n"},
    {SPEEDTEST_ERROR_NOSPEED, "Speedtest returned no speed 2 times. Skipping...\n"},
    {-1, ""}
};

LOOKUP_ITEM SPEEDTEST_MESSAGES_RPC[] = {
    {SPEEDTEST_MESSAGE_WELCOME, "{\"info\":\"started\"}\n"},
    {SPEEDTEST_MESSAGE_EOF, "{\"info\":\"eof\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDVMESS, "{\"info\":\"foundvmess\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDSS, "{\"info\":\"foundss\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDSSR, "{\"info\":\"foundssr\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDSUB, "{\"info\":\"foundsub\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDLOCAL, "{\"info\":\"foundlocal\"}\n"},
    {SPEEDTEST_MESSAGE_FOUNDUPD, "{\"info\":\"foundupd\"}\n"},
    {SPEEDTEST_MESSAGE_GOTSERVER, "{\"info\":\"gotserver\",\"id\":?id?,\"group\":\"?group?\",\"remarks\":\"?remarks?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTPING, "{\"info\":\"startping\",\"id\":?id?}\n"},
    {SPEEDTEST_MESSAGE_GOTPING, "{\"info\":\"gotping\",\"id\":?id?,\"ping\":\"?avgping?\",\"loss\":\"?pkloss?\"}\n"},
    {SPEEDTEST_MESSAGE_STARTSPEED, "{\"info\":\"startspeed\",\"id\":?id?}\n"},
    {SPEEDTEST_MESSAGE_GOTSPEED, "{\"info\":\"gotspeed\",\"id\":?id?,\"speed\":\"?speed?\",\"maxspeed\":\"?maxspeed?\"}\n"},
    {SPEEDTEST_MESSAGE_TRAFFIC, "(\"info\":\"traffic\",\"size\":\"?traffic?\"}\n"},
    {SPEEDTEST_MESSAGE_PICSAVING, "{\"info\":\"picsaving\"}\n"},
    {SPEEDTEST_MESSAGE_PICSAVED, "{\"info\":\"picsaved\",\"path\":\"?picpath?\"}\n"},
    {SPEEDTEST_MESSAGE_FETCHSUB, "{\"info\":\"fetchingsub\"}\n"},
    {SPEEDTEST_MESSAGE_PARSING, "{\"info\":\"parsing\"}\n"},
    {SPEEDTEST_MESSAGE_BEGIN, "{\"info\":\"begintest\"}\n"},
    {SPEEDTEST_MESSAGE_PICDATA, "{\"info\":\"picdata\",\"data\":\"?data?\"}\n"},
    {SPEEDTEST_ERROR_UNDEFINED, "{\"info\":\"error\",\"reason\":\"undef\"}\n"},
    {SPEEDTEST_ERROR_WSAERR, "{\"info\":\"error\",\"reason\":\"wsaerr\"}\n"},
    {SPEEDTEST_ERROR_SOCKETERR, "{\"info\":\"error\",\"reason\":\"socketerr\"}\n"},
    {SPEEDTEST_ERROR_NORECOGLINK, "{\"info\":\"error\",\"reason\":\"norecoglink\"}\n"},
    {SPEEDTEST_ERROR_UNRECOGFILE, "{\"info\":\"error\",\"reason\":\"unrecogfile\"}\n"},
    {SPEEDTEST_ERROR_NOCONNECTION, "{\"info\":\"error\",\"reason\":\"noconnection\",\"id\":?id?}\n"},
    {SPEEDTEST_ERROR_INVALIDSUB, "{\"info\":\"error\",\"reason\":\"invalidsub\"}\n"},
    {SPEEDTEST_ERROR_NONODES, "{\"info\":\"error\",\"reason\":\"nonodes\"}\n"},
    {SPEEDTEST_ERROR_NORESOLVE, "{\"info\":\"error\",\"reason\":\"noresolve\",\"id\":?id?}\n"},
    {SPEEDTEST_ERROR_RETEST, "{\"info\":\"error\",\"reason\":\"retest\",\"id\":?id?}\n"},
    {SPEEDTEST_ERROR_NOSPEED, "{\"info\":\"error\",\"reason\":\"nospeed\",\"id\":?id?}\n"},
    {-1, ""}
};

string lookup(int index, LOOKUP_ITEM *items)
{
    int i = 0;
    while (0 <= items[i].index) {
        if (items[i].index == index)
            return items[i].info;
        i++;
    }
    return string("");
}

void printmsg(int index,nodeInfo *node,bool rpcmode)
{
    string printout;
    if(rpcmode)
    {
        printout = lookup(index, SPEEDTEST_MESSAGES_RPC);
    } else {
        printout = lookup(index, SPEEDTEST_MESSAGES);
    }
    if(printout.size() == 0)
    {
        return;
    }
    printout = replace_all_distinct(printout, "?group?", trim(node->group));
    printout = replace_all_distinct(printout, "?remarks?", trim(node->remarks));
    printout = replace_all_distinct(printout, "?id?", to_string(node->id));
    printout = replace_all_distinct(printout, "?avgping?", node->avgping);
    printout = replace_all_distinct(printout, "?pkloss?", node->pkloss);
    printout = replace_all_distinct(printout, "?speed?", node->avgspeed);
    printout = replace_all_distinct(printout, "?maxspeed?", node->maxspeed);
    printout = replace_all_distinct(printout, "?traffic?", node->traffic);
    if(rpcmode)
        printout = replace_all_distinct(printout, "\\", "\\\\");
    cout<<printout;
    cout.clear();
    cout.flush();
}

void printmsg_with_dict(int index, bool rpcmode, vector<string> dict, vector<string> trans)
{
    string printout;
    //map<string, string> dict;
    if(rpcmode)
    {
        printout = lookup(index, SPEEDTEST_MESSAGES_RPC);
    } else {
        printout = lookup(index, SPEEDTEST_MESSAGES);
    }
    if(printout.size() == 0)
    {
        return;
    }
    for(unsigned int i = 0; i < dict.size(); i++)
    {
        printout = replace_all_distinct(printout, dict[i], trans[i]);
    }
    if(rpcmode)
        printout = replace_all_distinct(printout, "\\", "\\\\");
    cout<<printout;
    cout.clear();
    cout.flush();
}

int write2file(string path,string content,bool overwrite)
{
    fstream outfile;
    ios::openmode mode = ios::out;
    if(!overwrite)
        mode = ios::app;
    outfile.open(path, mode);
    outfile<<content<<endl;
    outfile.close();
    return 0;
}
