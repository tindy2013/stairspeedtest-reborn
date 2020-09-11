#include <iostream>
#include <string>
#include <random>

#include "socket.h"
#include "misc.h"
#include "logger.h"

//Stun message types
unsigned char BIND_REQUEST_MSG[] = {0x0, 0x1};
unsigned char BIND_RESPONSE_MSG[] = {0x1, 0x1};
unsigned char MAGIC_COOKIE[] = {0x21, 0x12, 0xa4, 0x42};

//Stun attributes
unsigned char MAPPED_ADDRESS[] = {0x0, 0x1};
unsigned char RESPONSE_ADDRESS[] = {0x0, 0x2};
unsigned char CHANGE_REQUEST_IPPORT[] = {0x0, 0x3, 0x0, 0x4, 0x0, 0x0, 0x0, 0x6};
unsigned char CHANGE_REQUEST_PORT[] = {0x0, 0x3, 0x0, 0x4, 0x0, 0x0, 0x0, 0x2};
unsigned char SOURCE_ADDRESS[] = {0x0, 0x4};
unsigned char CHANGED_ADDRESS[] = {0x0, 0x5};
unsigned char XOR_MAPPED_ADDRESS[] = {0x80, 0x20};

enum NAT_TYPE
{
    UNKNOWN,
    UDP_BLOCKED,
    FULL_CONE_NAT,
    RESTRICTED_CONE_NAT,
    PORT_RESTRICTED_CONE_NAT,
    SYMMETRIC_NAT
};

const char *NAT_TYPE_STR[] = {"Unknown", "Blocked", "FullCone", "RestrictedCone", "PortRestrictedCone", "Symmetric"};

struct STUN_RESPONSE
{
    bool failed = true;
    uint16_t self_port = 0;
    std::string src_ip;
    uint16_t src_port = 0;
    std::string ext_ip;
    uint16_t ext_port = 0;
    std::string change_ip;
    uint16_t change_port = 0;
    std::string xor_ip;
    uint16_t xor_port = 0;
};

std::string long_to_str(unsigned long num, size_t len)
{
    std::string result;
    while(len--)
        result += ((char)((num >> (len * 8)) & 0xff));
    return result;
}

int str_to_int(const std::string &str)
{
    if(str.size() != 2)
        return 0;
    return ((int)(str[0]) << 8) + (int)str[1];
}

//self_port, udp_port
std::tuple<uint16_t, uint16_t> socks5_init_udp(SOCKET s, SOCKET udp_s, const std::string &server, uint16_t server_port, const std::string &username = "", const std::string &password = "")
{
    sockaddr_in srcaddr = {};
    socklen_t len;

    setTimeout(s, 1000);
    setTimeout(udp_s, 600);
    if(startConnect(s, server, server_port) == SOCKET_ERROR || connectSocks5(s, username, password) == -1)
        return std::make_tuple(0, 0);
    len = sizeof(srcaddr);
    if(getsockname(s, reinterpret_cast<sockaddr*>(&srcaddr), &len) < 0)
    {
        //cerr<<"error on tcp sockeet getsockname"<<endl;
        return std::make_tuple(0, 0);
    }
    std::string self_ip = sockaddrToIPAddr(reinterpret_cast<sockaddr*>(&srcaddr));

    uint16_t src_port = 0;
    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srcaddr.sin_port = htons(src_port);

    if(bind(udp_s, reinterpret_cast<sockaddr*>(&srcaddr), sizeof(srcaddr)) < 0)
    {
        //cerr<<"error on bind"<<endl;
        return std::make_tuple(0, 0);
    }

    memset(&srcaddr, 0, sizeof(srcaddr));
    len = sizeof(srcaddr);
    if(getsockname(udp_s, reinterpret_cast<sockaddr*>(&srcaddr), &len) < 0)
    {
        //cerr<<"error on udp socket getsockname"<<endl;
        return std::make_tuple(0, 0);
    }
    src_port = srcaddr.sin_port;
    uint16_t self_port = src_port;

    uint16_t udp_port = socks5_start_udp(s, self_ip, src_port);
    return std::make_tuple(self_port, udp_port);
}

std::string random_string(string_size length)
{
    const std::string CHARACTERS = "0123456789ABCDEF";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    while(length--)
        random_string += CHARACTERS[distribution(generator)];

    return random_string;
}

std::string make_transaction_id()
{
    //classic stun
    //return random_string(16);
    std::string id;
    id.assign(reinterpret_cast<const char *>(MAGIC_COOKIE), 4);
    id += random_string(12);
    return id;
}

void parse_xor_address(std::string &address, const std::string &trans_id)
{
    if((address.size() != 8 && address.size() != 20) || trans_id.size() != 16)
        return;
    size_t family = address[1], addr_size = 0;
    /// xor port with magic cookie byte 3 and 4
    address[2] ^= trans_id[0];
    address[3] ^= trans_id[1];
    switch(family)
    {
    case 1: //IPv4
        /// xor IPv4 with magic cookie
        addr_size = 4;
        break;
    case 2: //IPv6
        /// xor IPv6 with magic cookie & 12 random bytes
        addr_size = 16;
        break;
    }
    uint8_t *pAddr = reinterpret_cast<uint8_t*>(address.data()) + 4;
    for(size_t i = 0; i < addr_size; i++)
        pAddr[i] ^= trans_id[i];
}

STUN_RESPONSE get_stun_response_thru_socks5(SOCKET udp_s, const std::string &server, uint16_t udp_port, const std::string &target_server, uint16_t target_port, const std::string &send_data = "")
{
    STUN_RESPONSE response;
    int len, sent_len;

    char msg_head[] = {0, 1};
    std::string message, trans_id_str = make_transaction_id(), send_data_len = long_to_str(send_data.size(), 2);
    message.assign(msg_head, 2);
    message += send_data_len;
    message += trans_id_str;
    message += send_data;

    int fail_count = 0, max_fails = 5;
    char buf[BUF_SIZE] = {};
    bool passed = false;

    std::string msg_type, recv_trans_id, attrs;
    while(fail_count < max_fails)
    {
        if((sent_len = socks5_send_udp_data(udp_s, server, udp_port, target_server, target_port, message)) < 0)
        {
            writeLog(LOG_TYPE_STUN, "Error on sendto.");
            fail_count++;
            continue;
        }

        if((len = socks5_get_udp_data(udp_s, buf, BUF_SIZE - 1)) < 0)
        {
            writeLog(LOG_TYPE_STUN, "Error on recvfrom.");
            fail_count++;
            continue;
        }
        msg_type.assign(buf, buf + 2);
        recv_trans_id.assign(buf + 4, buf + 20);
        attrs.assign(buf + 20, buf + len);
        if(memcmp(msg_type.data(), BIND_RESPONSE_MSG, 2) != 0)
        {
            //cerr<<"return false response"<<endl;
            writeLog(LOG_TYPE_STUN, "STUN returned false response. Returned: " + std::to_string((int)msg_type[0]) + " " + std::to_string((int)msg_type[1]));
            fail_count++;
            continue;
        }
        if(memcmp(recv_trans_id.data(), trans_id_str.data(), 16) != 0)
        {
            //cerr<<"return false trans_id"<<endl;
            writeLog(LOG_TYPE_STUN, "STUN returned false trans_id. Probably the response from the last test. Try again...");
            fail_count++;
            continue;
        }
        passed = true;
        break;
    }
    if(!passed)
        return response;

    string_size pos = 0, attr_length = 0;
    std::string attr_type, attr_value;
    while(pos < attrs.size())
    {
        attr_type = attrs.substr(pos, 2);
        attr_length = str_to_int(attrs.substr(pos + 2, 2));
        attr_value = attrs.substr(pos + 4, attr_length);
        pos += attr_length + 4;
        if(attr_length % 4)
            attr_length += 4 - (attr_length % 4);
        if(memcmp(attr_type.data(), MAPPED_ADDRESS, 2) == 0)
        {
            std::string ip;
            uint16_t port;
            std::tie(ip, port) = getSocksAddress(attr_value);
            response.ext_ip = ip;
            response.ext_port = port;
            response.failed = false;
        }
        else if(memcmp(attr_type.data(), SOURCE_ADDRESS, 2) == 0)
        {
            std::string ip;
            uint16_t port;
            std::tie(ip, port) = getSocksAddress(attr_value);
            response.src_ip = ip;
            response.src_port = port;
            response.failed = false;
        }
        else if(memcmp(attr_type.data(), CHANGED_ADDRESS, 2) == 0)
        {
            std::string ip;
            uint16_t port;
            std::tie(ip, port) = getSocksAddress(attr_value);
            response.change_ip = ip;
            response.change_port = port;
            response.failed = false;
        }
        else if(memcmp(attr_type.data(), XOR_MAPPED_ADDRESS, 2) == 0)
        {
            std::string ip;
            uint16_t port;
            parse_xor_address(attr_value, trans_id_str);
            std::tie(ip, port) = getSocksAddress(attr_value);
            response.xor_ip = ip;
            response.xor_port = port;
            response.failed = false;
        }
    }
    return response;
}

std::string get_nat_type_thru_socks5(const std::string &server, uint16_t port, const std::string &username, const std::string &password, const std::string &stun_server, uint16_t stun_port)
{
    writeLog(LOG_TYPE_STUN, "STUN on SOCKS5 server " + server + ":" + std::to_string(port) + " started. Using STUN server " + stun_server + ":" + std::to_string(stun_port) + ".");
    SOCKET s = initSocket(AF_INET, SOCK_STREAM, 0), udp_s = initSocket(AF_INET, SOCK_DGRAM, 0);
    defer(closesocket(s); closesocket(udp_s);)
    STUN_RESPONSE response;
    uint16_t self_port, udp_port;
    std::tie(self_port, udp_port) = socks5_init_udp(s, udp_s, server, port, username, password);
    if(udp_port == 0)
    {
        writeLog(LOG_TYPE_STUN, "Failed to start UDP Association with SOCKS5 server. Leaving...");
        return NAT_TYPE_STR[UNKNOWN];
    }
    writeLog(LOG_TYPE_STUN, "Trying STUN Test 1.");
    response = get_stun_response_thru_socks5(udp_s, server, udp_port, stun_server, stun_port);
    //check open internet or udp blocked, skip for now
    if(response.failed)
    {
        //std::cout<<"blocked"<<std::endl;
        writeLog(LOG_TYPE_STUN, "STUN Test 1 failed to get response. NAT type: UDP Blocked.");
        return NAT_TYPE_STR[UDP_BLOCKED];
    }
    writeLog(LOG_TYPE_STUN, "Public end: " + response.ext_ip + ":" + std::to_string(response.ext_port));
    std::string change_ip = response.change_ip, ext_ip = response.ext_ip, send_data;
    uint16_t change_port = response.change_port, ext_port = response.ext_port;
    send_data.assign((char*)CHANGE_REQUEST_IPPORT, 8);
    writeLog(LOG_TYPE_STUN, "STUN Test 1 passed. Trying STUN Test 2.");
    response = get_stun_response_thru_socks5(udp_s, server, udp_port, stun_server, stun_port, send_data);
    if(!response.failed)
    {
        //std::cout<<"full cone"<<std::endl;
        writeLog(LOG_TYPE_STUN, "STUN Test 2 passed. NAT type: Full Cone NAT.");
        return NAT_TYPE_STR[FULL_CONE_NAT];
    }
    writeLog(LOG_TYPE_STUN, "STUN Test 2 failed to get response. Trying STUN Test 1 with CHANGED_IP.");
    //other type of nat
    /*
    closesocket(s);
    closesocket(udp_s);
    s = initSocket(AF_INET, SOCK_STREAM, 0), udp_s = initSocket(AF_INET, SOCK_DGRAM, 0);
    std::tie(self_port, udp_port) = socks5_init_udp(s, udp_s, server, port);
    if(udp_port == 0)
    {
        writeLog(LOG_TYPE_STUN, "Failed to start UDP Association with SOCKS5 server. Leaving...");
        return NAT_TYPE_STR[UNKNOWN];
    }
    */
    response = get_stun_response_thru_socks5(udp_s, server, udp_port, change_ip, change_port);
    if(response.failed)
    {
        writeLog(LOG_TYPE_STUN, "STUN Test 1 with CHANGED_IP failed to get response. Something is wrong. Leaving...");
        return NAT_TYPE_STR[UNKNOWN];
    }
    writeLog(LOG_TYPE_STUN, "Public end: " + response.ext_ip + ":" + std::to_string(response.ext_port));
    if(response.ext_ip != ext_ip || response.ext_port != ext_port)
    {
        //std::cout<<"symmetric"<<std::endl;
        writeLog(LOG_TYPE_STUN, "STUN Test 1 with CHANGED_IP returned different SRC_IP/SRC_PORT. NAT type: Symmetric NAT.");
        return NAT_TYPE_STR[SYMMETRIC_NAT];
    }
    writeLog(LOG_TYPE_STUN, "STUN Test 1 with CHANGED_IP passed. Trying STUN Test 3 with CHANGED_IP.");
    send_data.assign((char*)CHANGE_REQUEST_PORT, 8);
    response = get_stun_response_thru_socks5(udp_s, server, udp_port, change_ip, change_port, send_data);
    if(response.failed)
    {
        //std::cout<<"port restricted"<<std::endl;
        writeLog(LOG_TYPE_STUN, "STUN Test 3 with CHANGED_IP failed to get response. NAT type: Port Restricted Cone NAT.");
        return NAT_TYPE_STR[PORT_RESTRICTED_CONE_NAT];
    }
    //std::cout<<"stun: restricted cone"<<std::endl;
    writeLog(LOG_TYPE_STUN, "STUN Test 3 with CHANGED_IP passed. NAT type: Restricted Cone NAT.");
    return NAT_TYPE_STR[RESTRICTED_CONE_NAT];
}
