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

using namespace std;
using namespace chrono;

int def_timeout = 2000;

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

//int WSAAPI getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

#ifndef _WIN32
void connect_sigalarm(int signo)
{
    //cerr<<"connect alarm"<<endl;
    return;
}
#endif // _WIN32

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

int socks5_do_auth_userpass(SOCKET sHost, string user, string pass)
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
    struct timeval timeo = {0, timeout*1000};
    ret = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeo, sizeof(timeo));
    ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeo, sizeof(timeo));
#endif
    def_timeout = timeout;
    return ret;
}

int connect_adv(SOCKET sockfd, const struct sockaddr* addr, int addrsize)
{
    int ret = -1;
#ifdef _WIN32
    int error = 1;
    timeval tm;
    fd_set set;
    unsigned long ul = 1;
    int len = sizeof(int);
    ioctlsocket(sockfd, FIONBIO, &ul); //set to non-blocking mode
    if(connect(sockfd, addr, addrsize) == -1)
    {
        tm.tv_sec = 0;
        tm.tv_usec = def_timeout * 1000;
        FD_ZERO(&set);
        FD_SET(sockfd, &set);
        if(select(sockfd+1, NULL, &set, NULL, &tm) > 0)
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

    ul = 0;
    ioctlsocket(sockfd, FIONBIO, &ul); //set to blocking mode
#else

//signal
    struct sigaction act, oldact;
    act.sa_handler = connect_sigalarm;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    //act.sa_flags = 0x20000000;
    sigaction(SIGALRM, &act, &oldact);
    if(alarm(def_timeout / 1000) != 0)
    {
        //cerr<<"connect timeout set"<<endl;
    }

    ret = connect(sockfd, addr, addrsize);
    if(ret < 0)
    {
        close(sockfd);
        if (errno == EINTR)
        {
            //errno = TIMEOUT;
            //cerr<<"connect timeout"<<endl;
            alarm(0);
            return 1;
        }
    }
    alarm(0);


//setsockopt
/*
    int flags = 0, error = 0;
    fd_set  rset, wset;
    socklen_t   len = sizeof(error);
    struct timeval  ts;

    ts.tv_sec = 0;
    ts.tv_usec = def_timeout * 1000;

    //clear out descriptor sets for select
    //add socket to the descriptor sets
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;    //structure assignment ok

    //set socket nonblocking flag
    if( (flags = fcntl(sockfd, F_GETFL, 0)) < 0)
        return -1;

    if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
        return -1;

    //initiate non-blocking connect
    if( (ret = connect(sockfd, addr, addrsize)) < 0 )
        if (errno != EINPROGRESS)
            return -1;

    if(ret == 0)    //then connect succeeded right away
        goto done;

    //we are waiting for connect to complete now
    if( (ret = select(sockfd + 1, &rset, &wset, NULL, &ts)) < 0)
        return -1;
    if(ret == 0){   //we had a timeout
        errno = ETIMEDOUT;
        return -1;
    }

    //we had a positivite return so a descriptor is ready
    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)){
        if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return -1;
    }else
        return -1;

    if(error){  //check if we had a socket error
        errno = error;
        return -1;
    }

done:
    //put socket back in blocking mode
    if(fcntl(sockfd, F_SETFL, flags) < 0)
        return -1;

    return 0;
*/
#endif // _WIN32
    return ret;
}

int startConnect(SOCKET sHost, string addr, int port)
{
    int retVal;
    SOCKADDR_IN servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(addr.data());
    servAddr.sin_port = htons((short)port);
    retVal = connect_adv(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
    //retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
    return retVal;
}

int send_simple(SOCKET sHost, string data)
{
    unsigned int retVal = Send(sHost, data.data(), data.size(), 0);
    return retVal;
}

int simpleSend(string addr, int port, string data)
{
    SOCKET sHost = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
    setTimeout(sHost, 1000);
    if(sHost == INVALID_SOCKET)
        return INVALID_SOCKET;
    if(startConnect(sHost, addr, port) == SOCKET_ERROR)
        return SOCKET_ERROR;
    unsigned int retVal = send_simple(sHost, data);
    if(retVal == data.size())
    {
        closesocket(sHost);
#ifdef _WIN32
        return WSAGetLastError();
#else
        return -1;
#endif // _WIN32
    }
    else
    {
        closesocket(sHost);
        return 1;
    }
}

char* hostnameToIPv4(string host)
{
    //old function
    /*
    struct in_addr inaddr;
    //string retstr;
    hostent *h = gethostbyname(host.data());
    if(h == NULL) return const_cast<char* >("");
    inaddr.s_addr = *(u_long*)h->h_addr_list[0];
    //addr = inet_ntoa(inaddr);
    return inet_ntoa(inaddr);
    */
    //new function
    int retVal;
    string portstr = to_string(80);
    struct sockaddr_in *target;
    struct addrinfo hint = {}, *retAddrInfo, *cur;
    retVal = getaddrinfo(host.data(), portstr.data(), &hint, &retAddrInfo);
    if(retVal != 0)
        return const_cast<char*>("");

    for(cur = retAddrInfo; cur != NULL; cur=cur->ai_next)
    {
        if(cur->ai_family == AF_INET)
        {
            target = (struct sockaddr_in *)cur->ai_addr;
            return inet_ntoa(target->sin_addr);
        }
    }
    return const_cast<char*>("");

}

int connectSocks5(SOCKET sHost, string username, string password)
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
        cerr<<"socks5: connect not accepted"<<endl;
        return -1;
    }
    int auth_method = bufRecv[1];
    int auth_result = -1;
    switch (auth_method)
    {
    case SOCKS5_AUTH_REJECT:
        cerr<<"socks5: no acceptable authentication method\n"<<endl;
        return -1;                              // fail

    case SOCKS5_AUTH_NOAUTH:
        // nothing to do
        auth_result = 0;
        break;

    case SOCKS5_AUTH_USERPASS:
        auth_result = socks5_do_auth_userpass(sHost, username, password);
        break;

    default:
        return -1;
    }
    if ( auth_result != 0 )
    {
        cerr<<"socks5: authentication failed."<<endl;
        return -1;
    }
    return 0;
}

int connectThruSocks(SOCKET sHost, string host, string addr, int port)
{
    char buf[BUF_SIZE];//bufRecv[BUF_SIZE];
    ZeroMemory(buf, BUF_SIZE);
    char* ptr;
    /* destination target host and port */
    const char* dest_host = NULL;
    struct sockaddr_in dest_addr = {};
    u_short dest_port = 0;

    dest_host = host.c_str();
    //dest_addr.sin_addr = addr.data();
    dest_port = port;
    ptr = buf;
    PUT_BYTE(ptr++, 5);                        // SOCKS version (5)
    PUT_BYTE(ptr++, 1);                        // CMD: CONNECT
    PUT_BYTE(ptr++, 0);                        // FLG: 0
    if (dest_addr.sin_addr.s_addr == 0)
    {
        // resolved by SOCKS server
        int len;
        PUT_BYTE(ptr++, 3);                    // ATYP: DOMAINNAME
        len = strlen(dest_host);
        PUT_BYTE(ptr++, len);                  // DST.ADDR (len)
        memcpy(ptr, dest_host, len);          //(hostname)
        ptr += len;
    }
    else
    {
        // resolved locally
        PUT_BYTE(ptr++, 1);                   // ATYP: IPv4
        memcpy(ptr, &dest_addr.sin_addr.s_addr, sizeof(dest_addr.sin_addr));
        ptr += sizeof(dest_addr.sin_addr);
    }
    PUT_BYTE(ptr++, dest_port>>8);     // DST.PORT
    PUT_BYTE(ptr++, dest_port & 0xFF);
    Send(sHost, buf, ptr - buf, 0);
    Recv(sHost, buf, 4, 0);
    if(buf[1] != SOCKS5_REP_SUCCEEDED)     // check reply code
    {
        cerr<<"socks5: got error response from SOCKS server"<<endl;
        return -1;
    }
    ptr = buf + 4;
    switch(buf[3])                           /* case by ATYP */
    {
    case 1:                                     /* IP v4 ADDR*/
        Recv(sHost, ptr, 4 + 2, 0);
        break;
    case 3:                                     /* DOMAINNAME */
        Recv(sHost, ptr, 1, 0);
        Recv(sHost, ptr + 1, *(unsigned char*)ptr + 2, 0);
        break;
    case 4:                                     /* IP v6 ADDR */
        Recv(sHost, ptr, 16 + 2, 0);
        break;
    }
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
