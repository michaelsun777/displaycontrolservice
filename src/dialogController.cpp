
#include "dialogController.h"
#include "cdataProcess.h"

DialogController::DialogController()
{

}

void DialogController::service(RequestHandler* pRequestHandler,HttpRequest& request, HttpResponse& response)
{
    QByteArray path = request.getPath();
    XINFO("Conroller: path={}", path.data());   

    response.setHeader("Content-Type", "application/json");

    if (path.startsWith("/displayctrlserver/dialog/add"))
    {
        QByteArray barray = request.getBody();
        std::string body = barray.data();
        // auto body = req.body;
        XINFO("received msg:{}", body);
        json jdata = json::parse(body);

        string name = jdata["name"].template get<std::string>();
        string url = jdata["url"].template get<std::string>();
        string path = jdata["path"].template get<std::string>();
        string param = jdata["param"].template get<std::string>();
        
        int type = jdata["type"].template get<int>();
        int posType = jdata["posType"].template get<int>();
        int xVirtual = jdata["xVirtual"].template get<int>();
        int yVirtual = jdata["yVirtual"].template get<int>();
        int xPos = jdata["xPos"].template get<int>();
        int yPos = jdata["yPos"].template get<int>();
        int height = jdata["height"].template get<int>();
        int width = jdata["width"].template get<int>();
        QtDlgInfo dlg;
        dlg.name = name;
        dlg.url = url;
        dlg.path = path;
        dlg.param = param;
        dlg.type = type;
        dlg.postype = posType;
        dlg.xVirtual = xVirtual;
        dlg.yVirtual = yVirtual;
        dlg.xPos = xPos;
        dlg.yPos = yPos;
        dlg.height = height;
        dlg.width = width;
        
        // QtDlgInfo dlg;
        // dlg.height = 1080;
        // dlg.width = 2500;
        // dlg.postype = 1;
        // dlg.name = "test";
        // dlg.xPos = 100;
        // dlg.yPos = 50;
        // dlg.url = "https://www.baidu.com";

        pRequestHandler->sendSignal(1,dlg);
        
    }
    else if(path.startsWith("/displayctrlserver/dialog/getall"))
    {
        ;

    }
    else if(path.startsWith("/displayctrlserver/dialog/search"))
    {

    }
    else if(path.startsWith("/displayctrlserver/dialog/modify"))
    {
        
    }
    else if(path.startsWith("/displayctrlserver/dialog/del"))
    {
        
    }
    else
    {

    }


}