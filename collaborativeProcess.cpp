#include <unistd.h>
#include <chrono>
#include <cstdio>
#include "3rd/cpp-httplib/httplib.h"
#include "src/cspdlog.h"
#include "3rd/json/include/nlohmann/json.hpp"
#include <sys/reboot.h>
#include <3rd/cpp-subprocess/subprocess.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "src/IniReader.h"


using namespace std;
using json = nlohmann::json;
namespace sp = subprocess;



enum MsgCmd
{
    MSG_CMD_NONE = 0,
    MSG_CMD_REBOOT = 1,
    MSG_CMD_RESTARTX11 = 2,
    MSG_CMD_SETIP = 3,
};

struct MsgInfo
{
    MsgCmd msgCmd;
    string msgJson;
    /* data */
};

list <MsgInfo> msgList;
boost::mutex m_msglstMtx;

using namespace httplib;

std::string dump_headers(const Headers &headers)
{
    std::string s;
    char buf[BUFSIZ];

    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        const auto &x = *it;
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }

    return s;
}

void * http_server(void *arg)
{
    CIniReader iniReader("config.ini");
    int nPort = iniReader.ReadInteger("common", "colbPort", 18186);///port
    XINFO("read config http server port {}\n",nPort);

    Server svr;
    svr.Get("/displaycontrol/reboot", [](const Request& req, Response& res) {
        XINFO("receive /displaycontrol/reboot\n");
    MsgInfo msg;
    msg.msgCmd = MSG_CMD_REBOOT;
    boost::lock_guard<boost::mutex> lock(m_msglstMtx);
    msgList.push_back(msg);
    res.status = 200;
    //res.set_content("Hello World!", "application/json");
  });

    svr.Get("/displaycontrol/resartx11", [](const Request& req, Response& res) {
        XINFO("receive /displaycontrol/resartx11\n");
    MsgInfo msg;
    msg.msgCmd = MSG_CMD_RESTARTX11;
    boost::lock_guard<boost::mutex> lock(m_msglstMtx);
    msgList.push_back(msg);
    sleep(3);
    res.status = 200;
    //res.set_content("Hello World!", "application/json");
  });

    svr.Post("/displaycontrol/setip", [](const Request& req, Response& res) {
        XINFO("receive /displaycontrol/resartx11\n");
        auto headers = dump_headers(req.headers);
        auto body = req.body;
    MsgInfo msg;
    msg.msgCmd = MSG_CMD_SETIP;
    msg.msgJson = body;
    boost::lock_guard<boost::mutex> lock(m_msglstMtx);
    msgList.push_back(msg);
    //sleep等待修改IP完成后，是否可以返回200？
    res.status = 200;
  });

    svr.listen("localhost", 18186);
    return 0;
}

void * msgThread(void *arg)
{
    while (true)
    {
        if (!msgList.empty())
        {
            boost::lock_guard<boost::mutex> lock(m_msglstMtx);
            MsgInfo msg = msgList.front();
            switch (msg.msgCmd)
            {
            case MSG_CMD_REBOOT:
                XINFO("reboot\n");
                {
                    sync();
                    int nRet = reboot(RB_AUTOBOOT);                    
                }
                
                break;
            case MSG_CMD_RESTARTX11:
            {
                try
                {
                    // auto obuf = subprocess::check_output({"systemctl", "restart gdm3"});
                    // std::cout << "Data : " << obuf.buf.data() << std::endl;
                    // std::cout << "Data len: " << obuf.length << std::endl;
                    // XINFO("systemctl restart gdm,ret:{}",obuf.buf.data());
                    int status = system("systemctl restart gdm3");
                    if (status == -1)
                    {
                        XERROR("systemctl restart gdm3 error = %d\n",errno);
                    }
                    else
                    {
                        XINFO("Command executed with status %d\n", status);                        
                    }

                    // status = system("systemctl restart dpcs");
                    // if (status == -1)
                    // {
                    //     XERROR("systemctl restart gdm3 error = %d\n",errno);
                    // }
                    // else
                    // {
                    //     XINFO("Command executed with status %d\n", status);                        
                    // }               
                    
                }
                catch(...)
                {
                    XERROR("try catch error,systemctl restart gdm,errno={}",errno);
                }
                
            }
                break;
            case MSG_CMD_SETIP:
            {
                try
                {                    
                    XINFO("msgThread deal msg:{}", msg.msgJson);
                    json jdata = json::parse(msg.msgJson);
                    //int num = jdata.at("num").get<int>();
                }
                catch(...)
                {
                    XERROR("try catch error, set ip,errno={}",errno);
                }

            }
            break;
            default:
                break;
            }

            msgList.pop_front();
            // 发送消息

        }
        else
        {
            sleep(1);
        }
    }
    return 0;
   
}


int main() {
    std::shared_ptr<CSpdlog> splog(CSpdlog::GetInstance("colbp"));

    pthread_t thread[2];

    int nRet = pthread_create(&thread[0], NULL, http_server, NULL);
    if (nRet)
    {
        XINFO("ERROR; return code from pthread_create() is %d\n", nRet);
        exit(-1);
    }

    nRet = pthread_create(&thread[1], NULL, msgThread, NULL);
    if (nRet)
    {
        XINFO("ERROR; return code from pthread_create() is %d\n", nRet);
        exit(-1);
    }
    XINFO("dpcs collaborative process started\n");
    while (1)
    {
        sleep(1);
    }
    
    return 0;
}