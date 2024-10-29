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
#include "3rd/json/include/nlohmann/json.hpp"
#include <map>

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
    std::map<int,string> m_mCodeMsg;

private:
    HttpManager(/* args */);
    void createRet(Response &res,int code,nlohmann::json & data);
    void createRet(Response &res,int code);
    string getRetMessage(int code);
public:
    static shared_ptr<HttpManager> GetInstance();
    
    ~HttpManager();
    bool start();
    bool close();
    std::string dump_headers(const Headers &headers);
    void getMonitorInfo(const Request &req, Response &res);
    void getGpusInfo(const Request &req, Response &res);
    void setMonitorInfo(const Request &req, Response &res);
    void getTestMonitorInfo(const Request &req, Response &res);
public:
    void addDlg(const Request &req, Response &res);
    void delDlg(const Request &req, Response &res);
    void modifyDlg(const Request &req, Response &res);
    void searchDlg(const Request &req, Response &res);
    void getAllDlg(const Request &req, Response &res);


public:
    void handleUserLogin(const Request &req, Response &res);
};





#endif