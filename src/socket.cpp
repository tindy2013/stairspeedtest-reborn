#include <iostream>
#include <chrono>
#include <memory>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "socket.h"
#include "misc.h"

using namespace std::chrono;

int connect_timeout = 3000;

#ifdef __CYGWIN32__
#undef _WIN32
#endif

#ifdef _WIN32
#ifndef ECONNRESET
#define ECONNRESET WSAECONNRESET
#endif	/* not ECONNRESET */
#endif /* _WI32 */

/* informations for SOCKS */
#define SOCKS5_REP_SUCCEEDED    0x00    /* succeeded */
#define SOCKS5_REP_FAIL         0x01    /* general SOCKS serer failure */
#define SOCKS5_REP_NALLOWED     0x02    /* connection not allowed by ruleset */
#define SOCKS5_REP_NUNREACH     0x03    /* Network unreachable */
#define SOCKS5_REP_HUNREACH     0x04    /* Host unreachable */
#define SOCKS5_REP_REFUSED      0x05    /* connection refused */
#define SOCKS5_REP_EXPIRED      0x06    /* TTL expired */
#define SOCKS5_REP_CNOTSUP      0x07    /* Command not supported */
#define SOCKS5_REP_ANOTSUP      0x08    /* Address not supported */
#define SOCKS5_REP_INVADDR      0x09    /* Invalid address */

/* SOCKS5 authentication methods */
#define SOCKS5_AUTH_REJECT      0xFF    /* No acceptable auth method */
#define SOCKS5_AUTH_NOAUTH      0x00    /* without authentication */
#define SOCKS5_AUTH_GSSAPI      0x01    /* GSSAPI */
#define SOCKS5_AUTH_USERPASS    0x02    /* User/Password */
#define SOCKS5_AUTH_CHAP        0x03    /* Challenge-Handshake Auth Proto. */
#define SOCKS5_AUTH_EAP         0x05    /* Extensible Authentication Proto. */
#define SOCKS5_AUTH_MAF         0x08    /* Multi-Authentication Framework */

/* socket related definitions */
#ifndef _WIN32
#define SOCKET int
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif /* WIN32 */

/* packet operation macro */
#define PUT_BYTE(ptr, data) (*(unsigned char* )(ptr) = (unsigned char)(data))

SOCKET initSocket(int af, int type, int protocol)
{
    SOCKET s = socket(af, type, protocol);
    int one = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int));
#ifdef SO_NOSIGPIPE
    setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (char *)&one, sizeof(int));
#endif
    return s;
}

int Send(SOCKET sHost, const char* data, int len, int flags)
{
#ifdef _WIN32
    return send(sHost, data, len, flags);
#else
    return send(sHost, data, len, flags | MSG_NOSIGNAL);
#endif // _WIN32
}

int Recv(SOCKET sHost, char* data, int len, int flags)
{
#ifdef _WIN32
    return recv(sHost, data, len, flags);
#else
    return recv(sHost, data, len, flags | MSG_NOSIGNAL);
#endif // _WIN32
}

int getNetworkType(std::string addr)
{
    if(isIPv4(addr))
        return AF_INET;
    else if(isIPv6(addr))
        return AF_INET6;
    else
        return AF_UNSPEC;
}

int socks5_do_auth_userpass(SOCKET sHost, std::string user, std::string pass)
{
    char buf[1024], *ptr;
    //char* pass = NULL;
    int len;
    ptr = buf;
    PUT_BYTE(ptr++, 1);                       // sub-negotiation ver.: 1
    len = user.size();                          // ULEN and UNAME
    PUT_BYTE(ptr++, len);
    strcpy(ptr, user.data());
    ptr += len;
    len = pass.size();                          // PLEN and PASSWD
    PUT_BYTE(ptr++, len);
    strcpy(ptr, pass.data());
    ptr += len;
    Send(sHost, buf, ptr - buf, 0);
    Recv(sHost, buf, 2, 0);
    if (buf[1] == 0)
        return 0;                               /* success */
    else
        return -1;                              /* fail */
}

int setTimeout(SOCKET s, int timeout)
{
    int ret = -1;
#ifdef _WIN32
    ret = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(int));
    ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
#else
    struct timeval timeo = {timeout / 1000, (timeout % 1000) * 1000};
    ret = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeo, sizeof(timeo));
    ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeo, sizeof(timeo));
#endif
    return ret;
}

int setSocketBlocking(SOCKET s, bool blocking)
{
#ifdef _WIN32
    unsigned long ul = !blocking;
    return ioctlsocket(s, FIONBIO, &ul); //set to non-blocking mode
#else
    int flags = fcntl(s, F_GETFL, 0);
    if(flags == -1)
        return -1;
    return fcntl(s, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
#endif // _WIN32
}

int connect_adv(SOCKET sockfd, const struct sockaddr* addr, int addrsize)
{
    int ret = -1;
    int error = 1;
    struct timeval tm;
    fd_set set;

    int len = sizeof(int);
    if(setSocketBlocking(sockfd, false) == -1)
        return -1;
    if(connect(sockfd, addr, addrsize) == -1)
    {
        tm.tv_sec = connect_timeout / 1000;
        tm.tv_usec = (connect_timeout % 1000) * 1000;
        FD_ZERO(&set);
        FD_SET(sockfd, &set);
        if(select(sockfd + 1, NULL, &set, NULL, &tm) > 0)
        {
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t *)&len);
            if(error == 0)
                ret = 0;
            else
                ret = 1;
        }
        else
            ret = 1;
    }
    else
        ret = 0;

    if(setSocketBlocking(sockfd, true) == -1)
        return -1;
    return ret;
}

int startConnect(SOCKET sHost, std::string addr, int port)
{
    int retVal = -1;
    struct sockaddr_in servAddr = {};
    struct sockaddr_in6 servAddr6 = {};
    if(isIPv4(addr))
    {
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons((short)port);
        inet_pton(AF_INET, addr.data(), (struct in_addr *)&servAddr.sin_addr.s_addr);
        retVal = connect_adv(sHost, reinterpret_cast<sockaddr *>(&servAddr), sizeof(servAddr));
    }
    else if(isIPv6(addr))
    {
        servAddr6.sin6_family = AF_INET6;
        servAddr6.sin6_port = htons((short)port);
        inet_pton(AF_INET6, addr.data(), (struct in_addr6 *)&servAddr6.sin6_addr);
        retVal = connect_adv(sHost, reinterpret_cast<sockaddr *>(&servAddr6), sizeof(servAddr6));
    }
    return retVal;
}

int send_simple(SOCKET sHost, std::string data)
{
    return Send(sHost, data.data(), data.size(), 0);
}

int simpleSend(std::string addr, int port, std::string data)
{
    SOCKET sHost = socket(getNetworkType(addr), SOCK_STREAM, IPPROTO_IP);
    if(sHost == INVALID_SOCKET)
        return SOCKET_ERROR;
    if(startConnect(sHost, addr, port) != 0)
        return SOCKET_ERROR;
    setTimeout(sHost, 3000);
    unsigned int retVal = send_simple(sHost, data);
    if(retVal == data.size())
    {
        closesocket(sHost);
#ifdef _WIN32
        return WSAGetLastError();
#else
        return 0;
#endif // _WIN32
    }
    else
    {
        closesocket(sHost);
        return SOCKET_ERROR;
    }
}

std::string sockaddrToIPAddr(sockaddr *addr)
{
    std::string retAddr;
    char cAddr[128] = {};
    struct sockaddr_in *target;
    struct sockaddr_in6 *target6;
    if(addr->sa_family == AF_INET)
    {
        target = reinterpret_cast<struct sockaddr_in *>(addr);
        inet_ntop(AF_INET, &target->sin_addr, cAddr, sizeof(cAddr));
    }
    else if(addr->sa_family == AF_INET6)
    {
        target6 = reinterpret_cast<struct sockaddr_in6 *>(addr);
        inet_ntop(AF_INET6, &target6->sin6_addr, cAddr, sizeof(cAddr));
    }
    retAddr.assign(cAddr);
    return retAddr;
}

std::string hostnameToIPAddr(std::string host)
{
    //old function
    /*
        struct in_addr inaddr;
        hostent *h = gethostbyname(host.data());
        if(h == NULL)
            return std::string();
        inaddr.s_addr = *(u_long*)h->h_addr_list[0];
        return inet_ntoa(inaddr);
    */
    //new function
    int retVal;
    std::string retstr;
    struct addrinfo hint = {}, *retAddrInfo = NULL, *cur;
    defer(if(retAddrInfo) freeaddrinfo(retAddrInfo));
    retVal = getaddrinfo(host.data(), NULL, &hint, &retAddrInfo);
    if(retVal != 0)
        return std::string();

    for(cur = retAddrInfo; cur != NULL; cur = cur->ai_next)
    {
        retstr = sockaddrToIPAddr(cur->ai_addr);
        if(!retstr.empty())
            break;
    }
    return retstr;
}

int connectSocks5(SOCKET sHost, std::string username, std::string password)
{
    char buf[BUF_SIZE], bufRecv[BUF_SIZE];
    //ZeroMemory(buf, BUF_SIZE);
    char* ptr;
    ptr = buf;
    PUT_BYTE(ptr++, 5); //socks5
    PUT_BYTE(ptr++, 2); //2 auth methods
    PUT_BYTE(ptr++, 0); //no auth
    PUT_BYTE(ptr++, 2); //user pass auth
    Send(sHost, buf, ptr - buf, 0);
    ZeroMemory(bufRecv, BUF_SIZE);
    Recv(sHost, bufRecv, 2, 0);
    if ((bufRecv[0] != 5) ||                       // ver5 response
            ((unsigned char)buf[1] == 0xFF))  	// check auth method
    {
        std::cerr << "socks5: connect not accepted" << std::endl;
        return -1;
    }
    int auth_method = bufRecv[1];
    int auth_result = -1;
    switch (auth_method)
    {
    case SOCKS5_AUTH_REJECT:
        std::cerr << "socks5: no acceptable authentication method\n" << std::endl;
        return -1;                              // fail

    case SOCKS5_AUTH_NOAUTH:
        // nothing to do
        auth_result = 0;
        break;

    case SOCKS5_AUTH_USERPASS:
        // do user/pass auth
        auth_result = socks5_do_auth_userpass(sHost, username, password);
        break;

    default:
        return -1;
    }
    if ( auth_result != 0 )
    {
        std::cerr << "socks5: authentication failed." << std::endl;
        return -1;
    }
    return 0;
}

int putSocksAddress(char **p, const std::string &host, const uint16_t dest_port)
{
    /* destination target host and port */
    int len = 0;
    struct sockaddr_in dest_addr = {};
    struct sockaddr_in6 dest_addr6 = {};
    int type = 0;
    char *ptr = *p;
    if(isIPv4(host))
    {
        // IPv4 address provided
        inet_pton(AF_INET, host.data(), &dest_addr.sin_addr.s_addr);
        PUT_BYTE(ptr++, 1);                   // ATYP: IPv4
        len = sizeof(dest_addr.sin_addr);
        memcpy(ptr, &dest_addr.sin_addr.s_addr, len);
        type = 1;
    }
    else if(isIPv6(host))
    {
        // IPv6 address provided
        inet_pton(AF_INET6, host.data(), &dest_addr6.sin6_addr);
        PUT_BYTE(ptr++, 4);                   // ATYP: IPv6
        len = sizeof(dest_addr6.sin6_addr);
        memcpy(ptr, &dest_addr6.sin6_addr, len);
        type = 4;
    }
    else
    {
        // host name provided
        PUT_BYTE(ptr++, 3);                    // ATYP: DOMAINNAME
        len = host.size();
        PUT_BYTE(ptr++, len);                  // DST.ADDR (len)
        memcpy(ptr, host.data(), len);          //(hostname)
        type = 3;
    }
    ptr += len;

    PUT_BYTE(ptr++, dest_port >> 8);     // DST.PORT
    PUT_BYTE(ptr++, dest_port & 0xFF);
    *p = ptr;
    return type;
}

std::tuple<std::string, uint16_t> getSocksAddress(const std::string &data)
{
    char cAddr[128] = {};
    std::string retAddr, port_str = data.substr(2, 2);
    int family = data[1];
    uint16_t port = ntohs(*(short*)port_str.data());
    switch(family)
    {
    case 1: //IPv4
        inet_ntop(AF_INET, data.data() + 4, cAddr, 127);
        break;
    case 2: //IPv6
        inet_ntop(AF_INET6, data.data() + 4, cAddr, 127);
        break;
    }
    retAddr.assign(cAddr);
    return std::make_tuple(retAddr, port);
}

int connectThruSocks(SOCKET sHost, std::string host, int port)
{
    char buf[BUF_SIZE];//bufRecv[BUF_SIZE];
    ZeroMemory(buf, BUF_SIZE);
    char* ptr;

    ptr = buf;
    PUT_BYTE(ptr++, 5);                        // SOCKS version (5)
    PUT_BYTE(ptr++, 1);                        // CMD: CONNECT
    PUT_BYTE(ptr++, 0);                        // FLG: 0

    putSocksAddress(&ptr, host, port);

    Send(sHost, buf, ptr - buf, 0);
    Recv(sHost, buf, 4, 0);
    if(buf[0] != 5 || buf[1] != SOCKS5_REP_SUCCEEDED)     // check reply code
    {
        std::cerr << "socks5: got error response from SOCKS server" << std::endl;
        return -1;
    }
    ptr = buf + 4;
    switch(buf[3])                           // case by ATYP
    {
    case 1:                                     // IP v4 ADDR
        Recv(sHost, ptr, 4 + 2, 0);
        break;
    case 3:                                     // DOMAINNAME
        Recv(sHost, ptr, 1, 0);
        Recv(sHost, ptr + 1, *(unsigned char*)ptr + 2, 0);
        break;
    case 4:                                     // IP v6 ADDR
        Recv(sHost, ptr, 16 + 2, 0);
        break;
    }
    return 0;
}

int connectThruHTTP(SOCKET sHost, std::string username, std::string password, std::string dsthost, int dstport)
{
    char bufRecv[BUF_SIZE] = {};
    std::string request = "CONNECT " + dsthost + ":" + std::to_string(dstport) + " HTTP/1.1\r\n";
    std::string authstr = "Authorization: Basic " + base64_encode(username + ":" + password) + "\r\n";
    if(username.size() && password.size())
        request += authstr;
    request += "\r\n";

    Send(sHost, request.data(), request.size(), 0);
    Recv(sHost, bufRecv, BUF_SIZE - 1, 0);
    if(!strFind(std::string(bufRecv), "HTTP/1.1 200"))
        return -1;

    return 0;
}

int checkPort(int startport)
{
    SOCKET fd = 0;
    int retVal;
    sockaddr_in servAddr;
    ZeroMemory(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    while(startport < 65536)
    {
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(fd == INVALID_SOCKET)
            continue;
        servAddr.sin_port = htons(startport);
        retVal = ::bind(fd, (struct sockaddr*)(&servAddr), sizeof(sockaddr_in));
        if(retVal == SOCKET_ERROR)
        {
            closesocket(fd);
            startport++;
        }
        else
        {
            closesocket(fd);
            return startport;
        }
    }
    closesocket(fd);
    return -1;
}

uint16_t socks5_start_udp(SOCKET sHost, const std::string &host, const uint16_t port)
{
    char buf[BUF_SIZE], *ptr = buf;

    PUT_BYTE(ptr++, 5);                        // SOCKS version (5)
    PUT_BYTE(ptr++, 3);                        // CMD: UDP
    PUT_BYTE(ptr++, 0);                        // FLG: 0

    putSocksAddress(&ptr, host, port);

    Send(sHost, buf, ptr - buf, 0);

    Recv(sHost, buf, 4, 0);
    if(buf[1] != SOCKS5_REP_SUCCEEDED)     // check reply code
    {
        std::cerr << "socks5: got error response from SOCKS server" << std::endl;
        return -1;
    }
    ptr = buf + 4;
    switch(buf[3])                           // case by ATYP
    {
    case 1:                                     // IP v4 ADDR
        Recv(sHost, ptr, 4, 0);
        break;
    case 3:                                     // DOMAINNAME
        Recv(sHost, ptr, 1, 0);
        Recv(sHost, ptr + 1, *(unsigned char*)ptr, 0);
        break;
    case 4:                                     // IP v6 ADDR
        Recv(sHost, ptr, 16, 0);
        break;
    }
    char udp_port_byte[2];
    Recv(sHost, udp_port_byte, 2, 0); // receive UDP port;
    return ntohs(*(short*)(&udp_port_byte));
}

int socks5_send_udp_data(SOCKET sHost, const std::string &server, uint16_t port, const std::string &dst_host, uint16_t dst_port, const std::string &data, const uint8_t fragment)
{
    char buf[BUF_SIZE], *ptr = buf;
    PUT_BYTE(ptr++, 0);
    PUT_BYTE(ptr++, 0);
    PUT_BYTE(ptr++, fragment);

    putSocksAddress(&ptr, dst_host, dst_port);

    std::string realdata = data, address = dst_host;
    realdata.insert(0, buf, ptr - buf);

    std::string server_addr = server;
    if(!isIPv4(server_addr) && !isIPv6(server_addr))
        server_addr = hostnameToIPAddr(server_addr);

    if(isIPv4(server_addr))
    {
        struct sockaddr_in addr4 = {};
        addr4.sin_family = AF_INET;
        inet_pton(AF_INET, server_addr.data(), (struct in_addr *)&addr4.sin_addr.s_addr);
        addr4.sin_port = htons(port);
        return sendto(sHost, realdata.data(), realdata.size(), 0, reinterpret_cast<struct sockaddr *>(&addr4), sizeof(addr4));
    }
    else if(isIPv6(server_addr))
    {
        sockaddr_in6 addr6 = {};
        addr6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, server_addr.data(), (struct in_addr6 *)&addr6.sin6_addr);
        addr6.sin6_port = htons(port);
        return sendto(sHost, realdata.data(), realdata.size(), 0, reinterpret_cast<struct sockaddr *>(&addr6), sizeof(addr6));
    }
    return -1;
}

int socks5_get_udp_data(SOCKET sHost, char *buf, int len)
{
    char buffer[BUF_SIZE], *ptr = buffer;
    int recv_len, offset = 4;
    if((recv_len = recvfrom(sHost, buffer, BUF_SIZE - 1, 0, NULL, NULL)) == -1)
        return -1;
    if(buffer[0] != 0 || buffer[1] != 0) /// reserved
        return -1;
    if(buffer[2] != 0) /// fragmented, not supported
        return -1;
    ptr = buffer + 4;
    switch(buffer[3])                           // case by ATYP
    {
    case 1:                                     // IP v4 ADDR
        offset += 4;
        ptr += 4;
        break;
    case 3:                                     // DOMAINNAME
        offset += *ptr + 1;
        ptr += *ptr + 1;
        break;
    case 4:                                     // IP v6 ADDR
        offset += 16;
        ptr += 16;
        break;
    }
    offset += 2;
    ptr += 2; /// port
    int reallen = std::min(recv_len - offset, len);
    memcpy(buf, ptr, reallen);
    return reallen;
}
