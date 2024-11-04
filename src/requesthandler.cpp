/**
  @file
  @author Stefan Frings
*/

//#include <logging/filelogger.h>

#include "requesthandler.h"
#include "cdataProcess.h"

#include "dialogController.h"

RequestHandler::RequestHandler(MainWindow * pMain,QObject* parent)
    :HttpRequestHandler(parent)
{
    XINFO("RequestHandler: created");
    m_pMain = pMain;
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


RequestHandler::~RequestHandler()
{
    XINFO("RequestHandler: deleted");
}

void RequestHandler::sendSignal(int type,QtDlgInfo & dlgInfo)
{
    QVariant dataInfo;
    dataInfo.setValue(dlgInfo);
    emit sendDlgSignal(type,dataInfo);
}


void RequestHandler::service(HttpRequest& request, HttpResponse& response)
{
    QByteArray path = request.getPath();
    XINFO("Conroller: path={}", path.data());
    

    response.setHeader("Content-Type", "application/json");

    if (path.startsWith("/displayctrlserver/get/monitor/info"))
    {
        getMonitorInfo(request, response);
    }
    else if (path.startsWith("/displayctrlserver/get/gpu/info"))
    {
        getGpusInfo(request, response);
    }
    else if (path.startsWith("/displayctrlserver/set/monitor/info"))
    {
        setMonitorInfo(request, response);
    }
    else if(path.startsWith("/displayctrlserver/dialog"))
    {        
        DialogController().service(this,m_pMain,request, response);
    }
    else
    {
        ;
    }

    //emit testSignal(3);   
   

    XINFO("RequestHandler: finished request");
}


void RequestHandler::createRet(HttpResponse &res,int code,nlohmann::json & data)
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

void RequestHandler::createRet(HttpResponse &res,int code)
{
    nlohmann::json ret;
    ret["code"] = code;
   
    ret["data"] = nlohmann::json::object();
    

    ret["msg"] = getRetMessage(code);
    std::string msg = ret.dump();
    //res.set_content(msg,"application/json");
    res.write(msg.c_str(),true);
}

std::string RequestHandler::getRetMessage(int code)
{
    auto it = m_mCodeMsg.find(code);
    if(it != m_mCodeMsg.end())
    {
        return it->second;
    }

    return "未定义错误码";
}

void RequestHandler::getMonitorInfo(const HttpRequest &req, HttpResponse& res)
{
   
    
    string strData;
    cdataProcess dataprocess;
    json js;
    if(dataprocess.GetMonitorsInfo_shell(js))
    {
        createRet(res,200,js);

        //res.set_content(strData, "application/json");
    }
    else
    {
        // string strRet = "{\"result\":\"error\",\"msg\":\"test error\"}";
        // res.set_content(strRet, "application/json");
        createRet(res,204);
    }   

}

void RequestHandler::getGpusInfo(const HttpRequest &req, HttpResponse& res)
{
    
    
    string strData;
    cdataProcess dataprocess;
  
    json js;
    if(dataprocess.GetGpuInfo(js))
    {
        createRet(res,200,js);
        // string strRet = "{\"result\":\"\",\"msg\":\"\"";
        // res.set_content(strData, "application/json");
        // res.status = 200;
    }
    else
    {
        createRet(res,204);
        // string strRet = "{\"result\":\"error\",\"msg\":\"test error\"}";
        // res.set_content(strRet, "application/json");
        // res.status = 204;
    }
    
}


void RequestHandler::setMonitorInfo(const HttpRequest &req, HttpResponse& res)
{
    QByteArray barray = req.getBody();
    std::string body = barray.data();


    //auto body = req.body;
    XINFO("received msg:{}",body);
    json jdata = json::parse(body);
    int num = jdata.at("num").get<int>();
    //std::cout <<"num:"<<num<<std::endl;

    vector<MONITORSETTINGINFO> vSetInfo;
    json jarry = jdata["data"];

    for (json::iterator it = jarry.begin();it!=jarry.end();it++)
    {
        MONITORSETTINGINFO info;
        //std::cout << (*it)["name"].template get<std::string>()<<std::endl;
        info.name = (*it)["name"].template get<std::string>();
        info.outputId = (*it)["rroutputId"].template get<int>();
        info.modeId = (*it)["modeId"].template get<int>();
        // info.pos.xPos = (*it)["xVirtual"].template get<int>();
        // info.pos.yPos = (*it)["yVirtual"].template get<int>();
        info.pos.xPos = (*it)["xPos"].template get<int>();
        info.pos.yPos = (*it)["yPos"].template get<int>();
        info.size.width = (*it)["width"].template get<int>();
        info.size.height = (*it)["height"].template get<int>();
        info.primary = (*it)["primary"].template get<Bool>();
        info.rotation = (*it)["rotation"].template get<int>();
        vSetInfo.push_back(info);  
    }
    
    if(num > 0)
    {
        bool bRet = false;
        string strErrorMsg;
        try
        {
            cdataProcess dataprocess;
            bRet = dataprocess.SetMonitorsInfo(&vSetInfo);
        }
        catch(...)
        {
            char czbuf[50] ={0};
            int nerrno = errno;
            sprintf(czbuf,"%d",nerrno);
            const char* error_msg = strerror(nerrno);
            strErrorMsg = czbuf;
            strErrorMsg += error_msg;
            XERROR("HttpManager SetMonitorsInfo error code:{},error msg:{}",nerrno,error_msg);
            createRet(res,500);
            return;
        }
        
        
        if(bRet)
        {
            // string strRet = "{\"result\":\"OK\",\"msg\":\"\"}";
            // res.set_content(strRet, "application/json");
            createRet(res,200);
        }
        else
        {
            // string strRet = "{\"result\":\"failed\",\"msg\":\"";
            // strRet+= strErrorMsg;
            // strRet+= "\"}";            
            // res.set_content(strRet, "application/json");
            createRet(res,500);
            return;            
        }
        
        
    }
    else
    {
        // string strRet = "{\"result\":\"error\",\"msg\":\"test error\"}";
        // res.set_content(strRet, "application/json");
        createRet(res,400);
    }

}

