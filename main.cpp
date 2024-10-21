
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

    cdataProcess pro;
    pro.GetMonitorsInfo();
    pro.SetMonitorsInfo();

    string strDisplayName = ":0";
    cmyxrandr xr(strDisplayName);
    list<RROutput> outputs = xr.getOutputs();

    
    for (list<RRCrtc>::iterator it = outputs.begin(); it != outputs.end(); it++)   
    {
        RROutput output = *it;
        cmyxrandr rroutput(strDisplayName, output);

        string strStatus = (rroutput.isConnected()) ? "Connected" : "Not connected";
        string strPrimary = (rroutput.isPrimary()) ? "Primary" : "";
        XINFO("{},{},{}",rroutput.getName(),strStatus ,strPrimary);

        if (strStatus == "Connected")
        {
            list<CMYSIZE> modes = rroutput.getOutputModes();
            for (list<CMYSIZE>::iterator it = modes.begin(); it != modes.end(); it++)
            {
                XINFO("w={},h={}",it->width,it->height);
            }           
            //QList<QSize> modes = rroutput.getOutputModes();
            // foreach (QSize item, modes)
            // {
            //     qDebug() << item.width() << "x" << item.height();
            // }
        }
    }

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