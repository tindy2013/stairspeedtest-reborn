#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <string>

#ifdef _WIN32
#ifndef WINVER
#define WINVER 0x0501
#endif // WINVER
#include <ws2tcpip.h>
#include <winsock2.h>
#else
//translate windows functions to linux functions
#include <unistd.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR	(-1)
#define closesocket close
#define SOCKADDR_IN sockaddr_in
#define ZeroMemory(d,l) memset((d), 0, (l))
#define ioctlsocket ioctl
#define SD_BOTH SHUT_RDWR
#define SD_RECEIVE SHUT_RD
#define SD_SEND SHUT_WR
#ifndef SA_INTERRUPT
#define SA_INTERRUPT 0 //ignore this setting
#endif
#ifndef HAVE_MSG_NOSIGNAL
#define MSG_NOSIGNAL 0 //ignore this setting
#endif
#ifndef __hpux
#include <sys/select.h>
#endif /* __hpux */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <signal.h>
typedef sockaddr *LPSOCKADDR;
#endif // _WIN32

#define BUF_SIZE 1024

SOCKET initSocket(int af, int type, int protocol);
int getNetworkType(std::string addr);
int Send(SOCKET sHost, const char* data, int len, int flags);
int Recv(SOCKET sHost, char* data, int len, int flags);
int socks5_do_auth_userpass(SOCKET sHost, std::string user, std::string pass);
int setTimeout(SOCKET s, int timeout);
int setSocketBlocking(SOCKET s, bool blocking);
int startConnect(SOCKET sHost, std::string addr, int port);
int simpleSend(std::string addr, int port, std::string data);
int send_simple(SOCKET sHost, std::string data);
std::string hostnameToIPAddr(std::string host);
int connectSocks5(SOCKET sHost, std::string username, std::string password);
int connectThruSocks(SOCKET sHost, std::string host, int port);
int connectThruHTTP(SOCKET sHost, std::string username, std::string password, std::string dsthost, int dstport);
int checkPort(int startport);

std::string sockaddrToIPAddr(sockaddr *addr);
uint16_t socks5_start_udp(SOCKET sHost, const std::string &address, const uint16_t port);
int socks5_send_udp_data(SOCKET sHost, const std::string &server, uint16_t port, const std::string &dst_host, uint16_t dst_port, const std::string &data, const uint8_t fragment = 0);
int socks5_get_udp_data(SOCKET sHost, char *buf, int len);
std::tuple<std::string, uint16_t> getSocksAddress(const std::string &data);

#endif // SOCKET_H_INCLUDED
