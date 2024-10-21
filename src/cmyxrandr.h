#ifndef _CMYXRANDR_H_
#define _CMYXRANDR_H_

#include <X11/extensions/Xrandr.h>
#include <list>
#include <string>

#include "cspdlog.h"


using namespace std;

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




class cmyxrandr
{
    
private:
    XRRScreenResources  *m_pRes;
    Display             *m_pDpy;
    Window               m_root;
    int                  m_screen;
    RRCrtc               m_crtc;
    RROutput             m_output;


private:
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
    list<CMYSIZE>        getOutputModes  ();
    bool                isConnected     ();
    bool                isEnabled       ();
    string             getName         ();
    list<CMYSIZE>        getModes        ();
    Status               enable          (CMYSIZE size);
    CMYSIZE               getOutputSize   ();
    RROutput            getOutputByName (string strName);
    list<RROutput>     getOutputs();

    Status              getXRandrVersion (int *ver, int *rev) const;
    Rotation            getRotate       () const;
    Rotation            getReflect      () const;
    CMYPOINT            getOffset       () const;
    RRMode              getMode         () const;
    Status              disable         ();
    CMYSIZE             getScreenSize   () const;
    Status              setScreenSize   (const int &width, const int &height, bool bForce = false);
    int                 setReflect      (Rotation reflection);
    Status              setOffset       (CMYPOINT offset);
    int                 setMode         (CMYSIZE size);
    int                 setRotate       (Rotation rotation);
    int                 setPanning      (CMYSIZE size);
    void                startEvents     ();
    Status              feedScreen      ();
    CMYSIZE             getPreferredMode();
    bool                isPrimary       ();
    void                setPrimary      ();
   
private:
    /* data */
public:
    cmyxrandr(string strDisplayName,RROutput output = NULL);
    ~cmyxrandr();
};








#endif