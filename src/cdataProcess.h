#ifndef CDATA_PROCESS_H
#define CDATA_PROCESS_H

#include <X11/Xlib.h>
/* we need to be able to manipulate the Display structure on events */
// #include <X11/Xlibint.h>
// #include <X11/extensions/render.h>
// #include <X11/extensions/Xrender.h>
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


using namespace std;
using namespace subprocess;


struct MY_OutputInfo
{
    XRROutputInfo  outputInfo;
    XRRPropertyInfo  propertyInfo;    
};



class cdataProcess
{     
private:
    int m_nAvailableMonitorsCount;
    std::map<XID,XRRMonitorInfo *> m_mMonitors;
    std::map<XID,MY_OutputInfo *> m_mOutputsInfo;
public:
    cdataProcess(/* args */);
    ~cdataProcess();
    void print_display_name(Display *dpy, int target_id, int attr,char *name);
    int GetNvXScreen(Display *dpy);
    void print_display_id_and_name(Display *dpy, int target_id, const char *tab);
    bool GetMonitorsInfo_N();
public://xrandr
    bool Output(string args);
    bool GetMonitorsInfo();
    bool SetMonitorsInfo();
    
    
};




#endif