
#ifndef _DIALOG_CONTROLLER_H_
#define _DIALOG_CONTROLLER_H_

#include "3rd/httpserver/httprequest.h"
#include "3rd/httpserver/httpresponse.h"
#include "3rd/httpserver/httprequesthandler.h"
#include "../3rd/json/include/nlohmann/json.hpp"
#include "requesthandler.h"
#include "../qtcommon.h"
#include "UI/mainwindow.h"




using namespace stefanfrings;

class DialogController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(DialogController)
private:
    std::map<int, std::string> m_mCodeMsg;
public:
    DialogController(/* args */);
    void service(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response);
    //void service(HttpRequest &request, HttpResponse &response);
    void createRet(HttpResponse &res, int code, nlohmann::json &data);
    void createRet(HttpResponse &res, int code);
    std::string getRetMessage(int code);
    int dlgAdd(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response);
    int dlgDel(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response);
    int dlgModify(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response);
    int dlgSearch(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response);
    int dlgGetAll(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response);
};

#endif