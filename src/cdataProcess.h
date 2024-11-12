#ifndef CDATA_PROCESS_H
#define CDATA_PROCESS_H

#include <QApplication>
#include "UI/mainwindow.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>


//
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
#include "3rd/cpp-subprocess/subprocess.hpp"

//#include <nlohmann/json.hpp>
using json = nlohmann::json;


using namespace std;

namespace sp = subprocess;


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
    bool GetServerInfo(json & js);
    void get_memoccupy(MEM_OCCUPY *mem); //对无类型get函数含有一个形参结构体类弄的指针O
    int get_cpuoccupy(CPU_OCCUPY *cpust); //对无类型get函数含有一个形参结构体类弄的指针O
    void cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n,float & util);
    std::string getCpuName();
    std::string get_cur_executable_path();
public:
    bool InitOutputInfo();

signals:
    void testSignal(int type);
    
    
};




#endif