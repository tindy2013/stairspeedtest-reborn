#include <bits/stdc++.h>
#include <chrono>

#include "misc.h"
#include "socket.h"

using namespace std;
using namespace chrono;

#define MAX_FILE_SIZE 512*1024*1024
#define BUF_SIZE 8192

//for use of multi-thread socket test
typedef lock_guard<mutex> guarded_mutex;
long long received_bytes = 0;
int launched = 0, still_running = 0, buffer_size = 4096;
bool EXIT_FLAG = false;
mutex received_mutex, thread_count_mutex, launched_mutex;

void launch_acc()
{
    guarded_mutex guard(launched_mutex);
    launched++;
}

void running_acc()
{
    guarded_mutex guard(thread_count_mutex);
    still_running++;
}

void running_dec()
{
    guarded_mutex guard(thread_count_mutex);
    still_running--;
}

int safe_read_running()
{
    int retVal;
    guarded_mutex guard(thread_count_mutex);
    retVal = still_running;
    return retVal;
}

int safe_read_launched()
{
    int retVal;
    guarded_mutex guard(launched_mutex);
    retVal = launched;
    return retVal;
}

static inline void append_recv_bytes(int received)
{
    guarded_mutex guard(received_mutex);
    //received_mutex.lock();
    received_bytes += received;
    //received_mutex.unlock();
}

static inline void draw_progress(int progress, long long this_bytes)
{
    cerr<<"[";
    for(int j = 0; j < progress; j++)
    {
        cerr<<"=";
    }
    if(progress < 20)
    cerr<<">";
    cerr<<"] "<<progress * 5<<"% "<<speedCalc(this_bytes)<<"\r";
}

int _thread_download(string host, string uri, string localaddr, int localport)
{
    launch_acc();
    running_acc();
    char bufRecv[BUF_SIZE];
    int retVal, cur_len/*, recv_len = 0*/;
    SOCKET sHost;
    string request;

    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
        goto end;
    setTimeout(sHost, 3000);
    if(startconnect(sHost, localaddr, localport) == SOCKET_ERROR)
        goto end;
    if(connectSocks5(sHost, "", "") == -1)
        goto end;
    if(connectThruSocks(sHost, host, "", 80) == -1)
        goto end;
    request = "GET " + uri + " HTTP/1.1\r\n"
              "Host: " + host + "\r\n"
              "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36\r\n\r\n";
    retVal = Send(sHost, request.data(), request.size(), 0);
    if (SOCKET_ERROR == retVal)
    {
        closesocket(sHost);
        return -1;
    }
    while(1)
    {
        cur_len = Recv(sHost, bufRecv, BUF_SIZE, 0);
        if(cur_len < 0)
        {
            if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        if(cur_len == 0)
            break;
        //recv_len += cur_len;
        append_recv_bytes(cur_len);
        //if(recv_len >= MAX_FILE_SIZE) break;
        if(EXIT_FLAG)
            break;
    }
end:
    closesocket(sHost);
    running_dec();
    return 0;
}

int perform_test(nodeInfo *node, string testfile, string localaddr, int localport, int thread_count)
{
    //prep up vars first
    string host, uri;
    testfile = regreplace(testfile, "^(http|https)://", "");
    host = testfile.substr(0, testfile.find("/"));
    uri = testfile.substr(testfile.find("/"));
    received_bytes = 0;
    EXIT_FLAG = false;

    int running;
    thread threads[thread_count];
    for(int i = 0; i != thread_count; i++)
    {
        threads[i]=thread(_thread_download, host, uri, localaddr, localport);
    }
    while(!safe_read_launched())
        sleep(20);
    auto start = steady_clock::now();
    long long last_bytes = 0, this_bytes = 0, max_speed = 0;
    for(int i = 1; i < 21; i++)
    {
        received_mutex.lock();
        this_bytes = (received_bytes - last_bytes) * 2; //these bytes were received in 0.5s
        last_bytes = received_bytes;
        //cerr<<this_bytes<<" "<<last_bytes<<endl;
        sleep(5);//slow down to prevent some problem
        received_mutex.unlock();
        node->raw_speed[i - 1] = this_bytes;
        max_speed = max(max_speed, this_bytes);
        running = safe_read_running();
        if(!running)
            break;
        draw_progress(i, this_bytes);
        sleep(500);
    }
    cerr<<endl;
    received_mutex.lock();//lock it to prevent any further data writing
    auto end = steady_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    int deltatime = duration.count() + 1;//add 1 to prevent some error
    //cerr<<deltatime<<" "<<received_bytes<<endl;
    sleep(5);//slow down to prevent some problem
    node->duration_ms = deltatime;
    node->total_recv_bytes = received_bytes;
    node->avgspeed = speedCalc(received_bytes * 1000.0 / deltatime);
    node->maxspeed = speedCalc(max_speed);
    EXIT_FLAG = true;
    received_mutex.unlock();//unlock to make threads continue running
    for(int i = 0; i < thread_count; i++)
    {
        if(threads[i].joinable())
            threads[i].join();//wait until all threads has exited
    }
    return 0;
}
