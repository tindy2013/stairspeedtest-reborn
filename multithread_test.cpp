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
#include "printout.h"
#include "webget.h"
#include "nodeinfo.h"

using namespace std::chrono;
using namespace std::__cxx11;

#define MAX_FILE_SIZE 512*1024*1024
#define BUF_SIZE 8192

//for use of site ping
const int times_to_ping = 10, fail_limit = 3;

//for use of multi-thread socket test
typedef std::lock_guard<std::mutex> guarded_mutex;
int received_bytes = 0;
int launched = 0, still_running = 0, buffer_size = 4096;
bool EXIT_FLAG = false;
std::mutex received_mutex, thread_count_mutex, launched_mutex, exit_flag_mutex;

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

static inline void draw_progress_dl(int progress, int this_bytes)
{
    std::cerr<<"\r[";
    for(int j = 0; j < progress; j++)
    {
        std::cerr<<"=";
    }
    if(progress < 20)
        std::cerr<<">";
    else
        std::cerr<<"]";
    std::cerr<<" "<<progress * 5<<"% "<<speedCalc(this_bytes);
}

static inline void draw_progress_icon(int progress)
{
    std::cerr<<"\r ";
    switch(progress % 4)
    {
    case 1:
        std::cerr<<"\\";
        break;
    case 2:
        std::cerr<<"|";
        break;
    case 3:
        std::cerr<<"/";
        break;
    default:
        std::cerr<<"-";
        break;
    }
}

static inline void draw_progress_ul(int progress, int this_bytes)
{
    draw_progress_icon(progress);
    std::cerr<<" "<<speedCalc(this_bytes);
}

static inline void draw_progress_gping(int progress, int *values)
{
    std::cerr<<"\r[";
    for(int i = 0; i <= progress; i++)
    {
        std::cerr<<(values[i] == 0 ? "*" : "-");
    }
    if(progress == times_to_ping - 1)
    {
        std::cerr<<"]";
    }
    std::cerr<<" "<<progress + 1<<"/"<<times_to_ping<<" "<<values[progress]<<"ms";
}

int _thread_download(std::string host, int port, std::string uri, std::string localaddr, int localport, std::string username, std::string password, bool useTLS = false)
{
    launch_acc();
    running_acc();
    char bufRecv[BUF_SIZE];
    int retVal, cur_len/*, recv_len = 0*/;
    SOCKET sHost;
    std::string request = "GET " + uri + " HTTP/1.1\r\n"
                          "Host: " + host + "\r\n"
                          "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36\r\n\r\n";

    sHost = socket(getNetworkType(localaddr), SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
        goto end;
    setTimeout(sHost, 5000);
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

int _thread_upload(std::string host, int port, std::string uri, std::string localaddr, int localport, std::string username, std::string password, bool useTLS = false)
{
    launch_acc();
    running_acc();
    int retVal, cur_len;
    SOCKET sHost;
    std::string request = "POST " + uri + " HTTP/1.1\r\n"
                          "Content-Length: 134217728\r\n"
                          "Host: " + host + "\r\n\r\n";
    std::string post_data;

    sHost = socket(getNetworkType(localaddr), SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost)
        goto end;
    setTimeout(sHost, 5000);
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

int _thread_upload_curl(nodeInfo *node, std::string url, std::string proxy)
{
    launch_acc();
    running_acc();
    long retVal = curlPost(url, rand_str(8388608), proxy);
    node->ulSpeed = speedCalc(retVal * 1.0);
    running_dec();
    return 0;
}

int perform_test(nodeInfo *node, std::string localaddr, int localport, std::string username, std::string password, int thread_count)
{
    writeLog(LOG_TYPE_FILEDL, "Multi-thread download test started.");
    //prep up vars first
    std::string host, uri, testfile = node->testFile;
    int port = 0, i;
    bool useTLS = false;

    writeLog(LOG_TYPE_FILEDL, "Fetch target: " + testfile);
    urlParse(testfile, host, uri, port, useTLS);
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
    std::thread threads[thread_count];
    for(i = 0; i != thread_count; i++)
    {
        writeLog(LOG_TYPE_FILEDL, "Starting up thread #" + to_string(i + 1) + ".");
        threads[i] = std::thread(_thread_download, host, port, uri, localaddr, localport, username, password, useTLS);
    }
    while(!safe_read_launched())
        sleep(20); //wait until any one of the threads start up

    writeLog(LOG_TYPE_FILEDL, "All threads launched. Start accumulating data.");
    auto start = steady_clock::now();
    int transferred_bytes = 0, last_bytes = 0, this_bytes = 0, max_speed = 0;
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
            max_speed = std::max(max_speed, (this_bytes + last_bytes) / 2); //pick 2 speed point and get average before calculating max speed
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
    std::cerr<<std::endl;
    writeLog(LOG_TYPE_FILEDL, "Test completed. Terminate all threads.");
    safe_set_exit_flag(); //terminate all threads right now
    received_mutex.lock(); //lock it to prevent any further data writing
    auto end = steady_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    int deltatime = duration.count() + 1;//add 1 to prevent some error
    //cerr<<deltatime<<" "<<received_bytes<<endl;
    sleep(5); //slow down to prevent some problem
    node->totalRecvBytes = received_bytes;
    node->avgSpeed = speedCalc(received_bytes * 1000.0 / deltatime);
    node->maxSpeed = speedCalc(max_speed);
    received_mutex.unlock(); //unlock to make threads continue running
    if(node->avgSpeed == "0.00B")
    {
        node->avgSpeed = "N/A";
        node->maxSpeed = "N/A";
    }
    writeLog(LOG_TYPE_FILEDL, "Downloaded " + to_string(received_bytes) + " bytes in " + to_string(deltatime) + " milliseconds.");
    for(int i = 0; i < thread_count; i++)
    {
        if(threads[i].joinable())
            threads[i].join();//wait until all threads has exited
    }
    writeLog(LOG_TYPE_FILEDL, "Multi-thread download test completed.");
    return 0;
}

int upload_test(nodeInfo *node, std::string localaddr, int localport, std::string username, std::string password)
{
    writeLog(LOG_TYPE_FILEUL, "Upload test started.");
    //prep up vars first
    std::string host, uri, testfile = node->ulTarget;
    int port = 0, i;
    bool useTLS = false;

    writeLog(LOG_TYPE_FILEUL, "Upload destination: " + testfile);
    urlParse(testfile, host, uri, port, useTLS);
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
    std::thread workers[2];
    for(i = 0; i < 1; i++)
    {
        writeLog(LOG_TYPE_FILEUL, "Starting up worker thread #" + to_string(i + 1) + ".");
        workers[i] = std::thread(_thread_upload, host, port, uri, localaddr, localport, username, password, useTLS);
    }
    while(!safe_read_launched())
        sleep(20); //wait until worker thread starts up

    writeLog(LOG_TYPE_FILEUL, "Worker threads launched. Start accumulating data.");
    auto start = steady_clock::now();
    int transferred_bytes = 0, this_bytes = 0;
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
    std::cerr<<std::endl;
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

int upload_test_curl(nodeInfo *node, std::string localaddr, int localport, std::string username, std::string password)
{
    writeLog(LOG_TYPE_FILEUL, "Upload test started.");
    std::string url = node->ulTarget;
    std::string proxy = buildSocks5ProxyString(localaddr, localport, username, password);
    writeLog(LOG_TYPE_FILEUL, "Starting up worker thread.");
    std::thread worker = std::thread(_thread_upload_curl, node, url, proxy);
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
    std::cerr<<std::endl;

    writeLog(LOG_TYPE_FILEUL, "Reported upload speed: " + node->ulSpeed);
    if(worker.joinable())
        worker.join();//wait until worker thread has exited
    writeLog(LOG_TYPE_FILEUL, "Upload test completed.");
    return 0;
}

int sitePing(nodeInfo *node, std::string localaddr, int localport, std::string username, std::string password, std::string target)
{
    char bufRecv[BUF_SIZE];
    int retVal, cur_len;
    SOCKET sHost;
    std::string host, uri;
    int port = 0;
    bool useTLS = false;
    urlParse(target, host, uri, port, useTLS);
    std::string request = "GET " + uri + " HTTP/1.1\r\n"
                          "Host: " + host + "\r\n"
                          "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626.121 Safari/537.36\r\n\r\n";


    writeLog(LOG_TYPE_GPING, "Website ping started. Target: '" + target + "' . Proxy: '" + localaddr + ":" + to_string(localport) + "' .");
    int loopcounter = 0, succeedcounter = 0, failcounter = 0, totduration = 0;
    while(loopcounter < times_to_ping)
    {
        if(failcounter >= fail_limit)
        {
            writeLog(LOG_TYPE_GPING, "Fail limit exceeded. Stop now.");
            break;
        }
        auto start = steady_clock::now();
        sHost = socket(getNetworkType(localaddr), SOCK_STREAM, IPPROTO_TCP);
        if(INVALID_SOCKET == sHost)
        {
            failcounter++;
            node->rawSitePing[loopcounter] = 0;
            writeLog(LOG_TYPE_GPING, "ERROR: Could not create socket.");
            goto end;
        }
        setTimeout(sHost, 5000);
        if(startConnect(sHost, localaddr, localport) == SOCKET_ERROR)
        {
            failcounter++;
            node->rawSitePing[loopcounter] = 0;
            writeLog(LOG_TYPE_GPING, "ERROR: Connect to SOCKS5 server " + localaddr + ":" + to_string(localport) + " failed.");
            goto end;
        }
        if(connectSocks5(sHost, username, password) == -1)
        {
            failcounter++;
            node->rawSitePing[loopcounter] = 0;
            writeLog(LOG_TYPE_GPING, "ERROR: SOCKS5 server authentication failed.");
            goto end;
        }
        if(connectThruSocks(sHost, host, port) == -1)
        {
            failcounter++;
            node->rawSitePing[loopcounter] = 0;
            writeLog(LOG_TYPE_GPING, "ERROR: Connect to " + host + ":" + to_string(port) + " through SOCKS5 server failed.");
            goto end;
        }

        if(useTLS)
        {
            SSL_CTX *ctx;
            SSL *ssl;

            ctx = SSL_CTX_new(TLS_client_method());
            if(ctx == NULL)
            {
                failcounter++;
                node->rawSitePing[loopcounter] = 0;
                writeLog(LOG_TYPE_GPING, "OpenSSL: " + std::string(ERR_error_string(ERR_get_error(), NULL)));
            }
            else
            {
                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, sHost);

                if(SSL_connect(ssl) != 1)
                {
                    failcounter++;
                    node->rawSitePing[loopcounter] = 0;
                    writeLog(LOG_TYPE_GPING, "ERROR: Connect to " + host + ":" + to_string(port) + " through SOCKS5 server failed.");
                }
                else
                {
                    SSL_write(ssl, request.data(), request.size());
                    cur_len = SSL_read(ssl, bufRecv, BUF_SIZE - 1);
                    auto end = steady_clock::now();
                    auto duration = duration_cast<milliseconds>(end - start);
                    int deltatime = duration.count();
                    if(cur_len <= 0)
                    {
                        failcounter++;
                        node->rawSitePing[loopcounter] = 0;
                        writeLog(LOG_TYPE_GPING, "Accessing '" + target + "' - Fail - time=" + to_string(deltatime) + "ms");
                    }
                    else
                    {
                        succeedcounter++;
                        node->rawSitePing[loopcounter] = deltatime;
                        totduration += deltatime;
                        writeLog(LOG_TYPE_GPING, "Accessing '" + target + "' - Success - time=" + to_string(deltatime) + "ms");
                    }
                }
                SSL_clear(ssl);
            }
        }
        else
        {
            retVal = Send(sHost, request.data(), request.size(), 0);
            if (SOCKET_ERROR == retVal)
            {
                closesocket(sHost);
                return -1;
            }
            cur_len = Recv(sHost, bufRecv, BUF_SIZE - 1, 0);
            auto end = steady_clock::now();
            auto duration = duration_cast<milliseconds>(end - start);
            int deltatime = duration.count();
            if(cur_len <= 0)
            {
                failcounter++;
                node->rawSitePing[loopcounter] = 0;
                writeLog(LOG_TYPE_GPING, "Accessing '" + target + "' - Fail - time=" + to_string(deltatime) + "ms");
            }
            else
            {
                succeedcounter++;
                node->rawSitePing[loopcounter] = deltatime;
                totduration += deltatime;
                writeLog(LOG_TYPE_GPING, "Accessing '" + target + "' - Success - time=" + to_string(deltatime) + "ms");

            }
        }
end:
        draw_progress_gping(loopcounter, node->rawSitePing);
        loopcounter++;
        closesocket(sHost);
    }
    std::cerr<<std::endl;
    float pingval = 0.0;
    if(succeedcounter > 0)
        pingval = totduration * 1.0 / succeedcounter;
    char strtmp[16] = {};
    snprintf(strtmp, sizeof(strtmp), "%0.2f", pingval);
    node->sitePing.assign(strtmp);
    writeLog(LOG_TYPE_GPING, "Ping statistics of target " + target + " : " \
             + to_string(loopcounter) + " probes sent, " + to_string(succeedcounter) + " successful, " + to_string(failcounter) + " failed. ");
    writeLog(LOG_TYPE_GPING, "Website ping completed. Leaving.");
    return SPEEDTEST_MESSAGE_GOTGPING;
}
