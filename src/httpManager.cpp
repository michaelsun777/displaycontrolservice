#include "httpManager.h"


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

HttpManager::HttpManager(/* args */):m_nPort(18180),m_nTimes(0),m_nUniformity_w(4096),m_nUniformity_h(2160)
{
   

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


void HttpManager::msgParse(const Request &req, Response &res)
{
    auto body = dump_headers(req.headers);//+ dump_multipart_files(req.files);

    //分屏格式1x2 1x3 1x4 1x5 1x6 1x7 1x8 ... 1x16
    //分屏格式2x2 2x4 2x8 2x16 3x3  4x4 4x2 4x1 5x3 5x2 5x1 6x2 6x1 7x2 7x1 8x2 8x1
    // int result = 0;
    // if(m_nTimes%1 == 0)
    //     result = system("xrandr --output DP-1 --left-of DP-2 --auto"); 
    // else
    //     result = system("xrandr --output DP-2 --left-of DP-1 --auto"); 
    // // 检查命令是否执行成功
    // if (result == 0)
    // {
    //     XINFO("Setting Successed!");
    // }
    // else
    // {
    //     XERROR("Setting failed!");
    // }
    // m_nTimes++;




    res.set_content("hello word!", "text/plain");

}

bool HttpManager::start()
{   
    //std::function<void(const Request &, Response &)> handler = HttpManager::dataProcess;
    std::function<void(const Request &, Response &)> funcPtr = std::bind( &HttpManager::msgParse, this,std::placeholders::_1,std::placeholders::_2);
    m_svr.Post("/displayctrlserver/getinfo",funcPtr);
    m_svr.listen("localhost", m_nPort);
    
    XINFO("开启监听端口");
    
    return true;
}

bool HttpManager::close()
{
    m_svr.stop();
    return true;
}