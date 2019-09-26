#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <mutex>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "misc.h"
#include "socket.h"
#include "logger.h"
#include "webget.h"

using namespace std;
using namespace chrono;

#define MAX_FILE_SIZE 512*1024*1024
#define BUF_SIZE 8192

//for use of multi-thread socket test
typedef lock_guard<mutex> guarded_mutex;
long long received_bytes = 0;
int launched = 0, still_running = 0, buffer_size = 4096;
bool EXIT_FLAG = false;
mutex received_mutex, thread_count_mutex, launched_mutex, exit_flag_mutex;

static inline void launch_acc()
{
    guarded_mutex guard(launched_mutex);
    launched++;
}

static inline void running_acc()
{
    guarded_mutex guard(thread_count_mutex);
    still_running++;
}

static inline void running_dec()
{
    guarded_mutex guard(thread_count_mutex);
    still_running--;
}

static inline int safe_read_running()
{
    int retVal;
    guarded_mutex guard(thread_count_mutex);
    retVal = still_running;
    return retVal;
}

static inline int safe_read_launched()
{
    int retVal;
    guarded_mutex guard(launched_mutex);
    retVal = launched;
    return retVal;
}

static inline bool safe_read_exit_flag()
{
    bool retVal;
    guarded_mutex guard(exit_flag_mutex);
    retVal = EXIT_FLAG;
    return retVal;
}

static inline void safe_set_exit_flag()
{
    guarded_mutex guard(exit_flag_mutex);
    EXIT_FLAG = true;
}

static inline void append_recv_bytes(int received)
{
    guarded_mutex guard(received_mutex);
    received_bytes += received;
}

static inline void draw_progress_dl(int progress, long long this_bytes)
{
    cerr<<"\r[";
    for(int j = 0; j < progress; j++)
    {
        cerr<<"=";
    }
    if(progress < 20)
        cerr<<">";
    else
        cerr<<"]";
    cerr<<" "<<progress * 5<<"% "<<speedCalc(this_bytes);
}

static inline void draw_progress_icon(int progress)
{
    cerr<<"\r ";
    switch(progress % 4)
    {
    case 1:
        cerr<<"\\";
        break;
    case 2:
        cerr<<"|";
        break;
    case 3:
        cerr<<"/";
        break;
    default:
        cerr<<"-";
        break;
    }
}

static inline void draw_progress_ul(int progress, long long this_bytes)
{
    draw_progress_icon(progress);
    cerr<<" "<<speedCalc(this_bytes);
}

int _thread_download(string host, int port, string uri, string localaddr, int localport, string username, string password, bool useTLS = false)
{
    launch_acc();
    running_acc();
    char bufRecv[BUF_SIZE];
    int retVal, cur_len/*, recv_len = 0*/;
    SOCKET sHost;
    string request = "GET " + uri + " HTTP/1.1\r\n"
                    "Host: " + host + "\r\n"
                    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36\r\n\r\n";

    sHost = socket(getNetworkType(localaddr), SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
        goto end;
    setTimeout(sHost, 3000);
    if(startConnect(sHost, localaddr, localport) == SOCKET_ERROR)
        goto end;
    if(connectSocks5(sHost, username, password) == -1)
        goto end;
    if(connectThruSocks(sHost, host, port) == -1)
        goto end;

    if(useTLS)
    {
        SSL_CTX *ctx;
        SSL *ssl;

        ctx = SSL_CTX_new(TLS_client_method());
        if(ctx == NULL)
        {
            ERR_print_errors_fp(stderr);
            goto end;
        }

        //SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sHost);

        if(SSL_connect(ssl) != 1)
        {
            ERR_print_errors_fp(stderr);
        }
        else
        {
            SSL_write(ssl, request.data(), request.size());
            while(1)
            {
                cur_len = SSL_read(ssl, bufRecv, BUF_SIZE - 1);
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
                append_recv_bytes(cur_len);
                if(safe_read_exit_flag())
                    break;
            }
        }
        SSL_clear(ssl);

    }
    else
    {
        retVal = Send(sHost, request.data(), request.size(), 0);
        if (SOCKET_ERROR == retVal)
        {
            closesocket(sHost);
            return -1;
        }
        while(1)
        {
            cur_len = Recv(sHost, bufRecv, BUF_SIZE - 1, 0);
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
            if(safe_read_exit_flag())
                break;
        }
    }

end:
    closesocket(sHost);
    running_dec();
    return 0;
}

int _thread_upload(string host, int port, string uri, string localaddr, int localport, string username, string password, bool useTLS = false)
{
    launch_acc();
    running_acc();
    char bufRecv[BUF_SIZE];
    int retVal, cur_len;
    SOCKET sHost;
    string request = "POST " + uri + " HTTP/1.1\r\n"
                    "Content-Length: 134217728\r\n"
                    "Host: " + host + "\r\n\r\n";
    string post_data;

    sHost = socket(getNetworkType(localaddr), SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
        goto end;
    setTimeout(sHost, 3000);
    if(startConnect(sHost, localaddr, localport) == SOCKET_ERROR)
        goto end;
    if(connectSocks5(sHost, username, password) == -1)
        goto end;
    if(connectThruSocks(sHost, host, port) == -1)
        goto end;

    if(useTLS)
    {
        SSL_CTX *ctx;
        SSL *ssl;

        ctx = SSL_CTX_new(TLS_client_method());
        if(ctx == NULL)
        {
            ERR_print_errors_fp(stderr);
            goto end;
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sHost);

        if(SSL_connect(ssl) != 1)
        {
            ERR_print_errors_fp(stderr);
        }
        else
        {
            SSL_write(ssl, request.data(), request.size());
            while(1)
            {
                post_data = rand_str(128);
                cur_len = SSL_write(ssl, post_data.data(), post_data.size());
                if(cur_len == SOCKET_ERROR)
                {
                    break;
                }
                append_recv_bytes(cur_len);
                if(safe_read_exit_flag())
                    break;
            }
        }
        SSL_clear(ssl);
    }
    else
    {
        retVal = Send(sHost, request.data(), request.size(), 0);
        if (SOCKET_ERROR == retVal)
        {
            closesocket(sHost);
            return -1;
        }
        while(1)
        {
            post_data = rand_str(128);
            cur_len = Send(sHost, post_data.data(), post_data.size(), 0);
            if(cur_len == SOCKET_ERROR)
            {
                break;
            }
            append_recv_bytes(cur_len);
            if(safe_read_exit_flag())
                break;
        }
    }

end:
    closesocket(sHost);
    running_dec();
    return 0;
}

int _thread_upload_curl(nodeInfo *node, string url, string proxy)
{
    launch_acc();
    running_acc();
    long retVal = curlPost(url, rand_str(8388608), proxy);
    node->ulSpeed = speedCalc(retVal * 1.0);
    running_dec();
    return 0;
}

int perform_test(nodeInfo *node, string localaddr, int localport, string username, string password, int thread_count)
{
    writeLog(LOG_TYPE_FILEDL, "Multi-thread download test started.");
    //prep up vars first
    string host, uri, testfile = node->testFile;
    vector<string> args;
    int port = 0, i;
    bool useTLS = false;

    writeLog(LOG_TYPE_FILEDL, "Fetch target: " + testfile);
    if(regMatch(testfile, "^https://(.*)"))
        useTLS = true;
    testfile = regReplace(testfile, "^(http|https)://", "");
    host = testfile.substr(0, testfile.find("/"));
    uri = testfile.substr(testfile.find("/"));
    if(regFind(host, "\\[(.*)\\]")) //IPv6
    {
        args = split(regReplace(host, "\\[(.*)\\](.*)", "$1,$2"), ",");
        if(args.size() == 2) //with port
            port = stoi(args[1].substr(1));
        host = args[0];
    }
    else if(strFind(host, ":"))
    {
        port = stoi(host.substr(host.rfind(":") + 1));
        host = host.substr(0, host.rfind(":"));
    }
    if(port == 0)
    {
        if(useTLS)
            port = 443;
        else
            port = 80;
    }
    received_bytes = 0;
    EXIT_FLAG = false;

    if(useTLS)
    {
        writeLog(LOG_TYPE_FILEDL, "Found HTTPS URL. Initializing OpenSSL library.");
        SSL_load_error_strings();
        SSL_library_init();
        OpenSSL_add_all_algorithms();
    }
    else
    {
        writeLog(LOG_TYPE_FILEDL, "Found HTTP URL.");
    }

    int running;
    thread threads[thread_count];
    for(i = 0; i != thread_count; i++)
    {
        writeLog(LOG_TYPE_FILEDL, "Starting up thread #" + to_string(i + 1) + ".");
        threads[i]=thread(_thread_download, host, port, uri, localaddr, localport, username, password, useTLS);
    }
    while(!safe_read_launched())
        sleep(20); //wait until any one of the threads start up

    writeLog(LOG_TYPE_FILEDL, "All threads launched. Start accumulating data.");
    auto start = steady_clock::now();
    long long transferred_bytes = 0, last_bytes = 0, this_bytes = 0, max_speed = 0;
    for(i = 1; i < 21; i++)
    {
        sleep(500); //accumulate data
        received_mutex.lock(); //stop the receive
        this_bytes = (received_bytes - transferred_bytes) * 2; //these bytes were received in 0.5s
        transferred_bytes = received_bytes;
        //cerr<<this_bytes<<" "<<last_bytes<<endl;
        sleep(5); //slow down to prevent some problem
        received_mutex.unlock();
        node->rawSpeed[i - 1] = this_bytes;
        if(i % 2 == 0)
        {
            max_speed = max(max_speed, (this_bytes + last_bytes) / 2); //pick 2 speed point and get average before calculating max speed
        }
        else
        {
            last_bytes = this_bytes;
        }
        running = safe_read_running();
        writeLog(LOG_TYPE_FILEDL, "Running threads: " + to_string(running) + ", total received bytes: " + to_string(transferred_bytes) \
                 + ", current received bytes: " + to_string(this_bytes) + ".");
        if(!running)
            break;
        draw_progress_dl(i, this_bytes);
    }
    cerr<<endl;
    writeLog(LOG_TYPE_FILEDL, "Test completed. Terminate all threads.");
    safe_set_exit_flag(); //terminate all threads right now
    received_mutex.lock(); //lock it to prevent any further data writing
    auto end = steady_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    int deltatime = duration.count() + 1;//add 1 to prevent some error
    //cerr<<deltatime<<" "<<received_bytes<<endl;
    sleep(5); //slow down to prevent some problem
    node->duration = deltatime;
    node->totalRecvBytes = received_bytes;
    node->avgSpeed = speedCalc(received_bytes * 1000.0 / deltatime);
    node->maxSpeed = speedCalc(max_speed);
    if(node->avgSpeed == "0.00B")
    {
        node->avgSpeed = "N/A";
        node->maxSpeed = "N/A";
    }
    writeLog(LOG_TYPE_FILEDL, "Downloaded " + to_string(received_bytes) + " bytes in " + to_string(deltatime) + " milliseconds.");
    received_mutex.unlock(); //unlock to make threads continue running
    for(int i = 0; i < thread_count; i++)
    {
        if(threads[i].joinable())
            threads[i].join();//wait until all threads has exited
    }
    writeLog(LOG_TYPE_FILEDL, "Multi-thread download test completed.");
    return 0;
}

int upload_test(nodeInfo *node, string localaddr, int localport, string username, string password)
{
    writeLog(LOG_TYPE_FILEUL, "Upload test started.");
    //prep up vars first
    string host, uri, testfile = node->ulTarget;
    vector<string> args;
    int port = 0, i;
    bool useTLS = false;

    writeLog(LOG_TYPE_FILEUL, "Upload destination: " + testfile);
    if(regMatch(testfile, "^https://(.*)"))
        useTLS = true;
    testfile = regReplace(testfile, "^(http|https)://", "");
    host = testfile.substr(0, testfile.find("/"));
    uri = testfile.substr(testfile.find("/"));
    if(regFind(host, "\\[(.*)\\]")) //IPv6
    {
        args = split(regReplace(host, "\\[(.*)\\](.*)", "$1,$2"), ",");
        if(args.size() == 2) //with port
            port = stoi(args[1].substr(1));
        host = args[0];
    }
    else if(strFind(host, ":"))
    {
        port = stoi(host.substr(host.find(":") + 1));
        host = host.substr(0, host.find(":"));
    }
    if(port == 0)
    {
        if(useTLS)
            port = 443;
        else
            port = 80;
    }
    received_bytes = 0;
    EXIT_FLAG = false;

    if(useTLS)
    {
        writeLog(LOG_TYPE_FILEUL, "Found HTTPS URL. Initializing OpenSSL library.");
        SSL_load_error_strings();
        SSL_library_init();
        OpenSSL_add_all_algorithms();
    }
    else
    {
        writeLog(LOG_TYPE_FILEUL, "Found HTTP URL.");
    }

    int running;
    thread workers[2];
    for(i = 0; i < 1; i++)
    {
        writeLog(LOG_TYPE_FILEUL, "Starting up worker thread #" + to_string(i + 1) + ".");
        workers[i] = thread(_thread_upload, host, port, uri, localaddr, localport, username, password, useTLS);
    }
    while(!safe_read_launched())
        sleep(20); //wait until worker thread starts up

    writeLog(LOG_TYPE_FILEUL, "Worker threads launched. Start accumulating data.");
    auto start = steady_clock::now();
    long long transferred_bytes = 0, this_bytes = 0;
    for(i = 1; i < 11; i++)
    {
        sleep(1000); //accumulate data
        received_mutex.lock(); //stop the process
        this_bytes = received_bytes - transferred_bytes;
        transferred_bytes = received_bytes;
        sleep(5); //slow down to prevent some problem
        received_mutex.unlock();
        running = safe_read_running();
        writeLog(LOG_TYPE_FILEUL, "Running worker threads: " + to_string(running) + ", total sent bytes: " + to_string(transferred_bytes) \
                 + ", current sent bytes: " + to_string(this_bytes) + ".");
        if(!running)
            break;
        draw_progress_ul(i, this_bytes);
    }
    cerr<<endl;
    writeLog(LOG_TYPE_FILEUL, "Test completed. Terminate worker threads.");
    safe_set_exit_flag(); //terminate worker thread right now
    received_mutex.lock(); //lock it to prevent any further data writing
    auto end = steady_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    int deltatime = duration.count() + 1;//add 1 to prevent some error
    sleep(5); //slow down to prevent some problem
    node->ulSpeed = speedCalc(received_bytes * 1000.0 / deltatime);
    if(node->ulSpeed == "0.00B")
    {
        node->ulSpeed = "N/A";
    }
    writeLog(LOG_TYPE_FILEUL, "Uploaded " + to_string(received_bytes) + " bytes in " + to_string(deltatime) + " milliseconds.");
    node->totalRecvBytes += received_bytes;
    received_mutex.unlock(); //unlock to make worker thread continue running
    for(auto &x : workers)
        if(x.joinable())
            x.join();//wait until worker thread has exited
    writeLog(LOG_TYPE_FILEUL, "Upload test completed.");
    return 0;
}

int upload_test_curl(nodeInfo *node, string localaddr, int localport, string username, string password)
{
    writeLog(LOG_TYPE_FILEUL, "Upload test started.");
    string url = node->ulTarget;
    string proxy = buildSocks5ProxyString(localaddr, localport, username, password);
    writeLog(LOG_TYPE_FILEUL, "Starting up worker thread.");
    thread worker = thread(_thread_upload_curl, node, url, proxy);
    while(!safe_read_launched())
        sleep(20); //wait until worker thread starts up

    writeLog(LOG_TYPE_FILEUL, "Worker thread launched. Wait for it to exit.");
    int progress = 0, running = 0;
    while(true)
    {
        sleep(200);
        running = safe_read_running();
        if(!running)
            break;
        draw_progress_icon(progress);
        progress++;
    }
    cerr<<endl;

    writeLog(LOG_TYPE_FILEUL, "Reported upload speed: " + node->ulSpeed);
    if(worker.joinable())
        worker.join();//wait until worker thread has exited
    writeLog(LOG_TYPE_FILEUL, "Upload test completed.");
    return 0;
}
