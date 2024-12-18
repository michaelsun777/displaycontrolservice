

#include <QApplication>
#include <QStandardPaths>
#include <QCefContext.h>
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
#include "UI/mymainwindow.h"

#include "build_date_time.h"
//#include "dlgManager.h"
#include "cmyxrandr.h"

#include "cdataProcess.h"

#include "nvControlInfo.h"

//#include <X11/extensions/Xrandr.h>
#include "../3rd/md5/src/md5.h"
#include "autoDelete.h"




#define VER_AUTO
std::string g_NameString = "Display Control Service";
std::string g_VERSION = "dpcs-0.0.2-241210-1";
std::string g_BRANCH = "dev";
std::string g_DATE = g_build_date_time;// "231103";
std::string g_NOTE = "";

void dumpVersion();
bool InitQCefConfig(QCefConfig & config,QApplication & app,int argc, char *argv[]);
unsigned int read_line_first_word(FILE *f,char *s);
unsigned int get_sys_runtime(void);


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

        shared_ptr<CAutoDeleteManager> pCAutoDeleteManager = CAutoDeleteManager::GetInstance();
        pCAutoDeleteManager->AddNeedDeleteFileInfo("/var/log/dpcs/", 15);
        pCAutoDeleteManager->AddNeedDeleteFileInfo("./var/", 15);

        QFile file("./config.ini");
        if (!file.exists())
        {
            QSettings config("config.ini", QSettings::IniFormat);
            //config.setValue("common/ip", "");
            config.setValue("common/port", "18180");
            config.setValue("screen/width", 1920);
            config.setValue("screen/height", 1080);
            config.setValue("screen/layout_horizontal", 1);
            config.setValue("screen/layout_vertical", 1);
            config.setValue("screen/isSetting", "false");

            XCRITICAL("config.ini不存在,程序自动创建了配置文件样本,请修改配置文件！");
            XCRITICAL("请先配置config.ini文件,配置文件在程序目录下");
            //exit(0);
        }
        file.close();

        {
            CMDEXEC::CmdRes respid;
            bool bret = false;
            unsigned int uRunTimesTmp = get_sys_runtime();
            XINFO("uRunTimesTmp 0 ={}\n", uRunTimesTmp);            
            string strpgrep = "pgrep -x gdm3";
            while (1)
            {
                bret = CMDEXEC::Execute(strpgrep, respid);
                if(bret)
                {
                    vector<string> vString;
                    CMDEXEC::Stringsplit(respid.StdoutString, '\n', vString);
                    if (vString.size() > 0)
                    {
                        unsigned int uRunTimes = get_sys_runtime();
                        XINFO("uRunTimes 1 = {}\n", uRunTimes);
                        if (uRunTimes > 12)
                        {
                            break;
                        }
                        else
                        {
                            usleep(5000);
                        }
                    }
                    else
                    {
                        sleep(1);
                    }
                }
                else
                {
                    XERROR("pgrep -x gdm3 执行失败,程序退出\n");
                    return -1;
                }
                
            }
        }

        QSettings getConfig("./config.ini", QSettings::IniFormat);

        //setConfig.GetValue("common","ip",value,error);
        QString _ip = "";//127.0.0.1默认为QHostAddress::Any
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
        settings.setValue("maxThreads","200");
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
        pcdataProcess->Init();
        QObject::connect(pRequestHandler,&RequestHandler::sendDlgSignal,&w,&MainWindow::onMouseEventRequested);
          

        //std::shared_ptr<dlgManager> pdlgManager = dlgManager::GetInstance();

        // DlgUrl dlg;
        // dlg.show();
        // dlg.UpdateUrl("https://www.baidu.com");
        // std::shared_ptr<HttpManager> pHttpManager = HttpManager::GetInstance();
        // build QCefConfig
        MyMainWindow *pMyW = nullptr;
        QCefConfig config;

        // config.setUserAgent("QCefViewTest");
        // // set log level
        // config.setLogLevel(QCefConfig::LOGSEVERITY_DEFAULT);
        // // set JSBridge object name (default value is CefViewClient)
        // config.setBridgeObjectName("CallBridge");
        // // set Built-in scheme name (default value is CefView)
        // config.setBuiltinSchemeName("CefView");
        // // port for remote debugging (default is 0 and means to disable remote debugging)
        // config.setRemoteDebuggingPort(9000);
        // // set background color for all browsers
        // // (QCefSetting.setBackgroundColor will overwrite this value for specified browser instance)
        // config.setBackgroundColor(Qt::lightGray);

        // // WindowlessRenderingEnabled is set to true by default,
        // // set to false to disable the OSR mode
        // config.setWindowlessRenderingEnabled(true);

        // // add command line args, you can any cef supported switches or parameters
        // config.addCommandLineSwitch("use-mock-keychain");
        // // config.addCommandLineSwitch("disable-gpu");
        // // config.addCommandLineSwitch("enable-media-stream");
        // // config.addCommandLineSwitch("allow-file-access-from-files");
        // // config.addCommandLineSwitch("disable-spell-checking");
        // // config.addCommandLineSwitch("disable-site-isolation-trials");
        // // config.addCommandLineSwitch("enable-aggressive-domstorage-flushing");
        // config.addCommandLineSwitchWithValue("renderer-process-limit", "1");
        // // allow remote debugging
        // config.addCommandLineSwitchWithValue("remote-allow-origins", "*");
        // // config.addCommandLineSwitchWithValue("disable-features", "BlinkGenPropertyTrees,TranslateUI,site-per-process");

        // // set cache folder
        // config.setCachePath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
        // QCefContext cefContext(&app, argc, argv, &config);
        // pMyW = new MyMainWindow();
        // pMyW->show();




        bool bretQCef = InitQCefConfig(config, app, argc, argv);
        if (!bretQCef)
        {
           XERROR("InitQCefConfig failed\n");
           return -1;
        }
        QCefContext cefContext(&app, argc, argv, &config);
        pMyW = new MyMainWindow();
        pMyW->show();

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


/*************************************************
  Function:    read_line_first_word
  Description: 读取指定的文件中一行中的一段(到第一个空格)到一字符串中
  Input:       
			  1.文件指针
			  2.欲存储的字符串指针
  Return: 	   读取的最后一个字符          
*************************************************/
unsigned int read_line_first_word(FILE *f,char *s) 
{
	unsigned int chr;
		
    do
	{
		chr = fgetc(f);
		*(s++) = chr;
	}while(chr!='\x20' && chr!=EOF && chr!='\xa'&& chr!='\xd');
	
	*(s-1)='\0';
	return chr;
}

/*************************************************
  Function:		get_sys_runtime
  Description:  获取系统启动运行时间	//daisy
  Input:		无
  Output:		
  Return:		系统启动运行时间
  Others:		
*************************************************/
unsigned int get_sys_runtime(void)
{
	char buf[30];
	unsigned int sys_run_time = 0;
	FILE *pf;
	
    memset(buf, 0, sizeof(buf));
    pf = popen("cat /proc/uptime", "r");
    if (NULL != pf)
    {
		read_line_first_word(pf, buf);
		sys_run_time = atoi(buf);

        pclose(pf);
        pf = NULL;
    }
	
	return sys_run_time;
}

bool InitQCefConfig(QCefConfig & config,QApplication & app,int argc, char *argv[])
{
    try
    {
        // build QCefConfig
        // QCefConfig config;
        // set user agent
        // config.setUserAgent("QCefViewTest");
        // set log level
        config.setLogLevel(QCefConfig::LOGSEVERITY_DEFAULT);
        // set JSBridge object name (default value is CefViewClient)
        config.setBridgeObjectName("CallBridge");
        // set Built-in scheme name (default value is CefView)
        config.setBuiltinSchemeName("CefView");
        // port for remote debugging (default is 0 and means to disable remote debugging)
        config.setRemoteDebuggingPort(9000);
        // set background color for all browsers
        // (QCefSetting.setBackgroundColor will overwrite this value for specified browser instance)
        config.setBackgroundColor(Qt::lightGray);
        // WindowlessRenderingEnabled is set to true by default,
        // set to false to disable the OSR mode
        config.setWindowlessRenderingEnabled(true);

        // add command line args, you can any cef supported switches or parameters
        config.addCommandLineSwitch("use-mock-keychain");
        //config.addCommandLineSwitch("disable-gpu");
        config.addCommandLineSwitch("enable-media-stream");
        config.addCommandLineSwitch("use-gl=desktop");
        config.addCommandLineSwitch("enable-gpu-rasterization");
        // config.addCommandLineSwitch("allow-file-access-from-files");
        // config.addCommandLineSwitch("disable-spell-checking");
        // config.addCommandLineSwitch("disable-site-isolation-trials");
        // config.addCommandLineSwitch("enable-aggressive-domstorage-flushing");
        config.addCommandLineSwitchWithValue("renderer-process-limit", "1");
        // allow remote debugging
        config.addCommandLineSwitchWithValue("remote-allow-origins", "*");
        // config.addCommandLineSwitchWithValue("disable-features", "BlinkGenPropertyTrees,TranslateUI,site-per-process");

        // set cache folder
        config.setCachePath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
        // create QCefContext instance with config,
        // the lifecycle of cefContext must be the same as QApplication instance     
        // QCefContext cefContext(&app, argc, argv, &config);

        
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}
