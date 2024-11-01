#ifndef CDATA_PROCESS_H
#define CDATA_PROCESS_H

#include <QApplication>
#include "UI/mainwindow.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>


#include "3rd/cpp-subprocess/subprocess.hpp"
#include <iostream>
#include <cstring>
#include "3rd/nvidia-settings/src/libXNVCtrl/NVCtrl.h"
#include "3rd/nvidia-settings/src/libXNVCtrl/NVCtrlLib.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>

#include "cspdlog.h"
#include <list>
#include <vector>

#include "common.h"
#include "cmyxrandr.h"
#include "3rd/json/include/nlohmann/json.hpp"
#include "config.h"

#include "nvControlInfo.h"

//#include <nlohmann/json.hpp>
using json = nlohmann::json;


using namespace std;
using namespace subprocess;


struct MY_OutputInfo
{
    XRROutputInfo  outputInfo;
    XRRPropertyInfo  propertyInfo;    
};



class cdataProcess :public QObject
{     
    Q_OBJECT
private:
    int m_nAvailableMonitorsCount;
    std::map<XID,XRRMonitorInfo *> m_mMonitors;
    std::map<XID,MY_OutputInfo *> m_mOutputsInfo;
    int m_nWidth;
    int m_nHight;
    int m_ndistribution_w;
    int m_ndistribution_h;
    MainWindow * m_pMainWindow;


public:
    cdataProcess(/* args */);
    ~cdataProcess();
    void print_display_name(Display *dpy, int target_id, int attr,char *name);
    int GetNvXScreen(Display *dpy);
    void print_display_id_and_name(Display *dpy, int target_id, const char *tab);
    bool GetMonitorsInfo_N();
public://xrandr
    bool Output(string args);
    bool GetMonitorsInfo(string & strInfo);
    bool GetMonitorsInfo_shell(json & js);
    bool GetGpuInfo(json & js);
    bool SetMonitorsInfo(vector<MONITORSETTINGINFO> *vSetInfo);
    bool TestMonitorInfo();
    void SetMainWindow(MainWindow * p);
public:
    bool InitOutputInfo();

signals:
    void testSignal(int type);
    
    
};




#endif