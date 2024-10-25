#include "cmyxrandr.h"

cmyxrandr::cmyxrandr(string strDisplayName, RROutput output) : m_screen(0), m_output(output), m_psConfig(NULL)
{
    m_strDisplayName = strDisplayName;
    m_pDpy = XOpenDisplay(m_strDisplayName.c_str());
    m_screen = DefaultScreen(m_pDpy);
    m_root = RootWindow(m_pDpy, m_screen);
    m_pRes = XRRGetScreenResources(m_pDpy, m_root);
    m_crtc = getCrtc();
}

cmyxrandr::~cmyxrandr()
{
    XSync(m_pDpy, false);
    if (m_pRes)
        XRRFreeScreenResources(m_pRes);

    XRRFreeScreenConfigInfo(m_psConfig);
    m_psConfig = NULL;
    XCloseDisplay(m_pDpy);
}

bool cmyxrandr::update()
{
    XSync(m_pDpy, false);
    if (m_pRes)
        XRRFreeScreenResources(m_pRes);

    XRRFreeScreenConfigInfo(m_psConfig);
    m_psConfig = NULL;
    XCloseDisplay(m_pDpy);

    m_pDpy = XOpenDisplay(m_strDisplayName.c_str());
    m_screen = DefaultScreen(m_pDpy);
    m_root = RootWindow(m_pDpy, m_screen);
    m_pRes = XRRGetScreenResources(m_pDpy, m_root);
    m_crtc = getCrtc();

}

void cmyxrandr::setOutPut(RROutput output)
{
    m_output = output;
}

void cmyxrandr::setCrtc(RRCrtc crtc)
{
    m_crtc = crtc;
}

XRRScreenResources *cmyxrandr::pRes() const
{
    return m_pRes;
}

Display *cmyxrandr::pDpy() const
{
    return m_pDpy;
}

Window cmyxrandr::root() const
{
    return m_root;
}

int cmyxrandr::screen() const
{
    return m_screen;
}

Rotation cmyxrandr::getRotate() const
{
    Rotation rotate = -1;
    if (m_crtc)
    {
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);
        if (pInfo)
        {
            rotate = pInfo->rotation & ~(RR_Reflect_X | RR_Reflect_Y);
            XRRFreeCrtcInfo(pInfo);
        }
    }
    return rotate;
}

Rotation cmyxrandr::getReflect() const
{
    int reflect = -1;

    if (m_crtc)
    {
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);
        if (pInfo)
        {
            reflect = pInfo->rotation & (RR_Reflect_X | RR_Reflect_Y);
            XRRFreeCrtcInfo(pInfo);
        }
    }
    return reflect;
}

CMYPOINT cmyxrandr::getOffset() const
{
    CMYPOINT offset;
    if (m_crtc)
    {
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);
        if (pInfo)
        {
            offset.xPos = pInfo->x,
            offset.yPos = pInfo->y;
            XRRFreeCrtcInfo(pInfo);
        }
    }
    return offset;
}

RRMode cmyxrandr::getMode() const
{
    RRMode mode = 0;
    if (m_crtc)
    {
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);
        if (pInfo)
        {
            mode = pInfo->mode;
            XRRFreeCrtcInfo(pInfo);
        }
    }
    return mode;
}

XRRCrtcInfo *cmyxrandr::getCrtcInfo()
{
    RRMode mode = 0;
    if (m_crtc)
    {
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);
        if (pInfo)
        {
            return pInfo;
        }
    }
    return NULL;
}

CMYSIZE cmyxrandr::getScreenSize() const
{
    int screen = DefaultScreen(m_pDpy);
    CMYSIZE mysize;
    mysize.width = DisplayWidth(m_pDpy, screen);
    mysize.height = DisplayHeight(m_pDpy, screen);
    return mysize;
}

RRCrtc cmyxrandr::getCrtc()
{
    RRCrtc rrcrtc = 0;
    if (m_output)
    {
        XRROutputInfo *pOutputInfo = XRRGetOutputInfo(m_pDpy, m_pRes, m_output);
        if (pOutputInfo->crtc)
        {
            rrcrtc = pOutputInfo->crtc;
            XRRFreeOutputInfo(pOutputInfo);
        }
    }
    return rrcrtc;
}

int cmyxrandr::setScreenSize(const int &width, const int &height, bool bForce /*= false*/)
{
    int mwidth = DisplayWidth(m_pDpy, 0);
    int mheight = DisplayHeight(m_pDpy, 0);

    if (!bForce)
    {
        if (height > mheight)
            mheight = height;
        if (width > mwidth)
            mwidth = width;
    }
    else
    {
        mwidth = width;
        mheight = height;
    }

    float dpi = (25.4 * DisplayHeight(m_pDpy, 0)) / DisplayHeightMM(m_pDpy, 0);
    int widthMM = (int)((25.4 * mwidth) / dpi);
    int heightMM = (int)((25.4 * mheight) / dpi);

    XRRSetScreenSize(m_pDpy,
                     m_root,
                     mwidth, mheight,
                     widthMM, heightMM);
    XSync(m_pDpy, false);
    return EXIT_SUCCESS;
}

int cmyxrandr::setReflect(Rotation reflection)
{
    int ret = -1;
    Rotation reflect;
    if (m_crtc)
    {
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);
        if (pInfo)
        {
            reflection <<= 4;
            reflect = pInfo->rotation & ~(RR_Reflect_X | RR_Reflect_Y);
            reflect |= reflection;
            ret = setRotate(reflect);
            XRRFreeCrtcInfo(pInfo);
        }
    }

    return ret;
}

int cmyxrandr::setOffset(CMYPOINT offset)
{
    Status ret = -1;
    CMYSIZE screenSize(0, 0);
    CMYSIZE crtcSize(0, 0);

    if (m_crtc)
    {
        //        setPanning(CMYSIZE(0,0));
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);

        screenSize = this->getScreenSize();
        // crtcSize = CMYSIZE (pInfo->width+offset.x(),pInfo->height+offset.y());
        crtcSize.width = pInfo->width + offset.xPos;
        crtcSize.height = pInfo->height + offset.yPos;

        if (crtcSize.width > screenSize.width)
            screenSize.width = crtcSize.width;

        if (crtcSize.height > screenSize.height)
            screenSize.height = crtcSize.height;

        this->setScreenSize(screenSize.width, screenSize.height);

        if (pInfo)
        {
            ret = XRRSetCrtcConfig(m_pDpy,
                                   m_pRes,
                                   m_crtc,
                                   CurrentTime,
                                   offset.xPos,
                                   offset.yPos,
                                   pInfo->mode,
                                   pInfo->rotation,
                                   pInfo->outputs,
                                   pInfo->noutput);

            XSync(m_pDpy, false);
            XRRFreeCrtcInfo(pInfo);
        }
        this->feedScreen();
    }
    return ret;
}

int cmyxrandr::setMode(CMYSIZE size,RRMode rrmode)
{
    int ret = -1;
    if (m_crtc)
    {
        XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);

        if (crtc_info)
        {
            if(rrmode == 0)
            {
                rrmode = getXRRModeInfo(size.width, size.height);
            }
            //RRMode rrmode = getXRRModeInfo(size.width, size.height);
            if (rrmode)
            {

                // disable();
                setScreenSize(size.width, size.height);

                ret = XRRSetCrtcConfig(m_pDpy,
                                       m_pRes,
                                       m_crtc,
                                       CurrentTime,
                                       0, 0,
                                       rrmode,
                                       1,
                                       crtc_info->outputs,
                                       crtc_info->noutput);

                if (ret == RRSetConfigSuccess)
                    setPanning(size);

                XRRFreeCrtcInfo(crtc_info);
            }
            else
                XINFO("Modo width={},height={},{No soportado}", size.width, size.height);
        }
    }
    this->feedScreen();
    XSync(m_pDpy, false);
    return (int)ret;
}

int cmyxrandr::disable()
{
    Status ret = -1;
    ret = XRRSetCrtcConfig(m_pDpy,
                           m_pRes,
                           m_crtc,
                           CurrentTime,
                           0, 0,
                           None,
                           RR_Rotate_0,
                           NULL,
                           0);
    return ret;
}

int cmyxrandr::setRotate(Rotation rotation)
{
    int ret = -1;
    int val = rotation & 0x07;

    if (val != RR_Rotate_0 && val != RR_Rotate_90 && val != RR_Rotate_180 && val != RR_Rotate_270)
    {
        return ret;
    }

    if (m_crtc)
    {
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, m_crtc);

        if (pInfo)
        {
            int height = pInfo->height;
            int width = pInfo->width;

            if ((rotation & 0x0f) == (unsigned short)RR_Rotate_90 || (rotation && 0x0f) == (unsigned short)RR_Rotate_180)
            {
                if (pInfo->width > pInfo->height)
                {
                    width = pInfo->height;
                    height = pInfo->width;
                }
            }
            else if (pInfo->width < pInfo->height)
            {
                width = pInfo->height;
                height = pInfo->width;
            }

            this->setScreenSize(width, height);

            ret = XRRSetCrtcConfig(m_pDpy,
                                   m_pRes,
                                   m_crtc,
                                   CurrentTime,
                                   0,
                                   0,
                                   pInfo->mode,
                                   rotation,
                                   pInfo->outputs,
                                   pInfo->noutput);

            this->setOffset(CMYPOINT(pInfo->x, pInfo->y));
            if (ret != RRSetConfigSuccess)
            {
                XCRITICAL("Error setRotate : rotation = {},return = {}", rotation, ret);
                // qCritical() << "Error setRotate : rotation = " << rotation << "return = " << ret;
            }
            XRRFreeCrtcInfo(pInfo);
        }
    }
    return ret;
}

int cmyxrandr::setPanning(CMYSIZE size)
{
    int ret = -1;
    XRRPanning *panning = NULL;
    panning = XRRGetPanning(m_pDpy, m_pRes, m_crtc);
    panning->left = 0;
    panning->top = 0;
    panning->track_height = size.height;
    panning->track_width = size.width;
    panning->width = size.width;
    panning->height = size.height;

    ret = XRRSetPanning(m_pDpy,
                        m_pRes,
                        m_crtc,
                        panning);

    XRRFreePanning(panning);
    return ret;
}

void cmyxrandr::startEvents()
{
    // m_pNotifier->start();
}

int cmyxrandr::feedScreen()
{
    list<RRCrtc> crtcs;
    crtcs = getCrtcs();
    CMYSIZE size(0, 0);

    // for (int crtc=0; crtc<crtcs.count();crtc++){
    for (list<RRCrtc>::iterator it = crtcs.begin(); it != crtcs.end(); it++)
    {
        // RRCrtc rrcrtc = crtcs[crtc];
        RRCrtc rrcrtc = *it;
        XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, rrcrtc);
        if (pInfo)
        {
            for (int m = 0; m < m_pRes->nmode; m++)
            {
                if (pInfo->mode == m_pRes->modes[m].id)
                {
                    XRRModeInfo *mode = &m_pRes->modes[m];

                    unsigned int height = mode->height + pInfo->y;
                    unsigned int width = mode->width + pInfo->x;

                    if (height > (unsigned int)size.height)
                        size.setHeight(size.height + (height - size.height));

                    if (width > (unsigned int)size.width)
                        size.setWidth(size.width + (width - size.width));
                }
            }
        }
    }
    this->setScreenSize(size.width, size.height, true);
    XINFO("Modo width={},height={},{No soportado}", size.width, size.height);
    return 0;
}

RRMode cmyxrandr::getXRRModeInfo(int width, int height)
{
    RRMode rrmode = 0;
    if (m_output)
    {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo)
        {
            for (int j = 0; j < pOutputInfo->nmode; j++)
            {
                for (int m = 0; m < m_pRes->nmode; m++)
                {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id)
                    {
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        if (mode->width == (unsigned int)width && mode->height == (unsigned int)height)
                        {
                            rrmode = pOutputInfo->modes[j];
                            XRRFreeOutputInfo(pOutputInfo);
                            goto end;
                        }
                    }
                }
            }
            XRRFreeOutputInfo(pOutputInfo);
        }
    }
end:
    return rrmode;
}

XRROutputInfo *cmyxrandr::GetOutputInfo()
{
    return XRRGetOutputInfo(m_pDpy, m_pRes, m_output);
}

list<MyModelInfoEX *> cmyxrandr::getOutputModes()
{
    list<MyModelInfoEX *> modes;

    if (m_output)
    {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo)
        {
            for (int j = 0; j < pOutputInfo->nmode; j++)
            {
                for (int m = 0; m < m_pRes->nmode; m++)
                {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id)
                    {
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        MyModelInfoEX *modex = new MyModelInfoEX(mode);
                        modes.push_back(modex);
                    }
                }
            }
        }
    }
    return modes;
}

CMYSIZE cmyxrandr::getOutputSize()
{
    CMYSIZE size(-1, -1);
    RRMode mode;

    if (m_output)
    {
        XRROutputInfo *pOutputInfo = GetOutputInfo();

        if (pOutputInfo)
        {
            if (pOutputInfo->crtc)
            {
                XRRCrtcInfo *pCrtcInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, pOutputInfo->crtc);
                if (pCrtcInfo)
                {
                    mode = pCrtcInfo->mode;

                    for (int m = 0; m < m_pRes->nmode; m++)
                    {
                        if (mode == m_pRes->modes[m].id)
                        {
                            XRRModeInfo *mode = &m_pRes->modes[m];
                            size = CMYSIZE(mode->width, mode->height);
                            break;
                        }
                    }
                }
            }
        }
        if (pOutputInfo)
            XRRFreeOutputInfo(pOutputInfo);
    }
    return size;
}

bool cmyxrandr::isEnabled()
{
    bool bEnabled = false;
    CMYSIZE size = this->getScreenSize();
    bEnabled = (size.width > 0 && size.width > 0);
    return bEnabled;
}

string cmyxrandr::getName()
{
    string strName;
    if (m_output)
    {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo)
        {
            strName = pOutputInfo->name;
            XRRFreeOutputInfo(pOutputInfo);
        }
    }
    return strName;
}

list<RRCrtc> cmyxrandr::getAllCrtc()
{
    list<RRCrtc> crtcs;
    for (int ncrtc = 0; ncrtc < m_pRes->ncrtc; ncrtc++)
    {
        crtcs.push_back(m_pRes->crtcs[ncrtc]);
    }
    return crtcs;
}

list<CMYSIZE> cmyxrandr::getModes()
{
    list<CMYSIZE> modes;
    if (m_output)
    {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo)
        {
            for (int j = 0; j < pOutputInfo->nmode; j++)
            {
                for (int m = 0; m < m_pRes->nmode; m++)
                {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id)
                    {
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        modes.push_back(CMYSIZE(mode->width, mode->height));
                    }
                }
            }
        }
    }
    return modes;
}

CMYSIZE cmyxrandr::getPreferredMode()
{
    list<CMYSIZE> modes;
    if (m_output)
    {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo)
        {
            for (int j = 0; j < pOutputInfo->nmode; j++)
            {
                for (int m = 0; m < m_pRes->nmode; m++)
                {
                    if (pOutputInfo->modes[j] == m_pRes->modes[m].id)
                    {
                        XRRModeInfo *mode = &m_pRes->modes[m];
                        modes.push_back(CMYSIZE(mode->width, mode->height));
                    }
                }
            }
        }
        // // Atencion!
        // // Esto se ha de probar en monitores con diferentes resoluciones nominales.
        // return modes.at(pOutputInfo->npreferred - 1);
        int n = 0;
        for (list<CMYSIZE>::iterator it = modes.begin(); it != modes.end(); it++, n++)
        {
            if (n == pOutputInfo->npreferred - 1)
            {
                return *it;
            }
        }
    }

    return CMYSIZE(0, 0);
}

bool cmyxrandr::isPrimary()
{
    RROutput rroutput = XRRGetOutputPrimary(m_pDpy, m_root);
    return (rroutput == m_output);
}

void cmyxrandr::setPrimary()
{
    if (!isPrimary())
        XRRSetOutputPrimary(m_pDpy, m_root, m_output);
}

Status cmyxrandr::enable(CMYSIZE size)
{
    Status ret = -1;
    if (m_output)
    {

        RRMode rrmode = getXRRModeInfo(size.width, size.height);
        if (!rrmode)
            return ret;

        list<RRCrtc> crtcs;
        crtcs = getCrtcs();

        // for (int crtc=0; crtc<crtcs.count();crtc++){
        for (list<RRCrtc>::iterator it = crtcs.begin(); it != crtcs.end(); it++)
        {
            RRCrtc rrcrtc = *it;
            XRRCrtcInfo *pInfo = XRRGetCrtcInfo(m_pDpy, m_pRes, rrcrtc);

            for (int poss = 0; poss < pInfo->npossible; poss++)
            {
                RROutput rr_output = pInfo->possible[poss];

                if (pInfo->noutput == 0 && pInfo->possible[poss] == m_output)
                {

                    // XRROutputInfo *pOutInfo = XRRGetOutputInfo(m_pDpy,m_pRes,rr_output);
                    // qDebug() << QString(pOutInfo->name);

                    RROutput *rr_outputs;
                    rr_outputs = (RROutput *)calloc(1, sizeof(RROutput));
                    rr_outputs[0] = rr_output;

                    setScreenSize(size.width, size.height);

                    ret = XRRSetCrtcConfig(m_pDpy,
                                           m_pRes,
                                           rrcrtc,
                                           CurrentTime,
                                           pInfo->x,
                                           pInfo->y,
                                           rrmode,
                                           RR_Rotate_0,
                                           rr_outputs,
                                           1);
                    return ret;
                }
            }
        }
    }
    return ret;
}

bool cmyxrandr::isConnected()
{
    bool bConnected = false;
    if (m_output)
    {
        XRROutputInfo *pOutputInfo = GetOutputInfo();
        if (pOutputInfo)
        {
            bConnected = (pOutputInfo->connection == 0) ? true : false;
            XRRFreeOutputInfo(pOutputInfo);
        }
    }
    return bConnected;
}

list<RROutput> cmyxrandr::getOutputs()
{
    list<RROutput> outputs;
    for (int nout = 0; nout < m_pRes->noutput; nout++)
    {
        outputs.push_back(m_pRes->outputs[nout]);
    }
    return outputs;
}

list<RRCrtc> cmyxrandr::getCrtcs()
{
    list<RRCrtc> crtcs;
    for (int ncrtc = 0; ncrtc < m_pRes->ncrtc; ncrtc++)
    {
        crtcs.push_back(m_pRes->crtcs[ncrtc]);
    }
    return crtcs;
}

RROutput cmyxrandr::getOutputByName(string strName)
{
    RROutput output = 0;

    for (int nout = 0; nout < m_pRes->noutput; nout++)
    {
        XRROutputInfo *pInfo = XRRGetOutputInfo(m_pDpy, m_pRes, m_pRes->outputs[nout]);
        if (strName == pInfo->name)
        {
            output = m_pRes->outputs[nout];
            XRRFreeOutputInfo(pInfo);
            break;
        }
        XRRFreeOutputInfo(pInfo);
    }
    return output;
}

XRRMonitorInfo *cmyxrandr::getScreenInfo()
{
    // Display *display;
    int major, minor;

    // display = XOpenDisplay(NULL);
    // Window window = DefaultRootWindow(display);

    int nRet = XRRQueryVersion(m_pDpy, &major, &minor);
    XINFO("xrandr version:{}.{}", major, minor);
    if (m_psConfig == NULL)
        m_psConfig = XRRGetScreenInfo(m_pDpy, m_root);

    int monitors = 0;
    XRRMonitorInfo *info = XRRGetMonitors(m_pDpy, m_root, True, &monitors);
    for (int i = 0; i < monitors; i++)
    {
        XID id;
        RROutput *out = info[i].outputs;
        memcpy(&id, out, sizeof(RROutput));
        // XRRMonitorInfo * m = new XRRMonitorInfo;
        // memcpy(m,&info[i],sizeof(XRRMonitorInfo));
        // m_mMonitors.insert(make_pair(id,m));

        XINFO("XRRMonitorInfo:{}{}, {}{}, {}{}, {}{}, {}{}, {}{}, {}{}, {}{}, {}{}", "index:", i,
              " primary:", info[i].primary,
              " noutput:", info[i].noutput,
              " x:", info[i].x,
              " y:", info[i].y,
              " width:", info[i].width,
              " height:", info[i].height,
              " widthmm:", info[i].mwidth,
              " heightm:", info[i].mheight,
              " outputs:", id);
    }

    // XRRFreeScreenConfigInfo(psConfig);
    XRRFreeMonitors(info);
    // XFree(display);
}

XRRScreenSize *cmyxrandr::getCurrentConfigSizes()
{
    if (m_psConfig == NULL)
        m_psConfig = XRRGetScreenInfo(m_pDpy, m_root);
    // int major, minor;
    // int nRet = XRRQueryVersion(m_pDpy,&major, &minor);
    // XINFO("xrandr version:{}.{}",major,minor);
    // XRRScreenConfiguration * psConfig = XRRGetScreenInfo (m_pDpy,m_root);

    int nsizes = 0;
    XRRScreenSize *pssz = XRRConfigSizes(m_psConfig, &nsizes);
    XINFO("XRRScreenSize nsizes:{},all-width:{},all-height:{},all-widthmm:{},all-heightm:{}", nsizes, pssz->width, pssz->height, pssz->mwidth, pssz->mheight);

    return pssz;
}

unsigned short cmyxrandr::getCurrentConfigRotation()
{
    if (m_psConfig == NULL)
        m_psConfig = XRRGetScreenInfo(m_pDpy, m_root);

    Rotation current_rotation;
    SizeID sid = XRRConfigCurrentConfiguration(m_psConfig, &current_rotation);
    XINFO("rotation:{}", current_rotation);
    return current_rotation;
}

short cmyxrandr::getAllScreenInfoEx(vector<MOutputInfo> & vOutputInfo,CMYSIZE & currentSize,CMYSIZE & maxSize)
{
    CMDEXEC::CmdRes res;
    bool bret = CMDEXEC::Execute("xrandr --verbose",res);
    printf("%s\n",res.StdoutString.c_str());
    vector<string> vString;
    CMDEXEC::Stringsplit(res.StdoutString,'\n',vString);
    // vector<string> screens;
    // vector<string> devices;
    // vector<string> items;
    // vector<string> vItems;
    // vector<string> hItems;

    vector<string> array[5][50][64][5];
    //std::vector<std::vector<std::vector<std::string>>> array;
    int current_w = 0,current_h = 0,maximum_w = 0,maximum_h = 0;

    for (size_t i = 0,j = 0,l = 0,m = 0; i < vString.size(); i++)
    {
        if(vString[i].find("\tIdentifier") != string::npos)
        {
            string str(vString[i].c_str());
            str = CMDEXEC::Strip(str);  
            str = CMDEXEC::Lstrip(str,'\t'); 
            array[1][j][l-1][m-1][0].append(" ");
            array[1][j][l-1][m-1][0].append(str.c_str());            
            continue;
        }
        else if(CMDEXEC::StartsWith(vString[i].c_str(), "\t"))
            continue;
        if(CMDEXEC::StartsWith(vString[i].c_str(), "Screen "))
        {
            string strScreenLine = vString[i].c_str();
            vector<string> vScreenLineTmp = CMDEXEC::Split(strScreenLine,',');
            if(vScreenLineTmp.size() < 3)
            {
                return -1;
            }

            vector<string> vScreenLineTmp2 = CMDEXEC::Split(vScreenLineTmp[1],' ');
            current_w = atoi(vScreenLineTmp2[1].c_str());
            current_h = atoi(vScreenLineTmp2[3].c_str());
            vector<string> vScreenLineTmp3 = CMDEXEC::Split(vScreenLineTmp[2],' ');
            maximum_w = atoi(vScreenLineTmp3[1].c_str());
            maximum_h = atoi(vScreenLineTmp3[3].c_str());
            currentSize.width = current_w;
            currentSize.height = current_h;
            maxSize.width = maximum_w;
            maxSize.height = maximum_h;

            //screens.push_back(vString[i].c_str());            
            array[0][0][0][m].push_back(vString[i].c_str());
            m++;
            j = 0;
            l = 0;
        }
        else if(CMDEXEC::StartsWith(vString[i].c_str(), "  "))
        {
            string str(vString[i].c_str());
            str = CMDEXEC::Strip(str);            
            //items.push_back(str);
            array[2][j][l-1][m-1].push_back(str.c_str());

            string strTempH(vString[i+1].c_str());
            strTempH = CMDEXEC::Lstrip(strTempH);
            bool bbrh = CMDEXEC::StartsWith(strTempH, "h:");
            string strTempV(vString[i+2].c_str());
            strTempV = CMDEXEC::Lstrip(strTempV);
            bool bbrv = CMDEXEC::StartsWith(strTempV, "v:");
            if(bbrh)
            {
                //hItems.push_back(strTempH);
                i++;
                array[3][j][l-1][m-1].push_back(strTempH.c_str());
                //j++;

            }
            if(bbrv)
            {
                //vItems.push_back(strTempV);
                i++;
                array[4][j][l-1][m-1].push_back(strTempV.c_str());
                //j++;
            }
            j++;
        }
        else
        {
            j = 0;
            //devices.push_back(vString[i].c_str());
            array[1][j][l][m-1].push_back(vString[i].c_str());//[hv][name][device][screen]
            l++;
                       
        }
    }

    for (size_t m = 0; m < 5; m++)//[hv][name][device][screen]
    {
        for (size_t l = 0; l < 64; l++)
        {
            if(array[1][0][l][m].size() == 0) continue;
            MOutputInfo testl;
            string strTmpLong(array[1][0][l][m][0]);
            
            string strTmp = strTmpLong;
            if(strTmpLong.find("Identifier") != string::npos)
            {   int pos = strTmpLong.find("Identifier");
                strTmp = strTmpLong.substr(0, pos);
                string strOutputid = strTmpLong.substr(pos,strTmpLong.length() - 1);

                vector<string> vOutputid = CMDEXEC::Split(strOutputid.c_str(),": ");
                testl.outputId = std::stoi(vOutputid[1], nullptr, 16);
                
            }

            strTmp = CMDEXEC::replaceAll(strTmp,"unknown connection","unknown-connection");

            vector<string> vItemsTmp = CMDEXEC::Split(strTmp.c_str()," ");
            string output = vItemsTmp[0];
            testl.name = output;

            if (CMDEXEC::StartsWith(vItemsTmp[1], "connected"))
                testl.connected = 1;
            else if (CMDEXEC::StartsWith(vItemsTmp[1], "disconnected"))
                testl.connected = 2;
            else if (CMDEXEC::StartsWith(vItemsTmp[1], "unknown-connection"))
                testl.connected = 3;
            else
                testl.connected = 0;
            
            if(testl.connected != 1)
                continue;

            testl.primary = false;

            for (int i = 0; i < vItemsTmp.size(); i++)
            {
                if (vItemsTmp[i].find("primary") != string::npos)
                {
                    testl.primary = true;
                    vItemsTmp.erase(vItemsTmp.begin() + i);
                    break;
                }
            }

            if (!CMDEXEC::StartsWith(vItemsTmp[2], "("))
            {
                testl.geometry = vItemsTmp[2].c_str();

                vector<string> vTmp = CMDEXEC::Split(vItemsTmp[2].c_str(),'+');
                vector<string> vTmpt = CMDEXEC::Split(vTmp[0].c_str(),'x');

                testl.pos.xPos = atoi(vTmp[1].c_str());
                testl.pos.yPos = atoi(vTmp[2].c_str());
                testl.size.width = atoi(vTmpt[0].c_str());
                testl.size.height = atoi(vTmpt[1].c_str());

                //string outputId = vItemsTmp[3];
                //outputId = CMDEXEC::StripBrackets(outputId,'(',')'); 
                //testl.outputId = std::stoi(outputId, nullptr, 16);


                if (CMDEXEC::StartsWith(vItemsTmp[4], "normal"))
                    testl.current_rotation = 1;
                else if (CMDEXEC::StartsWith(vItemsTmp[4], "right"))
                    testl.current_rotation = 2;
                else if (CMDEXEC::StartsWith(vItemsTmp[4], "inverted"))
                    testl.current_rotation = 3;
                else if (CMDEXEC::StartsWith(vItemsTmp[4], "left"))
                    testl.current_rotation = 4;
                else
                    testl.current_rotation = 1;
                
                
                //vector<string> vTmpmm = CMDEXEC::Split(vItemsTmp[3].c_str(),'x');
                string mmW = vItemsTmp[vItemsTmp.size()-3].c_str();
                string mmH = vItemsTmp[vItemsTmp.size()-1].c_str();
                mmW = CMDEXEC::replaceAll(mmW,"mm"," ");
                mmH = CMDEXEC::replaceAll(mmH,"mm"," ");
                testl.mmsize.width = atoi(mmW.c_str());
                testl.mmsize.height = atoi(mmH.c_str());
                
            }


            for (size_t j = 0; j < 50; j++)
            {
                if(array[2][j][l][m].size() == 0) 
                    continue;

                MyModelInfoEX mode;
                string strResolution(array[2][j][l][m][0]);
                if(strResolution.empty()) continue;
                bool bIsCurrent = false,bIsPreferred = false;
                if(strResolution.find("*current") !=string::npos) bIsCurrent = true;
                if(strResolution.find("+preferred") !=string::npos) bIsPreferred = true;
                if(strResolution.find("Interlace") !=string::npos) mode.interlace = true;

                vector<string> vItemsTmpT = CMDEXEC::Split(strResolution.c_str()," ");
                mode.name = vItemsTmpT[0].c_str();
                string modeId = vItemsTmpT[1];
                modeId = CMDEXEC::StripBrackets(modeId,'(',')'); 
                mode.id = std::stoi(modeId, nullptr, 16);
                mode.hSync = vItemsTmpT[3].c_str();
                mode.vSync = vItemsTmpT[4].c_str();

                vector<string> vWH = CMDEXEC::Split(vItemsTmpT[0].c_str(),'x');
                mode.width = atoi(vWH[0].c_str());
                mode.height = atoi(vWH[1].c_str());

                //string strResolutionH(array[3][j][l][m][0]);                
                string strResolutionV(array[4][j][l][m][0]);//v:
                vector<string> vItemsTmpV = CMDEXEC::Split(strResolutionV.c_str()," ");
                string strRate = vItemsTmpV[vItemsTmpV.size()-1];
                strRate = CMDEXEC::Rstrip(strRate,'z');
                strRate = CMDEXEC::Rstrip(strRate,'H');
                mode.rate = strRate;
                XINFO("modeId={},{}_{}",modeId,mode.name,mode.rate);
                //float fRate = atof(strRate.c_str());
                //XINFO("nRate:{:.2f}",fRate);
                             
                
                //
                if(bIsCurrent)
                {
                    testl.currentMode = mode;         
                  
                }
                testl.modes.push_back(mode);                
                
            }
            vOutputInfo.push_back(testl);
        }
    }

    return 0;


    // MONITORINFO test;
    // for(int i = 0;i<devices.size();i++)
    // {
    //     string strTmp(devices[i].c_str());
    //     strTmp = CMDEXEC::replaceAll(strTmp,"unknown connection","unknown-connection");
    //     devices[i] = strTmp.c_str();
    //     vector<string> vItemsTmp = CMDEXEC::Split(devices[i].c_str()," ");
    //     string output = vItemsTmp[0];
    //     test.name = output;
    //     if(CMDEXEC::StartsWith(vItemsTmp[1],"connected"))
    //         test.connected = 1;
    //     else if(CMDEXEC::StartsWith(vItemsTmp[1],"disconnected"))
    //         test.connected = 2;
    //     else if(CMDEXEC::StartsWith(vItemsTmp[1],"unknown-connection"))
    //         test.connected = 3;
    //     else
    //         test.connected = 0;
    //     test.primary = false;

    //     for(int i = 0;i < vItemsTmp.size(); i++)
    //     {
    //         if (vItemsTmp[i].find("primary") != string::npos)
    //         {
    //             test.primary = true;
    //             vItemsTmp.erase(vItemsTmp.begin() + i);
    //             break;
    //         }
    //     }

    //     if(!CMDEXEC::StartsWith(vItemsTmp[2],"("))
    //     {            
    //         test.geometry = vItemsTmp[2].c_str();

    //         if (CMDEXEC::StartsWith(vItemsTmp[4], "normal"))
    //             test.current_rotation = 1;
    //         else if (CMDEXEC::StartsWith(vItemsTmp[4], "right"))
    //             test.connected = 2;
    //         else if (CMDEXEC::StartsWith(vItemsTmp[4], "inverted"))
    //             test.connected = 3;
    //         else if (CMDEXEC::StartsWith(vItemsTmp[4], "left"))
    //             test.connected = 4;
    //         else 
    //             test.connected = 1;
    //     }
    // }

    //return 0;
}


int cmyxrandr::getScreenSizeRange(CMYSIZE & min,CMYSIZE & max)
{
    int minWidth = 0, minHeight = 0, maxWidth = 0, maxHeight = 0;

    Status state = XRRGetScreenSizeRange(m_pDpy, m_root, &minWidth,&minHeight,&maxWidth,&maxHeight);
    min.width = minWidth;
    min.height = minHeight;
    max.width = maxWidth;
    max.height = maxHeight;
    XINFO("state:{},minWidth:{},minHeight:{},maxWidth:{},maxHeight:{}", state,minWidth,minHeight,maxWidth,maxHeight);
    return state;
}