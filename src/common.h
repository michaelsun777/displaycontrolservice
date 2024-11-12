#ifndef _COMMON_H_
#define _COMMON_H_

//#include <QApplication>

#include <list>
#include <string>

#include "cspdlog.h"

namespace MYCOMMON
{



#include <X11/extensions/Xrandr.h>

/*
#define XCONFIG_MODE_PHSYNC    0x0001
#define XCONFIG_MODE_NHSYNC    0x0002
#define XCONFIG_MODE_PVSYNC    0x0004
#define XCONFIG_MODE_NVSYNC    0x0008
#define XCONFIG_MODE_INTERLACE 0x0010
#define XCONFIG_MODE_DBLSCAN   0x0020
#define XCONFIG_MODE_CSYNC     0x0040
#define XCONFIG_MODE_PCSYNC    0x0080
#define XCONFIG_MODE_NCSYNC    0x0100
#define XCONFIG_MODE_HSKEW     0x0200 // hskew provided 
#define XCONFIG_MODE_BCAST     0x0400
#define XCONFIG_MODE_CUSTOM    0x0800 // timing numbers customized by editor 
#define XCONFIG_MODE_VSCAN     0x1000

typedef enum {
    ROTATION_0 = 0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270,
} Rotation;


*/

struct CMYSIZE
{
public:
    CMYSIZE(int _width = 0,int _height = 0){
        width = _width;
        height = _height;
    }
    void setWidth(int _width)
    {
        width = _width;
    }

    void setHeight(int _height)
    {
        height = _height;
    }


public:
    int width;
    int height;
};

struct CMYPOINT
{
public:
    CMYPOINT(int _x = 0,int _y = 0)
    {
        xPos = _x;
        yPos = _y;

    }
public:
    int xPos;
    int yPos; 
};


struct MyModelInfoEX
{
public:
    unsigned long		id;
    unsigned int	width;
    unsigned int	height;
    unsigned long	dotClock;
    unsigned int	hSyncStart;
    unsigned int	hSyncEnd;
    unsigned int	hTotal;
    unsigned int	hSkew;
    unsigned int	vSyncStart;
    unsigned int	vSyncEnd;
    unsigned int	vTotal;
    std::string		    name;
    unsigned int	nameLength;
    unsigned long	modeFlags;

    std::string     rate;
    std::string     hSync;
    std::string     vSync;
    bool            interlace;
public:
    MyModelInfoEX():name("")
    {

    }

    // MyModelInfoEX():name("")
    // {
        
    // }
    

    MyModelInfoEX(XRRModeInfo *mode):name("")
    {
        rate = "0";
        id = mode->id;
        width = mode->width;
        height = mode->height;
        dotClock = mode->dotClock;
        hSyncStart = mode->hSyncStart;
        hSyncEnd = mode->hSyncEnd;
        hTotal = mode->hTotal;
        hSkew = mode->hSkew;
        vSyncStart = mode->vSyncStart;
        vSyncEnd = mode->vSyncEnd;
        vTotal = mode->vTotal;        
        nameLength = mode->nameLength;
        //name = new char[nameLength];
        //memcpy(name,mode->name,nameLength);
        std::string strTmp(mode->name);
        name = strTmp.c_str();
        modeFlags = mode->modeFlags;
    }
    
    ~MyModelInfoEX()
    {
        //if(name) delete [] name;
    }

    void operator=(MyModelInfoEX mode)
    {
        rate = mode.rate.c_str();
        id = mode.id;
        width = mode.width;
        height = mode.height;
        dotClock = mode.dotClock;
        hSyncStart = mode.hSyncStart;
        hSyncEnd = mode.hSyncEnd;
        hTotal = mode.hTotal;
        hSkew = mode.hSkew;
        vSyncStart = mode.vSyncStart;
        vSyncEnd = mode.vSyncEnd;
        vTotal = mode.vTotal;        
        nameLength = mode.nameLength;
        //name = new char[nameLength];
        //memcpy(name,mode.name,nameLength);
        name = mode.name.c_str();
        modeFlags = mode.modeFlags;

    }

    


};

struct MONITORSETTINGINFO
{
    CMYSIZE size;
    CMYPOINT pos;
    std::string name;
    bool primary;
    int modeId;
    int outputId;
    int rotation;
};



struct MOutputInfo
{
    std::string name;
    int connected;//1"connected", 2"disconnected", 3'unknown-connection'
    bool primary;
    int current_rotation;
    unsigned short outputId;
    CMYPOINT pos;
    CMYSIZE size;
    CMYSIZE mmsize;
    std::string geometry;
    std::vector<MyModelInfoEX> modes;
    MyModelInfoEX currentMode;
    MyModelInfoEX preferredMode;
    
};



struct MGPUINFO
{
    int id;
    std::string name;
    std::string Fan;
    std::string Temp;
    std::string Perf;
    std::string Pwr;
    std::string Mem;
    std::string Util;

};

struct MGPUINFOEX
{
    std::string videoBIOS;
    std::string driverVersion;
    std::vector<MGPUINFO> vgpus;
};


struct DLGINFO
{
    std::string name;
    std::string dlgId;
    int type;//dlg type
    std::string url;
    std::string path;
    std::string param;
    int postype;//1需要转换比例，2真实屏幕坐标
    int xVirtual;
    int yVirtual;
    int xPos;
    int yPos;
    int width;
    int height;
    bool titleEnable; //是否显示标题
    
};

}




#endif