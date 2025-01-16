
#include "dialogController.h"
#include "cdataProcess.h"

DialogController::DialogController()
{
    XINFO("DialogController: created");
    m_mCodeMsg[200] = "success";//请求成功,资源已成功返回
    m_mCodeMsg[201] = "Created";//请求已成功,且服务器创建了新的资源
    m_mCodeMsg[202] = "Accepted";//请求已接受，但处理尚未完成
    m_mCodeMsg[203] = "Non-Authoritative Information";//返回的信息可能不完整
    m_mCodeMsg[204] = "No Content";//请求成功，但响应中没有内容
    m_mCodeMsg[206] = "Partial Content";//返回部分资源内容
    m_mCodeMsg[400] = "Bad Request";//请求有语法错误
    m_mCodeMsg[401] = "Unauthorized";//未授权访问
    m_mCodeMsg[403] = "Forbidden";//禁止访问
    m_mCodeMsg[404] = "Not Found";//资源不存在
    m_mCodeMsg[405] = "Method Not Allowed";//不允许使用的方法
    m_mCodeMsg[500] = "Internal Server Error";//服务器内部错误
    m_mCodeMsg[503] = "Service Unavailable";//服务器暂时不可用‌

}


void DialogController::createRet(HttpResponse &res,int code,nlohmann::json & data)
{
    nlohmann::json ret;
    ret["code"] = code;
    if(data != nullptr)
    {
        ret["data"] = data;
    }
    else
    {
        ret["data"] = nlohmann::json::object();
    }

    ret["msg"] = getRetMessage(code);
    std::string msg = ret.dump();
    //res.set_content(msg,"application/json");
    res.write(msg.c_str(),true);
}

void DialogController::createRet(HttpResponse &res,int code)
{
    nlohmann::json ret;
    ret["code"] = code;
   
    ret["data"] = nlohmann::json::object();
    

    ret["msg"] = getRetMessage(code);
    std::string msg = ret.dump();
    //res.set_content(msg,"application/json");
    res.write(msg.c_str(),true);
}

std::string DialogController::getRetMessage(int code)
{
    auto it = m_mCodeMsg.find(code);
    if(it != m_mCodeMsg.end())
    {
        return it->second;
    }

    return "未定义错误码";
}


void DialogController::service(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response)
{
    QByteArray path = request.getPath();
    XINFO("Conroller: path={}", path.data());
    response.setHeader("Content-Type", "application/json");
    int nRet = 0;

    if (path.startsWith("/displayctrlserver/dialog/add"))
    {
        nRet = dlgAdd(pRequestHandler, pMain, request, response);
    }
    else if(path.startsWith("/displayctrlserver/dialog/getall"))
    {        
        nRet = dlgGetAll(pRequestHandler,pMain, request, response);
    }
    else if(path.startsWith("/displayctrlserver/dialog/search"))
    {
        nRet = dlgSearch(pRequestHandler,pMain, request, response);
    }
    else if(path.startsWith("/displayctrlserver/dialog/modify"))
    {
        nRet = dlgModify(pRequestHandler,pMain, request, response);
    }
    else if(path.startsWith("/displayctrlserver/dialog/del"))
    {
        nRet = dlgDel(pRequestHandler, pMain, request, response);        
    }
    else
    {
        ;
    }

    if(nRet < 0)
    {
        ;
    }


}

int DialogController::dlgAdd(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response)
{
    try
    {
        cdataProcess* pcdataProcess = cdataProcess::GetInstance();
        std::string resolution = pcdataProcess->GetAllResolution();
        int allWidth = std::stoi(resolution.substr(0, resolution.find("x")));
        int allHeight = std::stoi(resolution.substr(resolution.find("x") + 1));

        cmyxrandr* pcmyXrandr = cmyxrandr::GetInstance();
        CMYSIZE currentSize = pcmyXrandr->getScreenSize();

        QByteArray barray = request.getBody();
        std::string body = barray.data();
        // auto body = req.body;
        XINFO("received msg:{}", body);
        json jdata = json::parse(body);

        string name = "";
        string path = "";
        string param = "";
        string url = "";
        int type = 0;
        int posType = 0;
        int xVirtual = 0;
        int yVirtual = 0;
        int xPos = 0;
        int yPos = 0;
        int height = 0;
        int width = 0;
        int order = 0;

        if(jdata.find("name") != jdata.end())
        {
            name = jdata["name"].template get<std::string>();
        }
        
        if(jdata.find("url") != jdata.end())
        {
            url = jdata["url"].template get<std::string>();
        }
        if(jdata.find("path") != jdata.end())
        {
            path = jdata["path"].template get<std::string>();
        }

        if(jdata.find("param") != jdata.end())
        {
            param = jdata["param"].template get<std::string>();
        }

        if(jdata.find("type") != jdata.end())
        {
            type = jdata["type"].template get<int>();
        }
      
        if(jdata.find("posType") != jdata.end())
        {
            posType = jdata["posType"].template get<int>();
        }

        if(jdata.find("xVirtual") != jdata.end())
        {
            xVirtual = (jdata["xVirtual"].template get<float>())*allWidth;
        }
        
        if(jdata.find("yVirtual") != jdata.end())
        {
            yVirtual = (jdata["yVirtual"].template get<float>())*allHeight;
        }
        
        if(jdata.find("xPos") != jdata.end())
        {
            xPos = (jdata["xPos"].template get<float>())*allWidth;
            if(xPos >= currentSize.width)
            {
                json js;
                js["error"] = "The x-coordinate exceeds the actual width!";
                createRet(response, 400, js);
                return -1;
            }
        }
        else
        {
            json js;
            js["error"] = "xPos null or error!";
            createRet(response, 400, js);
            return -1;
        }
        
        if(jdata.find("yPos") != jdata.end())
        {
            yPos = (jdata["yPos"].template get<float>())*allHeight;
            if(yPos >= currentSize.height)
            {
                json js;
                js["error"] = "The y-coordinate exceeds the actual height!";
                createRet(response, 400, js);
                return -1;
            }
        }
        else
        {
            json js;
            js["error"] = "yPos null or error!";
            createRet(response, 400, js);
            return -1;
        }

        if(jdata.find("height") != jdata.end())
        {
            height = (jdata["height"].template get<float>())*allHeight;
            if(yPos + height > currentSize.height)
            {
                height = currentSize.height - yPos;
            }
            if(height <= 0)
            {
                json js;
                js["error"] = "The height exceeds the actual height!";
                createRet(response, 400, js);
                return -1;
            }
        }
        else
        {
            json js;
            js["error"] = "height null or error!";
            createRet(response, 400, js);
            return -1;
        }

        if(jdata.find("width") != jdata.end())
        {
            width = (jdata["width"].template get<float>())*allWidth;
            if (xPos + width > currentSize.width)
            {
                width = currentSize.width - xPos;
            }
            if (width <= 0)
            {
                json js;
                js["error"] = "The width exceeds the actual width!";
                createRet(response, 400, js);
                return -1;
            }
        }
        else
        {
            json js;
            js["error"] = "width null or error!";
            createRet(response, 400, js);
            return -1;
        }

        if(jdata.find("order") != jdata.end())
        {
            order = jdata["order"].template get<int>();
        }
        else
        {
            json js;
            js["error"] = "order null!";
            createRet(response, 400, js);
            return -1;
        }

        if (!pMain->checkOrder(order))
        {
            json js;
            js["error"] = "order duplication!";
            createRet(response, 400, js);
            return -1;
        }

        QtDlgInfo dlg;
        dlg.name = name;
        dlg.url = url;
        dlg.path = path;
        dlg.param = param;
        dlg.type = type;
        dlg.posType = posType;
        dlg.xVirtual = xVirtual;
        dlg.yVirtual = yVirtual;
        dlg.xPos = xPos;
        dlg.yPos = yPos;
        dlg.height = height;
        dlg.width = width;
        dlg.order = order;
        
        // QtDlgInfo dlg;
        // dlg.height = 1080;
        // dlg.width = 2500;
        // dlg.postype = 1;
        // dlg.name = "test";
        // dlg.xPos = 100;
        // dlg.yPos = 50;
        // dlg.url = "https://www.baidu.com";
        if (pMain->addDlg(dlg))
        {
            pRequestHandler->sendSignal(1, dlg);
            createRet(response,200);
        }
        else
        {
            ;
        }        
        
    }
    catch(...)
    {
        return -1;
    }    

    return 0;
}

int DialogController::dlgModify(RequestHandler *pRequestHandler,MainWindow * pMain, HttpRequest &request, HttpResponse &response)
{
    QByteArray barray = request.getBody();
    std::string body = barray.data();
    // auto body = req.body;
    XINFO("received msg:{}", body);
    json jdata = json::parse(body);

    cdataProcess* pcdataProcess = cdataProcess::GetInstance();
    std::string resolution = pcdataProcess->GetAllResolution();
    int allWidth = std::stoi(resolution.substr(0, resolution.find("x")));
    int allHeight = std::stoi(resolution.substr(resolution.find("x") + 1));

    cmyxrandr* pcmyxrandr = cmyxrandr::GetInstance();
    CMYSIZE currentSize = pcmyxrandr->getScreenSize();

    string dlgid = "";
    string name = "";
    string path = "";
    string param = "";
    string url = "";
    int type = 0;
    int posType = 0;
    int xVirtual = 0;
    int yVirtual = 0;
    int xPos = 0;
    int yPos = 0;
    int height = 0;
    int width = 0;

    if (jdata.find("dlgId") != jdata.end())
    {
        dlgid = jdata["dlgId"].template get<std::string>();
    }
    else
    {
        json js;
        js["error"] = "lost dlgid!";
        createRet(response, 400, js);
        return -1;
    }

    if (jdata.find("name") != jdata.end())
    {
        name = jdata["name"].template get<std::string>();
    }

    if (jdata.find("url") != jdata.end())
    {
        url = jdata["url"].template get<std::string>();
    }
    if (jdata.find("path") != jdata.end())
    {
        path = jdata["path"].template get<std::string>();
    }

    if (jdata.find("param") != jdata.end())
    {
        param = jdata["param"].template get<std::string>();
    }

    if (jdata.find("type") != jdata.end())
    {
        type = jdata["type"].template get<int>();
    }

    if (jdata.find("posType") != jdata.end())
    {
        posType = jdata["posType"].template get<int>();
    }

    if (jdata.find("xVirtual") != jdata.end())
    {
        xVirtual = (jdata["xVirtual"].template get<float>())*allWidth;
    }

    if (jdata.find("yVirtual") != jdata.end())
    {
        yVirtual = (jdata["yVirtual"].template get<float>())*allHeight;
    }

    if (jdata.find("xPos") != jdata.end())
    {
        xPos = (jdata["xPos"].template get<float>())*allWidth;
        if (xPos >= currentSize.width)
        {
            json js;
            js["error"] = "The x-coordinate exceeds the actual width!";
            createRet(response, 400, js);
            return -1;
        }
    }
    else
    {
        json js;
        js["error"] = "xPos null or error!";
        createRet(response, 400, js);
        return -1;
    }

    if (jdata.find("yPos") != jdata.end())
    {
        yPos = (jdata["yPos"].template get<float>())*allHeight;
        if (yPos >= currentSize.height)
        {
            json js;
            js["error"] = "The y-coordinate exceeds the actual height!";
            createRet(response, 400, js);
            return -1;
        }
    }
    else
    {
        json js;
        js["error"] = "yPos null or error!";
        createRet(response, 400, js);
        return -1;
    }
    if (jdata.find("height") != jdata.end())
    {
        height = (jdata["height"].template get<float>())*allHeight;
        if (yPos + height > currentSize.height)
        {
            height = currentSize.height - yPos;
        }
        if (height <= 0)
        {
            json js;
            js["error"] = "The height exceeds the actual height!";
            createRet(response, 400, js);
            return -1;
        }
    }
    else
    {
        json js;
        js["error"] = "height null or error!";
        createRet(response, 400, js);
        return -1;
    }
    if (jdata.find("width") != jdata.end())
    {
        width = (jdata["width"].template get<float>())*allWidth;
        if (xPos + width > currentSize.width)
        {
            width = currentSize.width - xPos;
        }
        if (width <= 0)
        {
            json js;
            js["error"] = "The width exceeds the actual width!";
            createRet(response, 400, js);
            return -1;
        }
    }
    else
    {
        json js;
        js["error"] = "width null or error!";
        createRet(response, 400, js);
        return -1;
    }

    QtDlgInfo dlg;
    if(jdata.find("order") != jdata.end())
    {
        dlg.order = jdata["order"].template get<int>();
    }
    else
    {
        json js;
        js["error"] = "order null!";
        createRet(response, 400, js);
        return -1;
    }

    if (!pMain->checkOrder(dlg.order, dlgid))
    {
        json js;
        js["error"] = "order duplication!";
        createRet(response, 400, js);
        return -1;
    }

    dlg.dlgId = dlgid;
    dlg.name = name;
    dlg.url = url;
    dlg.path = path;
    dlg.param = param;
    dlg.type = type;
    dlg.posType = posType;
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
    if (pMain->modifyDlg(dlg))
    {
        pRequestHandler->sendSignal(3, dlg);
        json js;
        dlg.toJson(js);
        createRet(response, 200,js);
    }    
    return 0;
}
int DialogController::dlgSearch(RequestHandler *pRequestHandler,MainWindow * pMain, HttpRequest &request, HttpResponse &response)
{
    QByteArray barray = request.getBody();
    std::string body = barray.data();
    // auto body = req.body;
    XINFO("received msg:{}", body);
    json jdata = json::parse(body);

    string dlgid = "";
    if (jdata.find("dlgId") != jdata.end())
    {
        dlgid = jdata["dlgId"].template get<std::string>();
        QtDlgInfo info;
        if(pMain->getDlgInfo(dlgid,info))
        {
            cdataProcess* pcdataProcess = cdataProcess::GetInstance();
            std::string resolution = pcdataProcess->GetAllResolution();
            int allWidth = std::stoi(resolution.substr(0, resolution.find("x")));
            int allHeight = std::stoi(resolution.substr(resolution.find("x") + 1));
            cmyxrandr* pcmyxrandr = cmyxrandr::GetInstance();
            CMYSIZE currentSize = pcmyxrandr->getScreenSize();
            nlohmann::json js;
            info.toJson(js);
            js["xVirtual"] = ((float)info.xVirtual)/allWidth;
            js["yVirtual"] = ((float)info.yVirtual)/allHeight;
            js["xPos"] = ((float)info.xPos)/allWidth;
            js["yPos"] = ((float)info.yPos)/allHeight;
            js["width"] = ((float)info.width)/allWidth;
            js["height"] = ((float)info.height)/allHeight;
            createRet(response,200,js);
        }
        else
        {
            json js;
            js["error"] = "not find!";
            createRet(response,404,js);
        }

    }
    else
    {
        json js;
        js["error"] = "lost dlgid!";
        createRet(response, 400, js);
        return -1;
    }
    return 0;
}

int DialogController::dlgGetAll(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response)
{
    XINFO("received msg:{dlgGetAll}");
    cdataProcess* pcdataProcess = cdataProcess::GetInstance();
    std::string resolution = pcdataProcess->GetAllResolution();
    int allWidth = std::stoi(resolution.substr(0, resolution.find("x")));
    int allHeight = std::stoi(resolution.substr(resolution.find("x") + 1));

    std::vector<QtDlgInfo> vInfo;
    if (pMain->getAllDlgInfo(vInfo))
    {
        nlohmann::json js;
        if(vInfo.size() > 0)
        {
            json dataArray;
            for (size_t i = 0; i < vInfo.size(); i++)
            {
                json data;
                vInfo[i].toJson(data);
                data["xVirtual"] = ((float)vInfo[i].xVirtual)/allWidth;
                data["yVirtual"] = ((float)vInfo[i].yVirtual)/allHeight;
                data["xPos"] = ((float)vInfo[i].xPos)/allWidth;
                data["yPos"] = ((float)vInfo[i].yPos)/allHeight;
                data["width"] = ((float)vInfo[i].width)/allWidth;
                data["height"] = ((float)vInfo[i].height)/allHeight;
                dataArray.push_back(data);               
            }
            js["dlgInfos"] = dataArray;
            js["num"] = vInfo.size();            
        }                
        createRet(response, 200, js);
    }
    else
    {
        json js;
        js["error"] = "not find!";
        createRet(response, 404, js);
    }

    return 0;
}

int DialogController::dlgDel(RequestHandler* pRequestHandler,MainWindow * pMain,HttpRequest& request, HttpResponse& response)
{
    QByteArray barray = request.getBody();
    std::string body = barray.data();
    // auto body = req.body;
    XINFO("received msg:{}", body);
    json jdata = json::parse(body);

    string dlgid = "";
    if (jdata.find("dlgId") != jdata.end())
    {
        dlgid = jdata["dlgId"].template get<std::string>();
        QtDlgInfo info;
        info.dlgId = dlgid;
        if(pMain->delDlg(info))
        {
            pRequestHandler->sendSignal(2, info);
            createRet(response, 200);
            return 0;
        }
    }
    else
    {
        json js;
        js["error"] = "lost dlgid!";
        createRet(response, 400, js);
        return -1;
    }
    return 0;
}