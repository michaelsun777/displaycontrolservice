
#ifndef _DIALOG_CONTROLLER_H_
#define _DIALOG_CONTROLLER_H_

#include "3rd/httpserver/httprequest.h"
#include "3rd/httpserver/httpresponse.h"
#include "3rd/httpserver/httprequesthandler.h"
#include "../3rd/json/include/nlohmann/json.hpp"
#include "requesthandler.h"
#include "../qtcommon.h"




using namespace stefanfrings;

class DialogController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(DialogController)
private:
    /* data */
public:
    DialogController(/* args */);
    void service(RequestHandler* pRequestHandler,HttpRequest& request, HttpResponse& response);


};

#endif