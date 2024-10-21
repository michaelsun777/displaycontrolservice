#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H


#include <iostream>
#include "3rd/cpp-httplib/benchmark/cpp-httplib-base/httplib.h"
#include <functional>
#include "cspdlog.h"
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread.hpp>

using namespace httplib;
using namespace std;

static mutex g_RecHttpManagerMutex;

class HttpManager
{
private:
    int m_nPort;
    Server m_svr;
    int m_nTimes;
    int m_nUniformity_w;
    int m_nUniformity_h;
    int m_nSum_w;
    int m_nSum_h;
    static std::shared_ptr<HttpManager> m_pHttpManager;
private:
    HttpManager(/* args */);
public:
    static shared_ptr<HttpManager> GetInstance();
    
    ~HttpManager();
    bool start();
    bool close();
    std::string dump_headers(const Headers &headers);
    void msgParse(const Request &req, Response &res);

public:
    
};





#endif