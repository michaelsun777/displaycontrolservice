
#include <iostream>
#include <unistd.h>
#include "cspdlog.h"
#include <list>

#include "build_date_time.h"
#include "httpManager.h"
#include "cdataProcess.h"
#include "cmyxrandr.h"


#define VER_AUTO
std::string g_NameString = "Display Ctrl Server";
std::string g_VERSION = "DCS-0.0.1-241021-1";
std::string g_BRANCH = "1.0";
std::string g_DATE = g_build_date_time;// "231103";
std::string g_NOTE = "XXX";


int main(int argc, char *argv[])
{
    std::shared_ptr<CSpdlog> splog(CSpdlog::GetInstance());

    //cmyxrandr cr(":0");
    // cr.getScreenInfo();
    // XRRScreenSize * psize = cr.getCurrentConfigSizes();
    // delete psize;

    //cdataProcess dataProcess;
    //string strJons;
    //dataProcess.GetMonitorsInfo(strJons);
    
    std::shared_ptr<HttpManager> pHttpManager = HttpManager::GetInstance();


    

    

    while (1)
    {
        std::string console;
        if (std::getline(std::cin, console))
        {
            // process(&console);
            // std::cout<<"cmd: "<< console  <<std::endl;
            if (console.find("stop") != std::string::npos)
                break;
        }
    }

    return 0;
}