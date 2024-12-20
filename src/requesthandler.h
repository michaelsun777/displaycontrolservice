#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "3rd/httpserver/httprequesthandler.h"
#include <QApplication>
#include <QVariant>
#include "../3rd/json/include/nlohmann/json.hpp"
#include <map>
#include "cspdlog.h"
#include <cstring>
#include "../qtcommon.h"
#include "UI/mainwindow.h"
#include "../3rd/md5/src/md5.h"
#include <mutex>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>


using namespace stefanfrings;
using namespace std;
/**
The request handler receives incoming HTTP requests and generates responses.
*/


class RequestHandler : public HttpRequestHandler
{
  Q_OBJECT
  Q_DISABLE_COPY(RequestHandler)
private:
  std::map<int, std::string> m_mCodeMsg;
  MainWindow * m_pMain;
  //std::mutex m_mutex;  
  boost::mutex m_mutex;

public:
  /**
    Constructor.
    @param parent Parent object
  */
  RequestHandler(MainWindow * pMain,QObject *parent = 0);

  /**
    Destructor
  */
  ~RequestHandler();

  /**
    Process an incoming HTTP request.
    @param request The received HTTP request
    @param response Must be used to return the response
  */
  void service(HttpRequest &request, HttpResponse &response);
  void createRet(HttpResponse &res,int code,nlohmann::json & data);
  void createRet(HttpResponse &res,int code);
  std::string getRetMessage(int code);

  void getMonitorInfo(const HttpRequest &req, HttpResponse &res);
  void getOutputsMode(const HttpRequest &req, HttpResponse &res);
  void getOutputsInfo(const HttpRequest &req, HttpResponse &res);
  void resetOutputsInfo(const HttpRequest &req, HttpResponse &res);
  void getGpusInfo(const HttpRequest &req, HttpResponse &res);
  void getGpuInterface(const HttpRequest &req, HttpResponse &res);
  void setMonitorInfo(const HttpRequest &req, HttpResponse &res);
  void setOutputsInfo(const HttpRequest &req, HttpResponse &res);
  void sendSignal(int type,QtDlgInfo & dlgInfo);
  void getServerInfo(const HttpRequest &req, HttpResponse &res);
  void login(const HttpRequest &req, HttpResponse &res);

signals:
  void sendDlgSignal(int type,QVariant dlgInfo);
};

#endif // REQUESTHANDLER_H
