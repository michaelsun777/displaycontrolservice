

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




#define VER_AUTO
std::string g_NameString = "Display Ctrl Server";
std::string g_VERSION = "DCS-0.0.1-241021-1";
std::string g_BRANCH = "1.0";
std::string g_DATE = g_build_date_time;// "231103";
std::string g_NOTE = "XXX";


int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    int nRet = 0;
    try
    {
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
        std::shared_ptr<CSpdlog> splog(CSpdlog::GetInstance());
        cdataProcess dataProcess;
        
        dataProcess.SetMainWindow(&w);
        dataProcess.InitOutputInfo(); 
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