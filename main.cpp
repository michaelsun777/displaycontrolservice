

#include <QApplication>
#include <QObject>
#include <QSettings>
#include "3rd/httpserver/httplistener.h"
#include "requesthandler.h"
#include <iostream>
#include <unistd.h>
#include "cspdlog.h"
#include <list>
//#include "httpManager.h"
#include "UI/dlgurl.h"
#include "UI/mainwindow.h"

#include "build_date_time.h"
//#include "dlgManager.h"
#include "cmyxrandr.h"

#include "cdataProcess.h"

#include "nvControlInfo.h"

//#include <X11/extensions/Xrandr.h>
#include "../3rd/md5/src/md5.h"




#define VER_AUTO
std::string g_NameString = "Display Control Service";
std::string g_VERSION = "dpcs-0.0.1-241120-1";
std::string g_BRANCH = "dev";
std::string g_DATE = g_build_date_time;// "231103";
std::string g_NOTE = "";

void dumpVersion();


int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    
    int nRet = 0;
    std::shared_ptr<CSpdlog> splog(CSpdlog::GetInstance());
    dumpVersion();
    try
    {
        QFile fileUser("./user.db");
        if (!fileUser.exists())
        {
            QSettings config("user.db", QSettings::IniFormat);
            config.setValue("user/name", "admin");
            config.setValue("user/pwd", "admin");
            config.sync();
        }
        fileUser.close();


        QFile file("./config.ini");
        if (!file.exists())
        {
            QSettings config("config.ini", QSettings::IniFormat);
            config.setValue("common/ip", "127.0.0.1");
            config.setValue("common/port", "18180");
            config.setValue("screen/width", 1920);
            config.setValue("screen/height", 1080);
            config.setValue("screen/layout_horizontal", 1);
            config.setValue("screen/layout_vertical", 1);

            XCRITICAL("config.ini不存在,程序自动创建了配置文件样本,请修改配置文件！");
            XCRITICAL("请先配置config.ini文件,配置文件在程序目录下");
            exit(0);
        }
        file.close();

        QSettings getConfig("./config.ini", QSettings::IniFormat);

        //setConfig.GetValue("common","ip",value,error);
        QString _ip = "127.0.0.1";
        QString _port = "18180";
        getConfig.beginGroup("common");
        QVariant value;
        value = getConfig.value("ip");
        if(!value.isNull())
        {
            _ip = value.toString();
        }        
        value.clear();
        value = getConfig.value("port");
        if(!value.isNull())
        {
            _port = value.toString();
        }
        

        MainWindow w;

        QSettings settings(&app);
        settings.setValue("host",_ip);
        settings.setValue("port",_port);
        settings.setValue("minThreads","4");
        settings.setValue("maxThreads","20");
        settings.setValue("cleanupInterval","60000");
        settings.setValue("readTimeout","60000");
        settings.setValue("maxRequestSize","16000");
        settings.setValue("maxMultiPartSize","10000000");
        RequestHandler * pRequestHandler = new RequestHandler(&w,&app);
        new HttpListener(&settings,pRequestHandler,&app);


        QApplication::setQuitOnLastWindowClosed(false);
        
        //cdataProcess dataProcess;
        // dataProcess.TestMonitorInfo();
        // sleep(3);
        // return 0;
        cdataProcess* pcdataProcess = cdataProcess::GetInstance();
        pcdataProcess->SetMainWindow(&w);
        pcdataProcess->InitOutputInfo(); 
        pcdataProcess->InitMainOutputModes();
        QObject::connect(pRequestHandler,&RequestHandler::sendDlgSignal,&w,&MainWindow::onMouseEventRequested);
          

        //std::shared_ptr<dlgManager> pdlgManager = dlgManager::GetInstance();

        // DlgUrl dlg;
        // dlg.show();
        // dlg.UpdateUrl("https://www.baidu.com");
        //std::shared_ptr<HttpManager> pHttpManager = HttpManager::GetInstance();
        
        w.show();
        w.Init();
        w.hide();
        nRet = app.exec();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return nRet;
}

void dumpVersion()
{
    XINFO("\n");
    XINFO("========================================================");
    XINFO("{}",g_NameString.c_str());
#ifdef VER_AUTO
    XINFO("Version:{}",g_VERSION.c_str());
    XINFO("Branch: {}", g_BRANCH.c_str());
    XINFO("compile date: {}",g_DATE.c_str());
#else
    printf("Version:  \n");
    printf("Branch:   \n");
#endif
    XINFO("NOTE: {}",g_NOTE.c_str());
    XINFO("========================================================\n");
}