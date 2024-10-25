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

            if(m_pHttpManager->start())
            {
                return m_pHttpManager;
            }
            else
            {
                return NULL;
            }
            
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


void HttpManager::getMonitorInfo(const Request &req, Response &res)
{
    auto body = dump_headers(req.headers);//+ dump_multipart_files(req.files);
    
    string strData;
    cdataProcess dataprocess;
    //dataprocess.GetMonitorsInfo(strData);
    //dataprocess.SetMonitorsInfo();
    dataprocess.GetMonitorsInfo_shell(strData);
    res.set_content(strData, "text/plain");

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
        info.pos.xPos = (*it)["xPos"].template get<int>();
        info.pos.xPos = (*it)["yPos"].template get<int>();
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
        }
        
        
        if(bRet)
        {
            string strRet = "{\"result\":\"OK\",\"msg\":\"\"}";
            res.set_content(strRet, "text/plain");
        }
        else
        {
            string strRet = "{\"result\":\"failed\",\"msg\":\"";
            strRet+= strErrorMsg;
            strRet+= "\"}";            
            res.set_content(strRet, "text/plain");
        }
        
        
    }
    else
    {
        string strRet = "{\"result\":\"error\",\"msg\":\"test error\"}";
        res.set_content(strRet, "text/plain");
    }

}

void HttpManager::getTestMonitorInfo(const Request &req, Response &res)
{
    auto headers = dump_headers(req.headers);
    auto body = req.body;

    cdataProcess dataprocess;
    dataprocess.TestMonitorInfo();

}

bool HttpManager::start()
{   
    //std::function<void(const Request &, Response &)> handler = HttpManager::dataProcess;
    std::function<void(const Request &, Response &)> funcPtrGet = std::bind( &HttpManager::getMonitorInfo, this,std::placeholders::_1,std::placeholders::_2);
    std::function<void(const Request &, Response &)> funcPtrSet = std::bind( &HttpManager::setMonitorInfo, this,std::placeholders::_1,std::placeholders::_2);

    std::function<void(const Request &, Response &)> funcPtrTest = std::bind( &HttpManager::getTestMonitorInfo, this,std::placeholders::_1,std::placeholders::_2);


    m_svr.Get("/displayctrlserver/get/monitor/info",funcPtrGet);   
    m_svr.Post("/displayctrlserver/set/monitor/info",funcPtrSet);
    m_svr.Get("/displayctrlserver/get/test",funcPtrTest); 
    XINFO("http开启监听,IP:{},Port:{}",m_Ip,m_nPort);
    m_svr.listen(m_Ip, m_nPort);   
    
    return true;
}

bool HttpManager::close()
{
    m_svr.stop();
    return true;
}