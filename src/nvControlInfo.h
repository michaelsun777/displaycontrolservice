#ifndef _NVCONTROLINFO_H_
#define _NVCONTROLINFO_H_

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <X11/Xlib.h>

#include "NVCtrl.h"
#include "NVCtrlLib.h"

#include <string.h>
#include <cstring>
#include <string>
#include <vector>

#include "cspdlog.h"
#include "common.h"

#include "exec_cmd.h"


using namespace std;
using namespace MYCOMMON;

class nvControlInfo
{
private:
    int                 m_major;
    int                 m_minor;
    int                 m_event_base; 
    int                 m_error_base;
    int                 m_screensNum;
    vector<string>              m_vGpuName;
    int                 m_gpuNum;
    string              m_videoBIOS;
    string              m_gpuDriverVersion;

public:
    nvControlInfo(/* args */);
    ~nvControlInfo();
    bool getGpuInfo(MGPUINFOEX & gpu);
    bool getGpuTempture(std::vector<MGPUINFO> &vMGPUINFO);
};





#endif