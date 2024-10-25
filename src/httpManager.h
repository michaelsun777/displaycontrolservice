#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H


#include <iostream>
#include "3rd/cpp-httplib/benchmark/cpp-httplib-base/httplib.h"
#include <functional>
#include "cspdlog.h"
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread.hpp>
#include <errno.h>

using namespace httplib;
using namespace std;

static mutex g_RecHttpManagerMutex;

class HttpManager
{
private:
    int m_nPort;
    string m_Ip;
    Server m_svr;  
    static std::shared_ptr<HttpManager> m_pHttpManager;
private:
    HttpManager(/* args */);
public:
    static shared_ptr<HttpManager> GetInstance();
    
    ~HttpManager();
    bool start();
    bool close();
    std::string dump_headers(const Headers &headers);
    void getMonitorInfo(const Request &req, Response &res);
    void setMonitorInfo(const Request &req, Response &res);
    void getTestMonitorInfo(const Request &req, Response &res);

public:
    
};





#endif