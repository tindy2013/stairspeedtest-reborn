#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <bits/stdc++.h>
#include <chrono>
#include <memory>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
#ifndef WINVER
#define WINVER 0x0501
#endif // WINVER
#include <ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#else
//translate windows functions to linux functions
#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR	(-1)
#define closesocket close
#define SOCKADDR_IN sockaddr_in
#define ZeroMemory(d,l) memset((d), 0, (l))
#ifdef _MACOS
#define ioctlsocket ioctl
#define SA_INTERRUPT SV_INTERRUPT
#define MSG_NOSIGNAL 0 //ignore this signal
#endif // _MACOS
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

#define BUF_SIZE 8192

using namespace std;

int Send(SOCKET sHost, const char* data, int len, int flags);
int Recv(SOCKET sHost, char* data, int len, int flags);
int socks5_do_auth_userpass(SOCKET sHost, string user, string pass);
int setTimeout(SOCKET s, int timeout);
int startConnect(SOCKET sHost, string addr, int port);
int simpleSend(string addr, int port, string data);
int send_simple(SOCKET sHost, string data);
char* hostnameToIPv4(string host, int port);
int connectSocks5(SOCKET sHost, string username, string password);
int connectThruSocks(SOCKET sHost, string host, string addr, int port);
int checkPort(int startport);

#endif // SOCKET_H_INCLUDED
