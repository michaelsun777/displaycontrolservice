#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "def_common.h"
#include <QApplication>
#include <QVariant>
#include "3rd/httpserver/httprequesthandler.h"
#include "../3rd/json/include/nlohmann/json.hpp"
#include <map>
#include "cspdlog.h"
#include <cstring>
#include "../qtcommon.h"


#ifdef USE_CEF_SWITCH
#include "UI/mainwindow.h"
#endif // USE_CEF_SWITCH


#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

#include <atomic>

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
  boost::mutex m_mutex;
  std::atomic<int> m_nCounter;

public:
  /**
    Constructor.
    @param parent Parent object
  */
  RequestHandler(QObject *parent = 0);

#ifdef USE_CEF_SWITCH

  RequestHandler(MainWindow *pMain, QObject *parent = 0);
  void setTitleWindow(const HttpRequest &req, HttpResponse &res);
  void getTitleWindow(const HttpRequest &req, HttpResponse &res);

private:
  MainWindow *m_pMain;
#endif // USE_CEF_SWITCH

  /**
    Destructor
  */
  ~RequestHandler();

 
 public:
  void service(HttpRequest &request, HttpResponse &response);
  void createRet(HttpResponse &res, int code, nlohmann::json &data);
  void createRet(HttpResponse &res, int code);
  std::string getRetMessage(int code);

  void getMonitorInfo(const HttpRequest &req, HttpResponse &res);
  void getOutputsMode(const HttpRequest &req, HttpResponse &res);
  void getOutputsInfo(const HttpRequest &req, HttpResponse &res);
  void resetOutputsInfo(const HttpRequest &req, HttpResponse &res);
  void getGpusInfo(const HttpRequest &req, HttpResponse &res);
  void getGpuInterface(const HttpRequest &req, HttpResponse &res);
  void setGpuInterface(const HttpRequest &req, HttpResponse &res);
  void setMonitorInfo(const HttpRequest &req, HttpResponse &res);
  void setOutputsInfo(const HttpRequest &req, HttpResponse &res);
  void sendSignal(int type, QtDlgInfo &dlgInfo);
  void getServerInfo(const HttpRequest &req, HttpResponse &res);
  void login(const HttpRequest &req, HttpResponse &res);

signals:
  void sendDlgSignal(int type, QVariant dlgInfo);
  void sendOpenTitleWindowSignal();
  void sendCloseTitleWindowSignal();
  
};

#endif // REQUESTHANDLER_H
