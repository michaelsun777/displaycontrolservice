#include "httpManager.h"

#include "cdataProcess.h"


shared_ptr<HttpManager> HttpManager::m_pHttpManager = NULL;

std::shared_ptr<HttpManager> HttpManager::GetInstance()
{
    if (m_pHttpManager == NULL)
    {
        lock_guard<mutex> m_lock(g_RecHttpManagerMutex);
        if (m_pHttpManager == NULL)
        {
            static std::once_flag s_flag;
            call_once(s_flag,[&]()
            {
                m_pHttpManager.reset(new HttpManager);
            });

            m_pHttpManager->start();

            // if()
            // {
            //     return m_pHttpManager;
            // }
            // else
            // {
            //     return NULL;
            // }
            
        }
        return m_pHttpManager;


        // ImgRecManager * p = new ImgRecManager();
        // if(p->start())
        // {            
        //     m_pImgSendManager.reset(p);
        // }
        // else
        //     return NULL;
        
    }
    return m_pHttpManager;
}

HttpManager::HttpManager(/* args */)
{

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



    CConfig config;
    config.SetFilePath("config.ini");
    string value = "";
	string error = "";
    config.GetValue("common","ip",value,error);
    if(value.empty()) 
        m_Ip="127.0.0.1";
    else
        m_Ip = value.c_str();

    value.clear();
    config.GetValue("common","port",value,error);
    if(value.empty())
        m_nPort = 18180;
    else
        m_nPort = atoi(value.c_str());  

}

void HttpManager::createRet(Response &res,int code,nlohmann::json & data)
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
    res.set_content(msg,"application/json");
}

void HttpManager::createRet(Response &res,int code)
{
    nlohmann::json ret;
    ret["code"] = code;
   
    ret["data"] = nlohmann::json::object();
    

    ret["msg"] = getRetMessage(code);
    std::string msg = ret.dump();
    res.set_content(msg,"application/json");
}

string HttpManager::getRetMessage(int code)
{
    auto it = m_mCodeMsg.find(code);
    if(it != m_mCodeMsg.end())
    {
        return it->second;
    }

    return "未定义错误码";
}

HttpManager::~HttpManager()
{
    close();
}


std::string HttpManager::dump_headers(const Headers &headers) {
  std::string s;
  char buf[BUFSIZ];

  for (auto it = headers.begin(); it != headers.end(); ++it) {
    const auto &x = *it;
    snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
    s += buf;
  }

  return s;
}

void HttpManager::handleUserLogin(const Request &req, Response &res)
{

}


void HttpManager::getMonitorInfo(const Request &req, Response &res)
{
    auto body = dump_headers(req.headers);//+ dump_multipart_files(req.files);
    
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

void HttpManager::getGpusInfo(const Request &req, Response &res)
{
    auto body = dump_headers(req.headers);//+ dump_multipart_files(req.files);
    
    string strData;
    cdataProcess dataprocess;
    //dataprocess.GetMonitorsInfo(strData);
    //dataprocess.SetMonitorsInfo();
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

void HttpManager::setMonitorInfo(const Request &req, Response &res)
{
    auto headers = dump_headers(req.headers);
    auto body = req.body;
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
        info.pos.xPos = (*it)["xVirtual"].template get<int>();
        info.pos.yPos = (*it)["yVirtual"].template get<int>();
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



void HttpManager::addDlg(const Request &req, Response &res)
{

}
void HttpManager::delDlg(const Request &req, Response &res)
{
    
}
void HttpManager::modifyDlg(const Request &req, Response &res)
{
    
}
void HttpManager::searchDlg(const Request &req, Response &res)
{
    
}
void HttpManager::getAllDlg(const Request &req, Response &res)
{
    
}

void HttpManager::getTestMonitorInfo(const Request &req, Response &res)
{
    auto headers = dump_headers(req.headers);
    auto body = req.body;

    cdataProcess dataprocess;
    dataprocess.TestMonitorInfo();
}

void HttpManager::listenThread()
{

    //std::function<void(const Request &, Response &)> handler = HttpManager::dataProcess;
    std::function<void(const Request &, Response &)> funcPtrUserLogin = std::bind( &HttpManager::handleUserLogin, this,std::placeholders::_1,std::placeholders::_2);
    m_svr.Post("/dcps/?service=User.Login",funcPtrUserLogin);

    std::function<void(const Request &, Response &)> funcPtrGetScreens = std::bind(&HttpManager::getMonitorInfo, this, std::placeholders::_1, std::placeholders::_2);
    std::function<void(const Request &, Response &)> funcPtrGetGpus = std::bind(&HttpManager::getGpusInfo, this, std::placeholders::_1, std::placeholders::_2);
    std::function<void(const Request &, Response &)> funcPtrSetScreens = std::bind(&HttpManager::setMonitorInfo, this, std::placeholders::_1, std::placeholders::_2);

    // 增，删，改，查窗口列表
    std::function<void(const Request &, Response &)> funcPtrDlgAdd = std::bind(&HttpManager::addDlg, this, std::placeholders::_1, std::placeholders::_2);
    std::function<void(const Request &, Response &)> funcPtrDlgDel = std::bind(&HttpManager::delDlg, this, std::placeholders::_1, std::placeholders::_2);
    std::function<void(const Request &, Response &)> funcPtrDlgModify = std::bind(&HttpManager::modifyDlg, this, std::placeholders::_1, std::placeholders::_2);
    std::function<void(const Request &, Response &)> funcPtrDlgSearch = std::bind(&HttpManager::searchDlg, this, std::placeholders::_1, std::placeholders::_2);
    std::function<void(const Request &, Response &)> funcPtrDlgGetAll = std::bind(&HttpManager::getAllDlg, this, std::placeholders::_1, std::placeholders::_2);

    std::function<void(const Request &, Response &)> funcPtrTest = std::bind(&HttpManager::getTestMonitorInfo, this, std::placeholders::_1, std::placeholders::_2);

    m_svr.Get("/displayctrlserver/get/monitor/info",funcPtrGetScreens);
    m_svr.Get("/displayctrlserver/get/gpu/info",funcPtrGetGpus);
    m_svr.Post("/displayctrlserver/set/monitor/info",funcPtrSetScreens);

    m_svr.Get("/displayctrlserver/get/test",funcPtrTest); 
    XINFO("http开启监听,IP:{},Port:{}",m_Ip,m_nPort);
    m_svr.listen(m_Ip, m_nPort);   

}


void HttpManager::run()
{   
    //boost::thread thr(&HttpManager::listenThread,this);
    listenThread();
}

bool HttpManager::close()
{
    m_svr.stop();
    return true;
}