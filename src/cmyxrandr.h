#ifndef _CMYXRANDR_H_
#define _CMYXRANDR_H_

#include <X11/extensions/Xrandr.h>
#include <list>
#include <string>

#include "cspdlog.h"
#include "common.h"

#include "exec_cmd.h"
#include "CNvControlEvents.h"


using namespace std;

using namespace MYCOMMON;

#define MAX_SIZE 200



class cmyxrandr
{
    
private:
    XRRScreenResources  *m_pRes;
    Display             *m_pDpy;
    Window               m_root;
    int                  m_screen;
    RRCrtc               m_crtc;
    RROutput             m_output;
    XRRScreenConfiguration * m_psConfig;
    string m_strDisplayName;
    int                 m_major;
    int                 m_minor;
    int                 m_event_base; 
    int                 m_error_base;
    static cmyxrandr *  m_instance;
    CNvControlEvents *  m_pEvents;
    vector<MOutputInfo> m_vOutputInfo;
    CMYSIZE m_currentSize;
    CMYSIZE m_maxSize;


public:
    static cmyxrandr * GetInstance();    
    int                 screen          () const;
    list<RRCrtc>        getAllCrtc      ();
    XRRScreenResources *pRes            () const;
    Display            *pDpy            () const;
    Window              root            () const;
    RRMode              getXRRModeInfo  (int width, int height);
    XRROutputInfo      *GetOutputInfo   ();
    list<RRCrtc>        getCrtcs();
    RRCrtc              getCrtc         ();

public:
    list<MyModelInfoEX *>        getOutputModes  ();
    bool                isConnected     ();
    bool                isEnabled       ();
    string             getName         ();
    list<CMYSIZE>        getModes        ();
    int               enable          (CMYSIZE size);
    CMYSIZE               getOutputSize   ();
    RROutput            getOutputByName (string strName);
    list<RROutput>     getOutputs();

    int              getXRandrVersion (int *ver, int *rev) const;
    Rotation            getRotate       () const;
    Rotation            getReflect      () const;
    CMYPOINT            getOffset       () const;
    RRMode              getMode         () const;
    int              disable         ();
    CMYSIZE             getScreenSize   () const;
    int              setScreenSize   (const int &width, const int &height, bool bForce = false);
    int                 setReflect      (Rotation reflection);
    int              setOffset       (CMYPOINT offset);
    int                 setMode         (CMYSIZE size,RRMode rrmode = 0);
    int                 setRotate       (Rotation rotation);
    int                 setPanning      (CMYSIZE size);
    void                startEvents     ();
    int              feedScreen      ();
    CMYSIZE             getPreferredMode();
    bool                isPrimary       ();
    void                setPrimary      ();
    XRRCrtcInfo *       getCrtcInfo();


   
private:
    /* data */
public:
    cmyxrandr(string strDisplayName,RROutput output = 0);
    ~cmyxrandr();
    void setOutPut(RROutput output);
    void setCrtc(RRCrtc crtc);
    XRRMonitorInfo * getScreenInfo();
    XRRScreenSize * getCurrentConfigSizes();
    unsigned short getCurrentConfigRotation();
    short getAllScreenInfoEx(vector<MOutputInfo> & vOutputInfo,CMYSIZE & currentSize,CMYSIZE & maxSize);
    short getAllScreenInfoNew(vector<MOutputInfo> & vOutputInfo,CMYSIZE & currentSize,CMYSIZE & maxSize);
    int getScreenInfoEx(MOutputInfo & vOutputInfo);
    int getScreenSizeRange(CMYSIZE & min,CMYSIZE & max);
    bool update();
    bool Init();
    bool OnUpdate();
};








#endif