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
    try
    {
        QByteArray path = request.getPath();
        XINFO("Conroller: path={}", path.data());

        response.setHeader("Content-Type", "application/json");

        if (path.startsWith("/displayctrlserver/get/monitor/info"))
        {
            getMonitorInfo(request, response);
        }
        else if (path.startsWith("/displayctrlserver/get/outputs/resolution"))
        {
            getOutputsMode(request, response);
        }
        else if (path.startsWith("/displayctrlserver/get/outputs/info"))
        {
            getOutputsInfo(request, response);
        }
        else if (path.startsWith("/displayctrlserver/get/gpu/info"))
        {
            getGpusInfo(request, response);
        }
        else if (path.startsWith("/displayctrlserver/get/outputs/reset"))
        {
            resetOutputsInfo(request, response);
        }
        else if (path.startsWith("/displayctrlserver/get/gpu/Interface"))
        {
            getGpuInterface(request, response);
        }
        else if (path.startsWith("/displayctrlserver/set/monitor/info"))
        {
            setMonitorInfo(request, response);
        }
        else if (path.startsWith("/displayctrlserver/set/outputs/info"))
        {
            setOutputsInfo(request, response);
        }
        else if (path.startsWith("/displayctrlserver/dialog"))
        {
            DialogController().service(this, m_pMain, request, response);
        }
        else if (path.startsWith("/displayctrlserver/get/server/info"))
        {
            getServerInfo(request, response);
        }
        else if (path.startsWith("/displayctrlserver/login"))
        {
            login(request, response);
        }
        else
        {
            createRet(response, 404);
        }
    }
    catch(...)
    {
        //m_mutex.unlock();
        XINFO("{RequestHandler::service catch unlock}\n");        
    }
    
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

void RequestHandler::login(const HttpRequest &req, HttpResponse &res)
{
    QByteArray barray = req.getBody();
    std::string body = barray.data();
    XINFO("received msg:{}",body);
    try
    {
        json jdata = json::parse(body);
        string strUserName = jdata.at("username").get<string>();
        string strPwd = jdata.at("password").get<string>();
        XINFO("{RequestHandler::login lock}\n");
        //m_mutex.lock();
        boost::lock_guard<boost::mutex> lock(m_mutex);
        QSettings config("user.db", QSettings::IniFormat);
        config.value("user/name", "admin");
        string localPwd = config.value("user/pwd", "admin").toString().toStdString();
        MD5 md5(localPwd);
        string loacalPwdMd5 = md5.toStr();
        //m_mutex.unlock();
        XINFO("{RequestHandler::login unlock}\n");
        
        if(strUserName.compare("admin") == 0 && strPwd.compare(loacalPwdMd5) == 0)
        {
            createRet(res,200);
        }
        else
        {
            createRet(res,401);
        }
        
    }
    catch(...)
    {
        createRet(res,500);
        //m_mutex.unlock();
        XINFO("{RequestHandler::login cache unlock}\n");
    }

}


void RequestHandler::getMonitorInfo(const HttpRequest &req, HttpResponse& res)
{
    string strData;
    //cdataProcess dataprocess;
    cdataProcess* pcdataProcess = cdataProcess::GetInstance();
    json js;
    XINFO("{RequestHandler::getMonitorInfo lock}\n");
    //m_mutex.lock();
    boost::lock_guard<boost::mutex> lock(m_mutex);
    if(pcdataProcess->GetMonitorsInfo_shell(js))
    {
        //m_mutex.unlock();
        XINFO("{RequestHandler::getMonitorInfo unlock0}\n");
        return;
        createRet(res,200,js);

        //res.set_content(strData, "application/json");
    }
    else
    {
        // string strRet = "{\"result\":\"error\",\"msg\":\"test error\"}";
        // res.set_content(strRet, "application/json");
        createRet(res,204);
        //m_mutex.unlock();
        XINFO("{RequestHandler::getMonitorInfo unlock1}\n");
        return;
    }

}

void RequestHandler::getOutputsMode(const HttpRequest &req, HttpResponse& res)
{
    string strData;
    //cdataProcess dataprocess;
    json js;
    try
    {
        cdataProcess* pcdataProcess = cdataProcess::GetInstance();
        XINFO("{RequestHandler::getOutputsMode lock}\n");
        //m_mutex.lock();
        boost::lock_guard<boost::mutex> lock(m_mutex);
        XINFO("{RequestHandler::getOutputsMode lock in}\n");
        if(pcdataProcess->GetMainOutputModes(js))
        {
            createRet(res,200,js);
            //m_mutex.unlock();
            //XINFO("{RequestHandler::getOutputsMode unlock0}\n");
            return;
        }
        //m_mutex.unlock();
        XINFO("{RequestHandler::getOutputsMode unlock}\n");
    }
    catch(...)
    {
        createRet(res,500);
        //m_mutex.unlock();
        XINFO("{RequestHandler::getOutputsMode catch unlock}\n");
    }  


}

void RequestHandler::getOutputsInfo(const HttpRequest &req, HttpResponse &res)
{
    string strData;
    //cdataProcess dataprocess;
    json js;
    try
    {
        cdataProcess *pcdataProcess = cdataProcess::GetInstance();
        XINFO("{RequestHandler::getOutputsInfo lock}\n");
        //m_mutex.lock();
        boost::lock_guard<boost::mutex> lock(m_mutex);
        XINFO("{RequestHandler::getOutputsInfo lock in}\n");
        int nRet = pcdataProcess->GetOutputsInfo_shell(js);
        if (nRet == 0)
        {
            //m_mutex.unlock();
            //XINFO("{RequestHandler::getOutputsInfo unlock0}\n");
            createRet(res, 200, js);
            return;

            // res.set_content(strData, "application/json");
        }
        else if(nRet == -1)
        {
            //m_mutex.unlock();
            //XINFO("{RequestHandler::getOutputsInfo unlock1}\n");
            createRet(res, 500);
            return;
        }
        else
        {
            //m_mutex.unlock();
            //XINFO("{RequestHandler::getOutputsInfo unlock2}\n");
            createRet(res, 204);
            return;
        }
    }
    catch(...)
    {
        XERROR("RequestHandler::getOutputsInfo error errno = {}",errno);
        createRet(res, 500);
        //m_mutex.unlock();
        XINFO("{RequestHandler::getOutputsInfo catch unlock}\n");
    }

}

void RequestHandler::getGpusInfo(const HttpRequest &req, HttpResponse& res)
{
    string strData;
    json js;
    try
    {
        
        //cdataProcess dataprocess;
        cdataProcess *pcdataProcess = cdataProcess::GetInstance();
        XINFO("{RequestHandler::getGpusInfo lock}\n");
        //m_mutex.lock();
        boost::lock_guard<boost::mutex> lock(m_mutex);
        XINFO("{RequestHandler::getGpusInfo lock in}\n");
        if (pcdataProcess->GetGpuInfo(js))
        {
            //m_mutex.unlock();
            //XINFO("{RequestHandler::getGpusInfo unlock0}\n");
            createRet(res, 200, js);
            return;
        }
        else
        {
            createRet(res, 204);
        }
        //m_mutex.unlock();
        XINFO("{RequestHandler::getGpusInfo unlock}\n");
    }
    catch (...)
    {
        //m_mutex.unlock();
        XINFO("{RequestHandler::getGpusInfo catch unlock}\n");
        createRet(res, 500);
    }
}

void RequestHandler::resetOutputsInfo(const HttpRequest &req, HttpResponse& res)
{
    string strData;
    json js;
    try
    {
        //m_mutex.unlock();
        //m_mutex.unlock();
        XINFO("{RequestHandler::resetOutputsInfo unlock0}\n");
        QSettings settings("config.ini", QSettings::IniFormat);
        settings.beginGroup("outputsSettings");
        // settings.setValue("outputs", js.dump().c_str());
        settings.remove("outputs");
        settings.endGroup();
        settings.beginGroup("screen");
        settings.setValue("isSetting", "false");
        settings.setValue("height", "1080");
        settings.setValue("width", "1920");
        settings.setValue("layout_horizontal", "1");
        settings.setValue("layout_vertical", "1");
        settings.endGroup();
        settings.sync();

        //cdataProcess dataprocess;       
        cdataProcess *pcdataProcess = cdataProcess::GetInstance();
        //m_mutex.lock(); 
        XINFO("{RequestHandler::resetOutputsInfo lock}\n");
        boost::lock_guard<boost::mutex> lock(m_mutex);
        XINFO("{RequestHandler::resetOutputsInfo lock in}\n");
        if (pcdataProcess->InitOutputInfo())
        {
            pcdataProcess->ResetOutputsInfo();
            createRet(res, 200);
            return;
        }
        else
        {
            createRet(res, 204);
        }        
        //m_mutex.unlock();
        XINFO("{RequestHandler::resetOutputsInfo unlock}\n");
    }
    catch (...)
    {
        //m_mutex.unlock();
        XINFO("{RequestHandler::resetOutputsInfo catch unlock}\n");
        createRet(res, 500);        
    }

}

void RequestHandler::getGpuInterface(const HttpRequest &req, HttpResponse& res)
{
    string strData;
    json js;
    try
    {
        //cdataProcess dataprocess;  
        cdataProcess *pcdataProcess = cdataProcess::GetInstance();
        XINFO("{RequestHandler::getGpuInterface lock}\n");
        ////m_mutex.lock();
        boost::lock_guard<boost::mutex> lock(m_mutex);
        XINFO("{RequestHandler::getGpuInterface lock in}\n");
        if(pcdataProcess->GetOutputAndGpuName(js))
        {
            ////m_mutex.unlock();
            XINFO("{RequestHandler::getGpuInterface unlock0}\n");
            createRet(res,200,js);
            return;
        }
        else
        {
            createRet(res,204);
        }
        ////m_mutex.unlock();
        XINFO("{RequestHandler::getGpuInterface unlock}\n");
    }
    catch(...)
    {
        createRet(res,500);
        //m_mutex.unlock();
        XINFO("{RequestHandler::getGpuInterface unlock}\n");
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
            //cdataProcess dataprocess;
            cdataProcess *pcdataProcess = cdataProcess::GetInstance();
            bRet = pcdataProcess->SetMonitorsInfo(&vSetInfo);
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

void RequestHandler::setOutputsInfo(const HttpRequest &req, HttpResponse &res)
{
    try
    {
        
        QByteArray barray = req.getBody();
        std::string body = barray.data();
        json js = json::parse(body);
        XINFO("received msg:{}",body);
        {
            // if(js.find("layoutName") == js.end())
            // {
            //     XERROR("RequestHandler::setOutputsInfo: layoutName is not exist");
            //     createRet(res, 400);
            //     return;
            // }
                
            if(js.find("resolution") == js.end())
            {
                XERROR("RequestHandler::setOutputsInfo: resolution is not exist");
                createRet(res, 400);
                return;
            }
            if(js.find("allResolution") == js.end())
            {
                XERROR("RequestHandler::setOutputsInfo: allResolution is not exist");
                createRet(res, 400);
                return;
            }
            if(js.find("layout_horizontal") == js.end())
            {
                XERROR("RequestHandler::setOutputsInfo: layout_horizontal is not exist");
                createRet(res, 400);
                return;
            }
            if(js.find("layout_vertical") == js.end())
            {
                XERROR("RequestHandler::setOutputsInfo: layout_vertical is not exist");
                createRet(res, 400);
                return;
            }
            if(js.find("layout") == js.end())
            {
                XERROR("RequestHandler::setOutputsInfo: layout is not exist");
                createRet(res, 400);
                return;
            }
        }
        
        //cdataProcess dataprocess;
        cdataProcess *pcdataProcess = cdataProcess::GetInstance();
        XINFO("{RequestHandler::setOutputsInfo lock}\n");
        //m_mutex.lock();
        //bool bRet = pcdataProcess->SetOutputsInfo(js);
        boost::lock_guard<boost::mutex> lock(m_mutex);
        XINFO("{RequestHandler::setOutputsInfo lock in}\n");
        bool bRet = pcdataProcess->setOutputsXrandr(js);
        if (bRet)
        {
            //m_mutex.unlock();
            XINFO("{RequestHandler::setOutputsInfo unlock}\n");
            //string layoutName = js["layoutName"].get<std::string>();
            string resolution = js["resolution"].get<std::string>();
            string allResolution = js["allResolution"].get<std::string>();
            
            std::vector<std::string> vWidthAndHight = CMDEXEC::Split(resolution, 'x');
            int _width = std::stoi(vWidthAndHight[0]);
            int _hight = std::stoi(vWidthAndHight[1]);
            // std::vector<std::string> vLayout = CMDEXEC::Split(layoutName, 'x');
            // int _layout_w = std::stoi(vLayout[0]);
            // int _layout_h = std::stoi(vLayout[1]);
            int _layout_h = js["layout_horizontal"].get<int>();
            int _layout_w = js["layout_vertical"].get<int>();
 

            QSettings settings("config.ini", QSettings::IniFormat);
            settings.beginGroup("outputsSettings");
            settings.setValue("outputs", js.dump().c_str());
            settings.endGroup();
            settings.beginGroup("screen");
            settings.setValue("isSetting", "true");
            settings.setValue("width", _width);
            settings.setValue("height", _hight);
            settings.setValue("layout_horizontal", _layout_h);
            settings.setValue("layout_vertical", _layout_w);
            settings.setValue("allResolution", allResolution.c_str());
            settings.endGroup();
            settings.sync();
            
            createRet(res, 200);
            return;
            
        }
        else
        {
            //m_mutex.unlock();
            XINFO("{RequestHandler::setOutputsInfo unlock0}\n");
            createRet(res, 500);
        }

    }
    catch(...)
    {
        //m_mutex.unlock();
        XINFO("{RequestHandler::setOutputsInfo catch unlock0}\n");
        createRet(res,400);
    }  

}



void RequestHandler::getServerInfo(const HttpRequest &req, HttpResponse &res)
{
    string strData;
    //cdataProcess dataprocess;
    cdataProcess *pcdataProcess = cdataProcess::GetInstance();
    json js;
    XINFO("{RequestHandler::getServerInfo lock}\n");
    //m_mutex.lock();
    boost::lock_guard<boost::mutex> lock(m_mutex);
    XINFO("{RequestHandler::getServerInfo lock in}\n");
    if(pcdataProcess->GetServerInfo(js))
    {
        //m_mutex.unlock();
        XINFO("{RequestHandler::getServerInfo unlock}\n");
        createRet(res,200,js);
        return;
        
    }
    else
    {
        // string strRet = "{\"result\":\"error\",\"msg\":\"test error\"}";
        // res.set_content(strRet, "application/json");
        createRet(res,204);
    }
    //m_mutex.unlock();
    XINFO("{RequestHandler::getServerInfo unlock0}\n");
}

