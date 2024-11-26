//#include "dlgManager.h"
#include "cdataProcess.h"
#include <sys/utsname.h>
#include <sys/statfs.h>
#include <QFile>


cdataProcess* cdataProcess::m_instance = NULL;

cdataProcess* cdataProcess::GetInstance()
{
    if (m_instance == NULL )
    {        
        m_instance = new cdataProcess();
        //m_instance->OnUpdate();
        //m_instance->Init();
    }

    return m_instance;
}

void * cdataProcess::workerThreadListen(void * p)
{
    // cdataProcess * pcdataProcess = (cdataProcess *)p;
    // while (1)
    // {

    // }

}

cdataProcess::cdataProcess(/* args */)
{
    m_nWidth = 0,m_nHight = 0;
    m_layout_vertical = 0,m_layout_horizontal = 0;

    QFile file("./config.ini");
    if (!file.exists())
    {
        QSettings config("config.ini",QSettings::IniFormat);
        config.setValue("screen/width",1920);
        config.setValue("screen/height",1080);
        config.setValue("screen/layout_horizontal",1);
        config.setValue("screen/layout_vertical",1);
        
        XCRITICAL("config.ini不存在");
        XCRITICAL("请先配置config.ini文件,配置文件在程序目录下");
        exit(0);
    }
    file.close();
    QSettings settings("config.ini",QSettings::IniFormat);
    m_nWidth = settings.value("screen/width",0).toInt();
    m_nHight = settings.value("screen/height",0).toInt();
    m_layout_horizontal = settings.value("screen/layout_horizontal",0).toInt();
    m_layout_vertical = settings.value("screen/layout_vertical",0).toInt();
    m_allLayouts = settings.value("screen/allResolution","").toString().toStdString();

    if(m_nWidth == 0)
    {
        m_nWidth = 1920;
        m_nHight = 1080;
    }
  
    if(m_layout_vertical ==  0 || m_layout_horizontal == 0)
    {
        XERROR("请先在config.ini中配置layout_horizontal和layout_vertical");
        exit(0);
    }

    MAINOUTPUTSUPPORTMODESTR mode1024,mode1050,mode1080,mode2160,mode4k;

    mode1024.outputModeName = "1280x1024";
    mode1050.outputModeName = "1680x1050";
    mode1080.outputModeName = "1920x1080";
    mode2160.outputModeName = "3840x2160";
    mode4k.outputModeName = "4096x2160";

    m_supportModes.push_back(mode4k);
    m_supportModes.push_back(mode2160);
    m_supportModes.push_back(mode1080);
    m_supportModes.push_back(mode1050);
    //m_supportModes.push_back(mode1024);

}

cdataProcess::~cdataProcess()
{
}

void cdataProcess::SetMainWindow(MainWindow * p)
{
    m_pMainWindow = p;
}

void cdataProcess::print_display_name(Display *dpy, int target_id, int attr,char *name,string & displayName)
{
    Bool ret;
    char *str;

    ret = XNVCTRLQueryTargetStringAttribute(dpy,
                                            NV_CTRL_TARGET_TYPE_DISPLAY,
                                            target_id, 0,
                                            attr,
                                            &str);
    if (!ret) {
        printf("    %18s : N/A\n", name);
        return;
    }

    printf("    %18s : %s\n", name, str);
    displayName = str;
    XFree(str);
}

int cdataProcess::GetNvXScreen(Display *dpy)
{
    int defaultScreen, screen;

    defaultScreen = DefaultScreen(dpy);

    if (XNVCTRLIsNvScreen(dpy, defaultScreen)) {
        return defaultScreen;
    }

    for (screen = 0; screen < ScreenCount(dpy); screen++) {
        if (XNVCTRLIsNvScreen(dpy, screen)) {
            printf("Default X screen %d is not an NVIDIA X screen.  "
                   "Using X screen %d instead.\n",
                   defaultScreen, screen);
            return screen;
        }
    }

    fprintf(stderr, "Unable to find any NVIDIA X screens; aborting.\n");

    exit(1);
}

void cdataProcess::print_display_id_and_name(Display *dpy, int target_id, const char *tab)
{
    char name_str[64];
    int len;

    len = snprintf(name_str, sizeof(name_str), "%sDP-%d", tab, target_id);

    if ((len < 0) || (len >= sizeof(name_str))) {
        return;
    }
    string strDisplayName;
    print_display_name(dpy, target_id, NV_CTRL_STRING_DISPLAY_DEVICE_NAME,name_str,strDisplayName);
}


bool cdataProcess::GetMonitorsInfo(string & strInfo)
{
    json js;
    //string strDisplayName = ":0";
    //cmyxrandr cxr(strDisplayName);
    //XRRScreenSize * psize = cxr.getCurrentConfigSizes();
    cmyxrandr* pcmxrandr =  cmyxrandr::GetInstance();
    XRRScreenSize * psize = pcmxrandr->getCurrentConfigSizes();

    CMYSIZE min,max;
    //cxr.getScreenSizeRange(min,max);
    pcmxrandr->getScreenSizeRange(min,max);

    //cxr.getScreenInfo();
    //short rate = cxr.getScreenRate();

    unsigned short	rotation = pcmxrandr->getCurrentConfigRotation();
   
    js["width"] = psize->width;
    js["height"] = psize->height;
    js["mwidth"] = psize->mwidth;
    js["mheight"] = psize->mheight;
    js["rotation"] = rotation;
    //js["rate"] = rate;
    js["maxWidth"] = max.width;
    js["maxHeight"] = max.height;
    js["output_width"] = m_nWidth;
    js["output_height"] = m_nHight;
    

    json jsdata;
    int nNum = 0;
    list<RROutput> outputs = pcmxrandr->getOutputs();
    

    for (list<RRCrtc>::iterator it = outputs.begin(); it != outputs.end(); it++)   
    {
        RROutput output = *it;
        pcmxrandr->setOutPut(output);
        // cmyxrandr rroutput(strDisplayName, output);
        XRROutputInfo *outinfo = pcmxrandr->GetOutputInfo();
        pcmxrandr->setOutPutName(outinfo->name);

        

        if (outinfo->nmode > 0)
        {
            nNum++;
            pcmxrandr->setCrtc(outinfo->crtc);
            XRRCrtcInfo *rcrtinfo = pcmxrandr->getCrtcInfo();
            json node;
            node["rroutputId"] = output;
            node["name"] = outinfo->name;
            node["xPos"] = rcrtinfo->x;
            node["yPos"] = rcrtinfo->y;
            node["width"] = rcrtinfo->width;
            node["height"] = rcrtinfo->height;
            node["rotation"] = rcrtinfo->rotation;
            node["mm_width"] = outinfo->mm_width;
            node["mm_height"] = outinfo->mm_height;

            node["xVirtual"] = rcrtinfo->x / m_nWidth;
            node["yVirtual"] = rcrtinfo->y / m_nHight;

            XRRFreeCrtcInfo(rcrtinfo);

            string strStatus = outinfo->connection ? "Not connected":"Connected";
            //string strPrimary = cxr.isPrimary() ? "Primary" : "";
            // XINFO("{},{},{}",cxr.getName(),strStatus ,strPrimary);
            node["primary"] = pcmxrandr->isPrimary();


            if (strStatus == "Connected")
            {
                node["connected"] = true;
                json jmodes;
                list<MyModelInfoEX *> modes = pcmxrandr->getOutputModes();
                for (list<MyModelInfoEX *>::iterator it = modes.begin(); it != modes.end(); it++)
                {
                    json mode;
                    XINFO("w={},h={}", (*it)->width, (*it)->height);
                    float rate = (float)(*it)->dotClock / (*it)->hTotal / (*it)->vTotal;
                    if ((*it)->modeFlags & 0x10)
                    {
                        mode["interlace"] = true;
                        rate *= 2;
                    }
                    mode["modeId"] = (*it)->id;
                    mode["name"] = (*it)->name;
                    mode["width"] = (*it)->width;
                    mode["height"] = (*it)->height;
                    char buffer[10] ={0};
                    sprintf(buffer,"%.2f",rate);
                    mode["rate"] = buffer;
                    jmodes.push_back(mode);                        

                    XINFO("modeId={},name={},width:{},height:{},dotClock:{},hSyncStart:{},hSyncEnd:{},hTotal:{},hSkew:{},\
                vSyncStart:{},vSyncEnd:{},vTotal:{},nameLength:{},modeFlags:{},rate=dotClock/hTotal/vTotal {:.2f}",
                          (*it)->id, (*it)->name, (*it)->width, (*it)->height, (*it)->dotClock, (*it)->hSyncStart,
                          (*it)->hSyncEnd, (*it)->hTotal, (*it)->hSkew, (*it)->vSyncStart, (*it)->vSyncEnd, (*it)->vTotal,
                          (*it)->nameLength, (*it)->modeFlags, rate);
                    
                    
                }

                node["data"]=jmodes;


                for (list<MyModelInfoEX *>::iterator it = modes.begin(); it != modes.end(); it++)
                {
                    delete (*it);
                }
            }
            jsdata.push_back(node);
            XRRFreeOutputInfo(outinfo);
        }
    }
    js["output"] = jsdata;
    js["num"] = nNum;
    strInfo = js.dump().c_str();
    XINFO("{}",js.dump().c_str());

    return true;
}

bool cdataProcess::GetMonitorsInfo_shell(json & js)
{
    //json js;
    //string strDisplayName = ":0";
    //cmyxrandr cxr(strDisplayName);
    //XRRScreenSize * psize = cxr.getCurrentConfigSizes();
    cmyxrandr* pcmxrandr =  cmyxrandr::GetInstance();
    CMYSIZE currentSize, maxSize;
    vector<MOutputInfo> vOutputInfo;  
    //short shRet = pcmxrandr->getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
    //short shRet = pcmxrandr->getAllScreenInfoNew(vOutputInfo,currentSize,maxSize);
    short shRet = pcmxrandr->getAllScreenInfoXrandr(vOutputInfo,currentSize,maxSize);
    if(shRet == 0)
    {
        js["width"] = currentSize.width;   // vOutputInfo[i].size.width;
        js["height"] = currentSize.height; // vOutputInfo[i].size.height;
        js["mwidth"] = 0;
        js["mheight"] = 0;
        js["rotation"] = 0;
        // js["rate"] = rate;
        js["maxWidth"] = maxSize.width;
        js["maxHeight"] = maxSize.height;
        js["output_width"] = m_nWidth;
        js["output_height"] = m_nHight;
        js["horizontal"] = m_layout_horizontal;
        js["vertial"] = m_layout_vertical;

        json jsdata;
        int nNum = vOutputInfo.size();
        if(nNum <= 0)
        {
            js.clear();
            //strInfo = "";
            return false;
        }
        

        for (size_t i = 0; i < vOutputInfo.size(); i++)
        {
            json node;
            node["rroutputId"] = vOutputInfo[i].outputId;
            node["name"] = vOutputInfo[i].name;
            node["xPos"] = vOutputInfo[i].pos.xPos;
            node["yPos"] = vOutputInfo[i].pos.yPos;
            node["width"] = vOutputInfo[i].size.width;
            node["height"] = vOutputInfo[i].size.height;
            node["rotation"] = vOutputInfo[i].current_rotation;
            node["mm_width"] = vOutputInfo[i].mmsize.width;
            node["mm_height"] = vOutputInfo[i].mmsize.height;

            node["xVirtual"] = (vOutputInfo[i].pos.xPos / m_nWidth);
            node["yVirtual"] = (vOutputInfo[i].pos.yPos / m_nHight);
            node["coordinateOrderX"] = (vOutputInfo[i].pos.xPos / m_nWidth);
            node["coordinateOrderY"] = (vOutputInfo[i].pos.yPos / m_nHight);
            node["primary"] = vOutputInfo[i].primary;
            node["currentModeId"] = vOutputInfo[i].currentMode.id;
            node["preferredModeId"] = vOutputInfo[i].preferredMode.id;

            if(vOutputInfo[i].connected)
            {
                node["connected"] = true;
                json jmodes;

                for (size_t j = 0; j < vOutputInfo[i].modes.size(); j++)
                {
                    MyModelInfoEX & modex = vOutputInfo[i].modes[j];
                    json mode;                    
                    mode["interlace"] = modex.interlace;
                    mode["modeId"] = modex.id;
                    mode["name"] = modex.name;
                    mode["width"] = modex.width;
                    mode["height"] = modex.height;
                    //char buffer[10] = {0};
                    //sprintf(buffer, "%.2f", modex.rate);
                    //mode["rate"] = buffer;
                    mode["rate"] = modex.rate;
                    mode["hSync"] = modex.hSync;
                    mode["vSync"] = modex.vSync;

                    jmodes.push_back(mode);

                    XINFO("modeId={},name={},width:{},height:{},dotClock:{},hSyncStart:{},hSyncEnd:{},hTotal:{},hSkew:{},\
                vSyncStart:{},vSyncEnd:{},vTotal:{},nameLength:{},modeFlags:{},rate={:.2f}",
                          modex.id, modex.name, modex.width, modex.height, modex.dotClock, modex.hSyncStart,
                          modex.hSyncEnd, modex.hTotal, modex.hSkew, modex.vSyncStart, modex.vSyncEnd, modex.vTotal,
                          modex.nameLength, modex.modeFlags, modex.rate);
                }
                node["data"]=jmodes;
            }
            jsdata.push_back(node);
        }

        if(vOutputInfo.size() < m_layout_vertical * m_layout_horizontal)
        {
            for (size_t i = vOutputInfo.size(); i < m_layout_vertical * m_layout_horizontal; i++)
            {
                json node;
                node["rroutputId"] = 0;
                node["name"] = "";
                node["xPos"] = 0;
                node["yPos"] = 0;
                node["width"] = 0;
                node["height"] = 0;
                node["rotation"] = 0;
                node["mm_width"] = 0;
                node["mm_height"] = 0;

                node["xVirtual"] = 0;
                node["yVirtual"] = 0;
                node["primary"] = false;
                node["currentModeId"] = 0;
                node["preferredModeId"] = 0;
                node["connected"] = false;
                node["data"]="";
                jsdata.push_back(node);
            }
        }

        js["output"] = jsdata;
        js["num"] = nNum;
        //strInfo = js.dump().c_str();
        XINFO("{}",js.dump().c_str());
        return true;
    }

    return false;

}

// bool cdataProcess::GetOutputsInfo()
// {
//     // string strDisplayName = ":0";
//     // cmyxrandr cxr(strDisplayName);
//     cmyxrandr* pcmxrandr =  cmyxrandr::GetInstance();
//     CMYSIZE currentSize, maxSize;   
//     short shRet = pcmxrandr->getAllScreenInfoEx(m_vOutputInfo,currentSize,maxSize);
//     if(shRet == 0)
//         return true;
//     else
//         return false;
// }

int cdataProcess::GetOutputsInfo_shell(json & js)
{
    cmyxrandr* pcmxrandr =  cmyxrandr::GetInstance();
    // string strDisplayName = ":0";
    // cmyxrandr cxr(strDisplayName);
    //XRRScreenSize * psize = cxr.getCurrentConfigSizes();
    CMYSIZE currentSize, maxSize;
    vector<MOutputInfo> vOutputInfo;  
    //short shRet = pcmxrandr->getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
    //short shRet = pcmxrandr->getAllScreenInfoNew(vOutputInfo,currentSize,maxSize);
    short shRet = pcmxrandr->getAllScreenInfoXrandr(vOutputInfo,currentSize,maxSize);
    if(shRet == 0)
    {
        /*
        QSettings settings("config.ini", QSettings::IniFormat);
        m_nWidth = settings.value("screen/width", 0).toInt();
        m_nHight = settings.value("screen/height", 0).toInt();
        m_layout_horizontal = settings.value("screen/layout_horizontal", 0).toInt();
        m_layout_vertical = settings.value("screen/layout_vertical", 0).toInt();
        m_allLayouts = settings.value("screen/allResolution", "").toString().toStdString();

        if (m_nWidth == 0)
        {
            m_nWidth = 1920;
            m_nHight = 1080;
        }
        */
        if(currentSize.width != m_nWidth || currentSize.height != m_nHight)
        {
            return -1;
        }

        int sortBuf[m_layout_horizontal * m_layout_vertical] = {0};
        char bufferLayout[40] = {0};
        sprintf(bufferLayout, "%dx%d", m_layout_horizontal,m_layout_vertical);
        string layoutName = bufferLayout;
        js["layoutName"] = layoutName;
        char buffer[40] = {0};
        sprintf(buffer, "%dx%d", m_nWidth, m_nHight);
        string resolution = buffer;
        js["resolution"] = resolution;
        js["allResolution"] = m_allLayouts;
        js["layout_horizontal"] = m_layout_horizontal;
        
        js["num"] = vOutputInfo.size();
        json jsdata;
        int nNum = vOutputInfo.size();
        if(nNum <= 0)
        {
            js.clear();
            //strInfo = "";
            return 0;
        }
        // if(m_layout_horizontal == 1 && nNum != m_layout_vertical)
        // {
        //     m_layout_vertical = nNum;
        // }
        js["layout_vertical"] = m_layout_vertical;

        for (size_t i = 0; i < vOutputInfo.size(); i++)
        {
            json node;
            node["name"] = vOutputInfo[i].name;
            node["coordinateOrderX"] = (vOutputInfo[i].pos.xPos / m_nWidth);
            node["coordinateOrderY"] = (vOutputInfo[i].pos.yPos / m_nHight);
            int id = (vOutputInfo[i].pos.xPos / m_nWidth) + (vOutputInfo[i].pos.yPos / m_nHight) * m_layout_vertical;
            sortBuf[id] = 1;
            node["id"] = id;
            node["primary"] = vOutputInfo[i].primary;
            if(vOutputInfo[i].connected)
            {
                node["connected"] = true;                
            }
            jsdata.push_back(node);
        }

        if(vOutputInfo.size() < m_layout_vertical * m_layout_horizontal)
        {
            for (size_t i = vOutputInfo.size(); i < m_layout_vertical * m_layout_horizontal; i++)
            {
                json node;             
                node["name"] = "";
                for (size_t i = 0; i < sizeof(sortBuf)/sizeof(int); i++)
                {
                    if(sortBuf[i] == 0)
                    {
                        node["id"] = i;
                        sortBuf[i] = 1;
                        break;
                    }
                }               
                node["coordinateOrderX"] = 0;
                node["coordinateOrderY"] = 0;
                node["primary"] = false;
                jsdata.push_back(node);
            }
        }

        js["layout"] = jsdata;
        //strInfo = js.dump().c_str();
        XINFO("{}",js.dump().c_str());
        return 0;
    }

    return -1;

}
bool cdataProcess::InitMainOutputModes()
{
    try
    {

        cmyxrandr *pcmxrandr = cmyxrandr::GetInstance();
        // string strDisplayName = ":0";
        // cmyxrandr cxr(strDisplayName);
        // XRRScreenSize * psize = cxr.getCurrentConfigSizes();
        CMYSIZE currentSize, maxSize;
        vector<MOutputInfo> vOutputInfo;
        //short shRet = pcmxrandr->getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
        //short shRet = pcmxrandr->getAllScreenInfoNew(vOutputInfo, currentSize, maxSize);
        short shRet = pcmxrandr->getAllScreenInfoXrandr(vOutputInfo,currentSize,maxSize);
        if (shRet == 0)
        {
            string strLastModeName = "";
            for (size_t m = 0; m < m_supportModes.size(); m++)
            {
                m_supportModes[m].bIsFound = false;
            }

            for (size_t i = 0; i < vOutputInfo.size(); i++)
            {
                if (vOutputInfo[i].primary)
                {
                    for (size_t j = 0; j < vOutputInfo[i].modes.size(); j++)
                    {
                        if (strLastModeName.compare(vOutputInfo[i].modes[j].name) != 0)
                        {
                            for (size_t m = 0; m < m_supportModes.size(); m++)
                            {
                                if (vOutputInfo[i].modes[j].name.compare(m_supportModes[m].outputModeName) == 0 && m_supportModes[m].bIsFound == false)
                                {
                                    m_supportModes[m].bIsFound = true;
                                    MAINOUTPUTSUPPORTMODESTR mode;
                                    mode.outputModeName = vOutputInfo[i].modes[j].name;
                                    mode.width = vOutputInfo[i].modes[j].width;
                                    mode.height = vOutputInfo[i].modes[j].height;
                                    m_InitSupportModes.push_back(mode);

                                    
                                    // json node;
                                    // node["resolution"] = vOutputInfo[i].modes[j].name;
                                    // node["width"] = vOutputInfo[i].modes[j].width;
                                    // node["height"] = vOutputInfo[i].modes[j].height;
                                    // strLastModeName = vOutputInfo[i].modes[j].name;
                                    // js.push_back(node);
                                    // XINFO("{}",js.dump().c_str());
                                    break;
                                }
                            }
                        }
                        else
                            continue;
                    }
                }
            }
        }
        else
        {
            return false;
        }
        XINFO("InitMainOutputModes finished!");
        return true;
    }
    catch (...)
    {
        XERROR("InitMainOutputModes error!");       
    }
    return false;
}

bool cdataProcess::GetMainOutputModes(json & js)
{

    try
    {
        for (size_t i = 0; i < m_InitSupportModes.size(); i++)
        {
            json node;
            node["resolution"] = m_InitSupportModes[i].outputModeName;
            node["width"] = m_InitSupportModes[i].width;
            node["height"] = m_InitSupportModes[i].height;
            js.push_back(node);
        }

        XINFO("cdataProcess::GetMainOutputModes:{}", js.dump().c_str());
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return false;
}

bool cdataProcess::ResetOutputsInfo()
{
    cmyxrandr *pcmxrandr = cmyxrandr::GetInstance();
    pcmxrandr->SetOutputIsChanged();
    sleep(1);
    return true;
}

bool cdataProcess::SetMonitorsInfo(vector<MONITORSETTINGINFO> * vSetInfo)
{
    for (size_t i = 0; i < vSetInfo->size(); i++)
    {
        std::cout<< (*vSetInfo)[i].name << std::endl;
        std::cout<< (*vSetInfo)[i].pos.xPos<<std::endl;
        std::cout<< (*vSetInfo)[i].pos.yPos<<std::endl;
    }

    if(vSetInfo->size() > 0)
    {
        // string strDisplayName = ":0";
        // cmyxrandr cxr(strDisplayName);
        cmyxrandr* pcmxrandr =  cmyxrandr::GetInstance();

        for (size_t i = 0; i < vSetInfo->size(); i++)
        {
            pcmxrandr->setOutPut((*vSetInfo)[i].outputId);
            RRCrtc rrcrtc = pcmxrandr->getCrtc();
            pcmxrandr->setCrtc(rrcrtc);
            CMYSIZE size((*vSetInfo)[i].size.width, (*vSetInfo)[i].size.height);
            pcmxrandr->setMode(size, 447);
        }

        for (size_t i = 0; i < vSetInfo->size(); i++)
        {
            pcmxrandr->setOutPut((*vSetInfo)[i].outputId);
            RRCrtc rrcrtc = pcmxrandr->getCrtc();
            pcmxrandr->setCrtc(rrcrtc);
            //CMYSIZE size((*vSetInfo)[i].size.width, (*vSetInfo)[i].size.height);
            //CMYPOINT offset((*vSetInfo)[i].pos.xPos * m_nWidth, (*vSetInfo)[i].pos.yPos * m_nHight);
            CMYPOINT offset((*vSetInfo)[i].pos.xPos, (*vSetInfo)[i].pos.yPos);
            pcmxrandr->setOffset(offset);
        }
        
    }

    // //"rroutputId": 479,"modeId": 447,"name": "HDMI-0","primary": true,"width": 1920,"height": 1080,"xPos": 1920,"yPos": 0
    // cmyxrandr cxr(strDisplayName);
    // CMYPOINT offset(1920,0);
    // cxr.setOutPut(479);
    // RRCrtc rrcrtc = cxr.getCrtc();
    // cxr.setCrtc(rrcrtc);
    // //
    // CMYSIZE size(1920,1080);
    // cxr.setMode(size,447);
    // cxr.setOffset(offset);
    

    return true;
}

bool cdataProcess::SetOutputsInfo(json & js)
{
    XINFO("{}",js.dump().c_str());
    // if(js.find("layoutName") == js.end())
    // {
    //     XERROR("layoutName is not exist");
    //     return false;
    // }
        
    if(js.find("resolution") == js.end())
    {
        XERROR("resolution is not exist");
        return false;
    }
    if(js.find("allResolution") == js.end())
    {
        XERROR("allResolution is not exist");
        return false;
    }
    if(js.find("layout") == js.end())
    {
        XERROR("layout is not exist");
        return false;
    }

    //string layoutName = js["layoutName"].get<std::string>();
    string resolution = js["resolution"].get<std::string>();
    string allResolution = js["allResolution"].get<std::string>();
    json jarry = js["layout"];
    int num = jarry.size();
    if(num <= 0)
    {
        return false;
    }
    std::vector<std::string> vWidthAndHight = CMDEXEC::Split(resolution, 'x');
    int _width = std::stoi(vWidthAndHight[0]);
    int _hight = std::stoi(vWidthAndHight[1]);
    // std::vector<std::string> vLayout = CMDEXEC::Split(layoutName, 'x');
    // int _layout_w = std::stoi(vLayout[0]);
    // int _layout_h = std::stoi(vLayout[1]);
    int _layout_horizontal = js["layout_horizontal"].get<int>();
    int _layout_vertical = js["layout_vertical"].get<int>();

    // string strDisplayName = ":0";
    // cmyxrandr cxr(strDisplayName);
    cmyxrandr* pcmxrandr = cmyxrandr::GetInstance();
    CMYSIZE currentSize, maxSize;
    vector<MOutputInfo> vOutputInfo;  
    //short shRet = pcmxrandr->getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
    //short shRet = pcmxrandr->getAllScreenInfoNew(vOutputInfo,currentSize,maxSize);
    short shRet = pcmxrandr->getAllScreenInfoXrandr(vOutputInfo,currentSize,maxSize);
    if(shRet != 0)
    {
        return false;
    }
    // for (size_t i = 0; i < vOutputInfo.size(); i++)
    // {
    //     vOutputInfo[i].bIsSeted = false;
    // }
    

    bool bIsChanged = false;

    string strAllSetStr = "xrandr";
    string strAllOutputMode = "xrandr";
    int nMaxId = 0;
    for (json::iterator it = jarry.begin(); it != jarry.end(); it++)
    {
        int nId = (*it)["id"].template get<int>();
        if(nId > nMaxId)
        {
            nMaxId = nId;
        }
    }

    m_mutex.lock();
    for (json::iterator it = jarry.begin();it!=jarry.end();it++)
    {
        string output = (*it)["name"].template get<std::string>();
        int nCoordinateX = 0;
        int nCoordinateY = 0;
        // if((*it).find("coordinateOrderX") != (*it).end() && (*it).find("coordinateOrderY") != (*it).end())
        // {
        //     nCoordinateX = (*it)["coordinateOrderX"].template get<int>();
        //     nCoordinateY = (*it)["coordinateOrderY"].template get<int>();
        //     nCoordinateX = nCoordinateX * _width;
        //     nCoordinateY = nCoordinateY * _hight;
        // }
        // else
        // {
            int nId = (*it)["id"].template get<int>();
            nCoordinateX = (nId % _layout_vertical) * _width;
            nCoordinateY = (nId / _layout_vertical) * _hight;
        //}               
        //
        bool primary = (*it)["primary"].template get<bool>();
        for (size_t i = 0; i < vOutputInfo.size(); i++)
        {
            if(output.compare(vOutputInfo[i].name) == 0)//找到显示器,未找的话不处理
            {
                MyModelInfoEX * pMode = &vOutputInfo[i].currentMode;
                if(vOutputInfo[i].currentMode.name.compare(resolution) != 0 || vOutputInfo[i].pos.xPos != nCoordinateX || vOutputInfo[i].pos.yPos != nCoordinateY)
                {
                    vOutputInfo[i].bIsSeted = true;
                    bool bFind = false;
                    for (size_t l = 0; l < vOutputInfo[i].modes.size(); l++)
                    {
                        if(vOutputInfo[i].modes[l].name.compare(resolution) == 0)
                        { 
                            bIsChanged = true;
                            bFind = true;
                            pMode = &vOutputInfo[i].modes[l];
                            string strOutputMode = "";
                            //if(!setOutputMode(vOutputInfo[i].name, vOutputInfo[i].modes[l].name,vOutputInfo[i].modes[l].rate,strOutputMode))
                            string strTmp = "";
                            //sleep(8);
                            if(!setOutputModeAndPos(vOutputInfo[i].name, pMode->name,pMode->rate,nCoordinateX, nCoordinateY,strTmp))
                            {
                                m_mutex.unlock();
                                return false;
                            }
                            //usleep(500000);
                            strAllOutputMode += strOutputMode;
                            break;
                        }
                    }
                    if(!bFind)
                    {
                        m_mutex.unlock();
                        XERROR("未找到相应的分辨率,{} can not find resolution:{}",vOutputInfo[i].name,resolution);
                        return false;
                    }
                }

                //if (vOutputInfo[i].pos.xPos != nCoordinateX || vOutputInfo[i].pos.yPos != nCoordinateY)
                // {
                //     bIsChanged = true;
                //     //if(!setOutputPos(vOutputInfo[i].name, nCoordinateX, nCoordinateY))
                //     string strOutputLayout ="";
                //     if(!setOutputModeAndPos(vOutputInfo[i].name, pMode->name,pMode->rate,nCoordinateX, nCoordinateY,strOutputLayout))
                //     {
                //         return false;
                //     }
                //     strAllSetStr += strOutputLayout;
                // }
                break;
            }
        }
    }

    //后添加的显示器，没有配置的分辨率，需要添加
    for (size_t i = 0; i < vOutputInfo.size(); i++)
    {
        if (!vOutputInfo[i].bIsSeted)
        {
            int nCoordinateX = 0;
            int nCoordinateY = 0;
            nMaxId++;
            nCoordinateX = (nMaxId % _layout_vertical) * _width;
            nCoordinateY = (nMaxId / _layout_vertical) * _hight;
            MyModelInfoEX *pMode = &vOutputInfo[i].currentMode;
            for (size_t l = 0; l < vOutputInfo[i].modes.size(); l++)
            {
                if (vOutputInfo[i].modes[l].name.compare(resolution) == 0)
                {
                    bIsChanged = true;

                    pMode = &vOutputInfo[i].modes[l];
                    string strOutputMode = "";
                    // if(!setOutputMode(vOutputInfo[i].name, vOutputInfo[i].modes[l].name,vOutputInfo[i].modes[l].rate,strOutputMode))
                    string strTmp = "";
                    if (!setOutputModeAndPos(vOutputInfo[i].name, pMode->name, pMode->rate, nCoordinateX, nCoordinateY, strTmp))
                    {
                        m_mutex.unlock();
                        return false;
                    }
                    // usleep(500000);
                    strAllOutputMode += strOutputMode;
                    break;
                }
            }

            //XERROR("未找到相应的输出,can not find output:{}", output);
            m_mutex.unlock();
            return false;
        }
    }

    //cmyxrandr* pcmxrandr = cmyxrandr::GetInstance();
    bool bIsNeedSendMsg = false;
    if(bIsChanged)
    {
        pcmxrandr->SetOutputIsChanged();
        sleep(3);
        shRet = pcmxrandr->getAllScreenInfoNew(vOutputInfo,currentSize,maxSize);
        if(shRet == 0)
        {
            for (json::iterator it = jarry.begin(); it != jarry.end(); it++)
            {
                string outputName = (*it)["name"].template get<std::string>();
                int nId = (*it)["id"].template get<int>();
                string resolution = js["resolution"].get<std::string>();
                int nCoordinateX = 0;
                int nCoordinateY = 0;
                nCoordinateX = (nId % _layout_vertical) * _width;
                nCoordinateY = (nId / _layout_vertical) * _hight;
                if(!checkOutputModeAndPos(vOutputInfo,outputName,nCoordinateX,nCoordinateY,resolution))
                {
                    bIsNeedSendMsg = true;
                    string strTmp = "";
                    string strTmp2 = "";
                    setOutputModeAndPos(outputName, resolution, strTmp2, nCoordinateX, nCoordinateY, strTmp);                    
                }
            }
        }
    }
    //m_mutex.unlock();       
    
    // if(strAllSetStr.length() > 10)
    // {
    //     // XINFO("setOutputModeAndPos exec :{}\n", strAllOutputMode.c_str());
    //     CMDEXEC::CmdRes res;
    //     // bool bRet = CMDEXEC::Execute(strAllOutputMode, res);
    //     // XCRITICAL("setOutputModeAndPos CMDEXEC::Execute errono:{},error:{}\n", res.ExitCode, res.StderrString);        
        
    //     res.ExitCode = 0;   
    //     res.StderrString = ""; 
    //     res.StdoutString = "";
    //     XINFO("setOutputModeAndPos exec :{}", strAllSetStr.c_str());        
    //     bool bRet = CMDEXEC::Execute(strAllSetStr, res);
    //     XERROR("cdataProcess::setOutputModeAndPos CMDEXEC::Execute errono:{},error:{}\n", res.ExitCode, res.StderrString);
    //     if (!bRet)
    //     {
    //         XERROR("cdataProcess::setOutputModeAndPos CMDEXEC::Execute errono:{},error:{}\n", res.ExitCode, res.StderrString);
    //         return false;
    //     }
    // }


    m_layout_horizontal = _layout_horizontal;
    m_layout_vertical = _layout_vertical;
    m_nWidth = _width;
    m_nHight = _hight;
    m_allLayouts = allResolution;
    if(bIsNeedSendMsg)
    {
        //cmyxrandr* pcmxrandr = cmyxrandr::GetInstance();
        pcmxrandr->SetOutputIsChanged();
        usleep(3.5f*1000*1000);
    }

    m_mutex.unlock();

    return true;
}

bool cdataProcess::checkOutputModeAndPos(vector<MOutputInfo> & vOutputInfo,string &strOutputName,int x, int y, string & strModeName)
{
    for (size_t i = 0; i < vOutputInfo.size(); i++)
    {
        if(vOutputInfo[i].name.compare(strOutputName) == 0)
        {
            if(vOutputInfo[i].pos.xPos == x && vOutputInfo[i].pos.yPos == y  && vOutputInfo[i].currentMode.name.compare(strModeName) == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }       

    return true;
}



bool cdataProcess::TestMonitorInfo()
{
    json js;
    GetOutputAndGpuName(js);
    XINFO("{}",js.dump());
    //
    //GetServerInfo(js);


    // string strDisplayName = ":0";
    // cmyxrandr cxr(strDisplayName);
    // vector<MOutputInfo> vOutputInfo;
    // CMYSIZE currentSize, maxSize;
    // cxr.getAllScreenInfoEx(vOutputInfo,currentSize, maxSize);
    // nvControlInfo nv;
    // MGPUINFOEX  gpu;
    // nv.getGpuInfo(gpu);

    // DLGINFO dlg;
    // dlg.height = 1080;
    // dlg.width = 3840;
    // dlg.postype = 1;
    // dlg.name = "test";
    // dlg.xPos = 200;
    // dlg.yPos = 50;
    // dlg.url = "https://www.baidu.com";

    //std::shared_ptr<dlgManager> pdlgManager = dlgManager::GetInstance();
    //pdlgManager->addDlg(dlg);

    //m_pMainWindow->test();
    
    


    return false;
}

bool cdataProcess::GetGpuInfo(json & js)
{
    XINFO("{cdataProcess::GetGpuInfo in}\n");
    try
    {
        MGPUINFOEX gpu;
        nvControlInfo nv;
        if (nv.getGpuInfo(gpu))
        {
            for (size_t i = 0; i < gpu.vgpus.size(); i++)
            {
                MGPUINFO info = gpu.vgpus[i];
                json node;
                node["gpuId"] = info.id;
                node["fan"] = info.Fan;
                node["mem"] = info.Mem;
                node["name"] = info.name;
                node["perf"] = info.Perf;
                node["pwr"] = info.Pwr;
                node["tempture"] = info.Temp;
                node["util"] = info.Util;
                js["gpus"].push_back(node);
            }
            // strInfo = js.dump().c_str();
            return true;
        }
        XINFO("{cdataProcess::GetGpuInfo out}\n");
    }
    catch(...)
    {
        XERROR("cdataProcess::GetGpuInfo errno={}",errno);
    }
    return false;
}

bool cdataProcess::setOutputMode(string &strOutputName, string &strModeName,string & strRate,string & outputMode)
{
    outputMode = " --output ";
    string strTmp = "";
    string strCmd = "xrandr --output ";
    strTmp += strOutputName;
    strTmp += " --mode ";
    strTmp += strModeName;
    // strCmd += " --pos ";
    // strCmd += strPos;
    strTmp += " --rate ";
    strTmp += strRate;

    outputMode += strTmp;
    strCmd += strTmp;

    // XINFO("exec :{}", strCmd.c_str());
    // CMDEXEC::CmdRes res;
    // bool bRet = CMDEXEC::Execute(strCmd, res);
    // if (!bRet)
    // {
    //     XERROR("cdataProcess::setOutputMode CMDEXEC::Execute errono:{},error:{}", res.ExitCode, res.StderrString);
    // }
    ////usleep(500000);
    return true;
}

bool cdataProcess::setOutputPos(string &strOutputName, int start_x,int start_y)
{
    string strCmd = "xrandr --output ";
    char buf[20] = {0};
    sprintf(buf, "%dx%d", start_x, start_y);
    string strPos = buf;
    strCmd += strOutputName;
    strCmd += " --pos ";
    strCmd += strPos;
    XINFO("setOutputPos exec :{}", strCmd.c_str());
    CMDEXEC::CmdRes res;
    bool bRet = CMDEXEC::Execute(strCmd, res);
    if (!bRet)
    {
        XERROR("cdataProcess::setOutputPos CMDEXEC::Execute errono:{},error:{}", res.ExitCode, res.StderrString);
        return false;
    }
    //usleep(500000);
    return true;
}

bool cdataProcess::setOutputModeAndPos(string &strOutputName,string &strModeName,string & strRate, int start_x,int start_y,string & outputLayout)
{    
    char buf[20] = {0};
    sprintf(buf, "%dx%d", start_x, start_y);
    string strPos = buf;
    string strCmd = "xrandr --output ";
    outputLayout  = " --output ";
    string strTmp = "";
    strTmp += strOutputName;
    strTmp += " --mode ";
    strTmp += strModeName;
    strTmp += " --pos ";
    strTmp += strPos;
    outputLayout += strTmp;
    if(!strRate.empty())
    {
        strTmp += " --rate ";
        strTmp += strRate;
    }
    //strTmp += " --rate ";
    //strTmp += strRate;
    strTmp += " --auto";
    strCmd += strTmp;
    

   


    // if(start_x == 0 && start_y == 0)
    // {
    //     strCmd += " --primary";
    // }

   
    XINFO("setOutputModeAndPos exec :{}", strCmd.c_str());
    CMDEXEC::CmdRes res;
    bool bRet = CMDEXEC::Execute(strCmd, res);
    if (!bRet)
    {
        XERROR("cdataProcess::setOutputModeAndPos CMDEXEC::Execute errono:{},error:{}", res.ExitCode, res.StderrString);
        return false;
    }
    return true;
}


bool cdataProcess::setOutputsXrandr(json & js)
{
    string resolution = js["resolution"].get<std::string>();
    string allResolution = js["allResolution"].get<std::string>();
    json jarry = js["layout"];
    int num = jarry.size();
    if (num <= 0)
    {
        return false;
    }

    int nMaxId = 0;
    for (json::iterator it = jarry.begin(); it != jarry.end(); it++)
    {
        int nId = (*it)["id"].template get<int>();
        if (nId > nMaxId)
        {
            nMaxId = nId;
        }
    }
    bool bIsChange = false;

    cmyxrandr *pcmxrandr = cmyxrandr::GetInstance();
    list<RROutput> outputs = pcmxrandr->getOutputs();
    
    {        
        std::vector<std::string> vWidthAndHight = CMDEXEC::Split(resolution, 'x');
        int _width = std::stoi(vWidthAndHight[0]);
        int _hight = std::stoi(vWidthAndHight[1]);
        
        int _layout_horizontal = js["layout_horizontal"].get<int>();
        int _layout_vertical = js["layout_vertical"].get<int>();

       
        
        CMYSIZE currentSize, maxSize;
        vector<MOutputInfo> vOutputInfo;
        //short shRet = pcmxrandr->getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
        //short shRet = pcmxrandr->getAllScreenInfoNew(vOutputInfo, currentSize, maxSize);
        short shRet = pcmxrandr->getAllScreenInfoXrandr(vOutputInfo,currentSize,maxSize);
        if (shRet != 0)
        {
            return false;
        }
       
        for (json::iterator it = jarry.begin(); it != jarry.end(); it++)
        {
            string outputName = (*it)["name"].template get<std::string>();
            int nCoordinateX = 0;
            int nCoordinateY = 0;
           
            int nId = (*it)["id"].template get<int>();
            nCoordinateX = (nId % _layout_vertical) * _width;
            nCoordinateY = (nId / _layout_vertical) * _hight;
            
            bool primary = (*it)["primary"].template get<bool>();
            for (size_t i = 0; i < vOutputInfo.size(); i++)
            {
                if (outputName.compare(vOutputInfo[i].name) == 0) // 找到显示器,未找的话不处理
                {
                    vOutputInfo[i].bIsSeted = true;
                    MyModelInfoEX *pMode = &vOutputInfo[i].currentMode;
                    bool bIsNeedSetMode = false;
                    bool bIsFoundMode = false;
                    bool bIsNeedSetPos = false;
                    //if (vOutputInfo[i].currentMode.name.compare(resolution) != 0)
                    {
                        bIsNeedSetMode = true;
                        for (size_t l = 0; l < vOutputInfo[i].modes.size(); l++)
                        {
                            if (vOutputInfo[i].modes[l].name.compare(resolution) == 0)
                            {
                                bIsFoundMode = true;              
                                pMode = &vOutputInfo[i].modes[l];
                                // if (!setOutputMode(vOutputInfo[i].name, vOutputInfo[i].modes[l].name, vOutputInfo[i].modes[l].rate))
                                // {
                                //     return false;
                                // }
                                break;
                            }
                        }
                    }

                    //if (vOutputInfo[i].pos.xPos != nCoordinateX || vOutputInfo[i].pos.yPos != nCoordinateY)
                    {
                        bIsNeedSetPos = true;                        
                    }

                    //for (list<RROutput>::iterator itOut = outputs.begin(); itOut != outputs.end(); itOut++)
                    {
                        //RROutput rroutput = *itOut;
                        //pcmxrandr->setOutPut(rroutput);
                        pcmxrandr->setOutPut(vOutputInfo[i].outputId);
                        XRROutputInfo *xrroutinfo = pcmxrandr->GetOutputInfo();
                        string strOutputName = xrroutinfo->name;
                        pcmxrandr->setOutPutName(xrroutinfo->name);

                        //if (strOutputName.compare(outputName) == 0)
                        {
                            if(xrroutinfo->crtc > 0)
                            {
                                pcmxrandr->setCrtc(xrroutinfo->crtc);
                            }
                            // else
                            // {
                            //     MYCOMMON::CMYSIZE size(pMode->width,pMode->height);
                            //     pcmxrandr->setMode(size,0);
                            //     RRCrtc crtc = pcmxrandr->getCrtc();
                            //     
                            //     bIsChange = true;          
                                
                            // }
                            //XINFO("rroutput={},vOutputInfo[i].outputId={},xrroutinfo->crtc={},pMode.id={}",rroutput,vOutputInfo[i].outputId,xrroutinfo->crtc,pMode->id);
                            if(bIsNeedSetMode && bIsFoundMode)
                            {
                                bIsChange = true;          
                                MYCOMMON::CMYSIZE size(pMode->width,pMode->height);
                                if(xrroutinfo->crtc > 0)
                                {
                                    pcmxrandr->setMode(size,pMode->id);
                                }
                                else
                                {
                                    pcmxrandr->setMode(size,0);
                                    RRCrtc crtc = pcmxrandr->getCrtc();
                                    if(crtc == 0)
                                    {
                                        XERROR("cdataProcess::setOutputsXrandr error, crtc id = 0");
                                    }
                                    pcmxrandr->setCrtc(crtc);
                                }
                            }
                            else if(bIsNeedSetMode && bIsFoundMode == false)
                            {
                                bIsChange = true;
                                std::vector<std::string> vWidthAndHight = CMDEXEC::Split(resolution, 'x');
                                int _lswidth = std::stoi(vWidthAndHight[0]);
                                int _lshight = std::stoi(vWidthAndHight[1]);
                                MYCOMMON::CMYSIZE size(_lswidth,_lshight);
                                pcmxrandr->setMode(size,0);
                            }                           

                            if(bIsNeedSetPos)
                            {                 
                                bIsChange = true;               
                                CMYPOINT offset(nCoordinateX, nCoordinateY);
                                pcmxrandr->setOffset(offset);
                                if(nCoordinateX == 0 && nCoordinateY == 0)
                                    pcmxrandr->setPrimary();
                            }
                            //break;
                        }
                    }
                    break;
                }
            }
        }

        // 后添加的显示器，没有配置的分辨率，需要添加
        /*
        for (size_t i = 0; i < vOutputInfo.size(); i++)
        {
            if (!vOutputInfo[i].bIsSeted)
            {
                int nCoordinateX = 0;
                int nCoordinateY = 0;
                nMaxId++;
                nCoordinateX = (nMaxId % _layout_vertical) * _width;
                nCoordinateY = (nMaxId / _layout_vertical) * _hight;
                MyModelInfoEX *pMode = &vOutputInfo[i].currentMode;
                for (size_t l = 0; l < vOutputInfo[i].modes.size(); l++)
                {

                    if (vOutputInfo[i].modes[l].name.compare(resolution) == 0)
                    {
                        pcmxrandr->setOutPut(vOutputInfo[i].outputId);
                        XRROutputInfo *xrroutinfo = pcmxrandr->GetOutputInfo();
                        string strOutputName = xrroutinfo->name;
                        pcmxrandr->setOutPutName(xrroutinfo->name);
                        if (strOutputName.compare(vOutputInfo[i].name) != 0)
                        {
                            XERROR("have a bug!!!\n");
                            break;
                        }
                        bIsChange = true;
                        pcmxrandr->setCrtc(xrroutinfo->crtc);

                        std::vector<std::string> vWidthAndHight = CMDEXEC::Split(resolution, 'x');
                        int _lswidth = std::stoi(vWidthAndHight[0]);
                        int _lshight = std::stoi(vWidthAndHight[1]);

                        MYCOMMON::CMYSIZE size(_lswidth, _lshight);
                        pcmxrandr->setMode(size, 0);

                        CMYPOINT offset(nCoordinateX, nCoordinateY);
                        pcmxrandr->setOffset(offset);

                        // pMode = &vOutputInfo[i].modes[l];
                        // string strOutputMode = "";
                        // // if(!setOutputMode(vOutputInfo[i].name, vOutputInfo[i].modes[l].name,vOutputInfo[i].modes[l].rate,strOutputMode))
                        // string strTmp = "";
                        // if (!setOutputModeAndPos(vOutputInfo[i].name, pMode->name, pMode->rate, nCoordinateX, nCoordinateY, strTmp))
                        // {
                        //     m_mutex.unlock();
                        //     return false;
                        // }
                        // // usleep(500000);
                        // strAllOutputMode += strOutputMode;
                        break;
                    }
                }

                // XERROR("未找到相应的输出,can not find output:{}", output);                
                //return false;
            }
        }
        */
        m_nWidth = _width;
        m_nHight = _hight;
        m_layout_horizontal = _layout_horizontal;
        m_layout_vertical = _layout_vertical;
        m_allLayouts = allResolution;
    }
    
    //if(bIsChange)
    {
        //pcmxrandr->SetOutputIsChanged();
        usleep(2000*1000);
        pcmxrandr->OnUpdate();
        //usleep(3500*1000);
        pcmxrandr->SetOutputIsChanged();
    }    

    return true;
}

bool cdataProcess::InitOutputInfo()
{
    cmyxrandr *pcmxrandr = cmyxrandr::GetInstance();
    
    // list<RROutput> outputs = pcmxrandr->getOutputs();
    // for (list<RRCrtc>::iterator it = outputs.begin(); it != outputs.end(); it++)
    // {
    //     RROutput output = *it;
    //     pcmxrandr->setOutPut(output);        
    //     XRROutputInfo *outinfo = pcmxrandr->GetOutputInfo();
    //     string strOutputName = outinfo->name;
    //     if(strOutputName.compare("DP-5") == 0)
    //     {
    //         pcmxrandr->setCrtc(outinfo->crtc);
    //         CMYPOINT offset(0, 1050);
    //         pcmxrandr->setOffset(offset);
    //     }

        
    //     // outinfo->name;
    //     // outinfo->mm_width;
    //     // outinfo->mm_height;

    // }
    // return true;

    QSettings settings("config.ini", QSettings::IniFormat);
    bool bIsSetting = settings.value("screen/isSetting",false).toBool();

    if(bIsSetting)
    {
        string strJson = "";
        strJson = settings.value("outputsSettings/outputs", "").toString().toStdString();
        if(!strJson.empty())
        {
            XINFO("outputs json:{}",strJson.c_str());
            json jdata = json::parse(strJson.c_str());           
            //SetOutputsInfo(jdata);
            setOutputsXrandr(jdata);
            return true;
        }
    }

   
    m_nWidth = settings.value("screen/width",0).toInt();
    m_nHight = settings.value("screen/height",0).toInt();
    m_layout_horizontal = settings.value("screen/layout_horizontal",0).toInt();
    m_layout_vertical = settings.value("screen/layout_vertical",0).toInt();
    m_allLayouts = settings.value("screen/allResolution","").toString().toStdString();

    if(m_nWidth == 0)
    {
        m_nWidth = 1920;
        m_nHight = 1080;
    }
  
    if(m_layout_vertical ==  0 || m_layout_horizontal == 0)
    {
        XERROR("请先在config.ini中配置layout_horizontal和layout_vertical");
        exit(0);
    }



    vector<MOutputInfo> vOutputInfo;
    CMYSIZE currentSize, maxSize;
    //cmyxrandr* pcmxrandr = cmyxrandr::GetInstance();
    //pcmxrandr->getAllScreenInfoEx(vOutputInfo,currentSize, maxSize);
    //pcmxrandr->getAllScreenInfoNew(vOutputInfo,currentSize, maxSize);
    short shRet = pcmxrandr->getAllScreenInfoXrandr(vOutputInfo,currentSize,maxSize);

    unsigned long currentModeId = 0,preferredModeId = 0,lastDeterminedModeId = 0;
    string lastDeterminedModeName = "",lastDeterminedModeRate = "";
    int size_w = 0,size_h = 0;


    for (size_t i = 0; i < vOutputInfo.size(); i++)
    {
        
        currentModeId = vOutputInfo[i].currentMode.id;
        preferredModeId = vOutputInfo[i].preferredMode.id;
        if(lastDeterminedModeId == 0)
        {
            if (vOutputInfo[i].connected && vOutputInfo[i].primary)
            {
                for (size_t j = 0; j < vOutputInfo[i].modes.size(); j++)
                {
                    MyModelInfoEX &modex = vOutputInfo[i].modes[j];
                    if(currentModeId == modex.id)
                    {
                        size_w = modex.width;
                        size_h = modex.height;
                        lastDeterminedModeName = modex.name;
                        lastDeterminedModeRate = modex.rate;
                        XINFO("preferred OutputName={}, modeName={}, OutputId={}, modeId={}, width:{}, height:{}, rate:{}",
                        vOutputInfo[i].name,modex.name,vOutputInfo[i].outputId,modex.id,modex.width,modex.height,modex.rate);
                        break;
                    }                  
                }

                if(size_w != m_nWidth || size_h != m_nHight)
                {
                    XCRITICAL("Setting width_height:{}x{},preferred width_height:{}x{},there is significant defferenc,We will try using the set values.",
                    m_nWidth,m_nHight,size_w,size_h);
                    for (size_t j = 0; j < vOutputInfo[i].modes.size(); j++)
                    {
                        if(vOutputInfo[i].modes[j].width == m_nWidth && vOutputInfo[i].modes[j].height == m_nHight)
                        {
                            lastDeterminedModeId = vOutputInfo[i].modes[j].id;
                            lastDeterminedModeName = vOutputInfo[i].modes[j].name;
                            lastDeterminedModeRate = vOutputInfo[i].modes[j].rate;
                            //, rate:{} ,vOutputInfo[i].modes[j].rate
                            XINFO("we find a mode,Name={}, modeId={}, width:{}, height:{}",vOutputInfo[i].modes[j].name,
                            vOutputInfo[i].modes[j].id,vOutputInfo[i].modes[j].width,vOutputInfo[i].modes[j].height);
                            break;
                        }
                    }
                }
                else
                {
                    lastDeterminedModeId = currentModeId;
                    break;
                }
                      
            }
        }
    }

    int start_x = 0,start_y = 0;
    if (m_layout_vertical == 1 && m_layout_horizontal == 1)
        m_layout_vertical = vOutputInfo.size();

    for (size_t i = 0,j = 0; i < vOutputInfo.size(); i++)
    {
        //if(vOutputInfo[i].currentMode.name.compare(lastDeterminedModeName) != 0)//先设置mode
        {
            if ((i - (j * m_layout_vertical)) >= m_layout_vertical)
            {
                j++;
            }

            start_x = (i - (j * m_layout_vertical)) * m_nWidth;
            start_y = j * m_nHight;

            /////
            pcmxrandr->setOutPut(vOutputInfo[i].outputId);
            XRROutputInfo *xrroutinfo = pcmxrandr->GetOutputInfo();
            string strOutputName = xrroutinfo->name;
            pcmxrandr->setOutPutName(xrroutinfo->name);

            {
                if (xrroutinfo->crtc > 0)
                {
                    pcmxrandr->setCrtc(xrroutinfo->crtc);
                }
            
                if (vOutputInfo[i].preferredMode.id > 0)
                {
                    MYCOMMON::CMYSIZE size(vOutputInfo[i].preferredMode.width, vOutputInfo[i].preferredMode.height);
                    pcmxrandr->setMode(size, vOutputInfo[i].preferredMode.id);
                    
                    
                }
                else
                {                               
                    MYCOMMON::CMYSIZE size(vOutputInfo[i].modes[0].width, vOutputInfo[i].modes[0].height);
                    if (xrroutinfo->crtc > 0)
                    {
                        pcmxrandr->setMode(size, vOutputInfo[i].modes[0].id);
                    }
                    else
                    {
                        pcmxrandr->setMode(size, 0);
                        RRCrtc crtc = pcmxrandr->getCrtc();
                        if (crtc == 0)
                        {
                            XERROR("cdataProcess::setOutputsXrandr error, crtc id = 0");
                        }
                        pcmxrandr->setCrtc(crtc);
                    }         
                    
                }

                CMYPOINT offset(start_x, start_y);
                pcmxrandr->setOffset(offset);
                if (start_x == 0 && start_y == 0)
                    pcmxrandr->setPrimary();
            }

            /*

            char buf[20] = {0};
            sprintf(buf, "%dx%d", start_x, start_y);
            string strPos = buf;
            string strOutputName = vOutputInfo[i].name;            
            string strCmd = "xrandr --output ";
            strCmd += strOutputName;
            strCmd += " --mode ";
            strCmd += lastDeterminedModeName;
            strCmd += " --pos ";
            strCmd += strPos;
            strCmd += " --auto";
            //strCmd += " --rate ";      
            //strCmd += lastDeterminedModeRate;
            if(vOutputInfo[i].primary)
            {
                strCmd += " --primary"; 
            }
            XINFO("exec :{}",strCmd.c_str());
            CMDEXEC::CmdRes res;
            bool bRet = CMDEXEC::Execute(strCmd,res);
            if(!bRet)
            {
                XERROR("cdataProcess::InitOutputInfo CMDEXEC::Execute errono:{},error:{}",res.ExitCode,res.StderrString);
            }
            */

        }
/*
        {

            if ((i - (j * m_layout_vertical)) >= m_layout_vertical)
            {
                j++;
            }

            start_x = (i - (j * m_layout_vertical)) * m_nWidth;
            start_y = j * m_nHight;
            char buf[20] = {0};
            sprintf(buf, "%dx%d", start_x, start_y);
            string strPos = buf;
            string strOutputName = vOutputInfo[i].name;
            string strCmd = "xrandr --output ";
            strCmd += strOutputName;
            strCmd += " --mode ";
            strCmd += lastDeterminedModeName;
            strCmd += " --pos ";
            strCmd += strPos;
            strCmd += " --auto";
            //strCmd += " --rate ";
            //strCmd += lastDeterminedModeRate;
            if (vOutputInfo[i].primary)
            {
                strCmd += " --primary";
            }
            XINFO("exec :{}", strCmd.c_str());

            if (vOutputInfo[i].pos.xPos != start_x || vOutputInfo[i].pos.yPos != start_y) // 再设置pos
            {
                // if((i - j*m_nWidth) < m_nWidth)               
                CMDEXEC::CmdRes res;
                bool bRet = CMDEXEC::Execute(strCmd, res);
                if (!bRet)
                {
                    XERROR("cdataProcess::InitOutputInfo CMDEXEC::Execute errono:{},error:{}", res.ExitCode, res.StderrString);
                }
            }
            else
                continue;
        }
        */      
    }
 
    
    
    return true;

}





bool cdataProcess::GetOutputAndGpuName(json & js)
{
    cmyxrandr * p =cmyxrandr::GetInstance();
    return p->GetOutputAndGpuName(js);
    /*    
    int major, minor, len;
    char *start, *str0, *str1;
    int *enabledDpyIds;

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) 
    {
        XERROR("Cannot open display {}.", XDisplayName(NULL));
        return false;
    }
    
    int screen = GetNvXScreen(dpy);
    Bool ret = XNVCTRLQueryVersion(dpy, &major, &minor);
    if (ret != True)
    {
        XERROR("The NV-CONTROL X extension does not exist on {}.\n\n",XDisplayName(NULL));
        return false;
    }

    XINFO("Using NV-CONTROL extension {}.{} on {} \n", major, minor, XDisplayName(NULL));
    
    XINFO("Display Device Probed Information:\n\n");


    int num_gpus = 0;
    ret = XNVCTRLQueryTargetCount(dpy, NV_CTRL_TARGET_TYPE_GPU,&num_gpus);
    if (!ret)
    {
        XERROR("Failed to query number of gpus\n\n");
        XCloseDisplay(dpy);
        return false;
    }
    XINFO("number of GPUs: {}\n", num_gpus);

   

    for (int i = 0; i < num_gpus; i++)
    {
        json node;

        int deprecated;
        int *pData;

        
        char *gpuName = nullptr;
        ret = XNVCTRLQueryTargetStringAttribute(dpy, NV_CTRL_TARGET_TYPE_GPU, i, 0,NV_CTRL_STRING_PRODUCT_NAME, &gpuName);
        if (!ret)
        {
            XERROR("Failed to query gpu name\n\n");
            XCloseDisplay(dpy);
            return false;
        }

       
        ret = XNVCTRLQueryTargetAttribute(dpy,
                                          NV_CTRL_TARGET_TYPE_GPU, i,
                                          0,
                                          NV_CTRL_PROBE_DISPLAYS,
                                          &deprecated);

        if (!ret)
        {
            XERROR("Failed to probe the enabled Display Devices on GPU-{} ({}).\n\n",i, gpuName);
            XCloseDisplay(dpy);
            return false;
        }

        XINFO("display devices on GPU-{} ({}):\n", i, gpuName);
        node["gpuName"].push_back(gpuName);
        if (gpuName) XFree(gpuName);
        

        
        ret = XNVCTRLQueryTargetBinaryData(dpy,
                                           NV_CTRL_TARGET_TYPE_GPU, i,
                                           0,
                                           NV_CTRL_BINARY_DATA_DISPLAYS_CONNECTED_TO_GPU,
                                           (unsigned char **)&pData,
                                           &len);
        if (!ret || (len < sizeof(pData[0])))
        {
            XERROR("Failed to query the connected Display Devices.\n\n");
            return 1;
        }

        for (int j = 0; j < pData[0]; j++)
        {
            int dpyId = pData[j + 1];
            print_display_id_and_name(dpy, dpyId, "    ");
            string strDisplayName;
            print_display_name(dpy, dpyId,NV_CTRL_STRING_DISPLAY_NAME_RANDR,"RANDR",strDisplayName);
            node["display"].push_back(strDisplayName);  
        }
        XFree(pData); 
        js["gpu"].push_back(node);        
    }   
    XCloseDisplay(dpy);
    */
    return true;
}

bool cdataProcess::GetServerInfo(json & js)
{
    string strCpuName = getCpuName();
    
    MEM_OCCUPY mem_stat;
    CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;
    //获取内存
    //(MemTotal - MemFree)/ MemTotal
    get_memoccupy((MEM_OCCUPY *)&mem_stat);  
    //printf(" [MemTotal] = %lu \n [MemFree] = %lu \n [Buffers] = %lu \n [Cached] = %lu \n [SwapCached] = %lu \n", mem_stat.MemTotal, mem_stat.MemFree, mem_stat.Buffers, mem_stat.Cached, mem_stat.SwapCached);  
    //(MemTotal - MemFree)/ MemTotal //1-(内存空闲 / 内存总数)*100
    printf("%.3f\n", (mem_stat.MemTotal - mem_stat.MemFree) / ( mem_stat.MemTotal * 1.0f) * 100.0f);
    float fMemUtil = (mem_stat.MemTotal - mem_stat.MemFree) / ( mem_stat.MemTotal * 1.0f) * 100.0f;
    //第一次获取cpu使用情况  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);  
    
    usleep(100000);
    
    //第二次获取cpu使用情况  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);  
    //计算cpu使用率  
    float fCpuUtil = 0;
    cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2,fCpuUtil);
    char strbuf[200] = {0};
    sprintf(strbuf,"%0.1fG",mem_stat.MemTotal/1024.0f/1024.0f);
    string strMemTotal = strbuf;
    memset(strbuf,0,sizeof(strbuf));
    sprintf(strbuf,"%0.1f%%",fMemUtil);
    string strMemUtil = strbuf;
    memset(strbuf,0,sizeof(strbuf));
    sprintf(strbuf,"%.1f%%",fCpuUtil * 100);
    string strCpuUtil = strbuf;

    string strOsName = "";
    auto obuf = sp::check_output({"cat", "/etc/issue"});
    XINFO("os:{}",obuf.buf.data());
    
    strOsName = obuf.buf.data();
    vector<string> vString;
    CMDEXEC::Stringsplit(strOsName,'\\',vString);
    if(vString.size()> 0)
        strOsName = vString[0];
    else
        strOsName = "";
   
    /// 读取executable所在绝对路径
    std::string exec_str = get_cur_executable_path();
    /// 用于获取磁盘剩余空间
    struct statfs diskInfo;
    statfs(exec_str.c_str(), &diskInfo);

    unsigned long long blocksize = diskInfo.f_bsize;              // 每个block里包含的字节数
    unsigned long long totalsize = blocksize * diskInfo.f_blocks; // 总的字节数，f_blocks为block的数目

    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
           totalsize, totalsize >> 10, totalsize >> 20, totalsize >> 30);

    unsigned long long freeDisk = diskInfo.f_bfree * blocksize;       // 剩余空间的大小
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize; // 可用空间大小
    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",
           freeDisk >> 20, freeDisk >> 30, availableDisk >> 20, availableDisk >> 30);

    memset(strbuf,0,sizeof(strbuf));
    sprintf(strbuf,"%dGB|%dGB",(totalsize - availableDisk) >> 30, totalsize >> 30);
    XINFO("disk total:{}GB, used:{}GB",totalsize >> 30,(totalsize - availableDisk) >> 30);
    string strDiskInfo = strbuf;

    js["cpu"] = strCpuName;
    js["cpuUtil"] = strCpuUtil;
    js["memTotal"] = strMemTotal;
    js["memUtil"] = strMemUtil;
    js["os"] = strOsName;
    js["disks"] = strDiskInfo;
    XINFO("system info:{}",js.dump().c_str());  

    return true;
}

void cdataProcess::get_memoccupy(MEM_OCCUPY *mem) //对无类型get函数含有一个形参结构体类弄的指针O  
{  
    FILE *fd;  
    char buff[256];  
    MEM_OCCUPY *m;  
    m = mem;  
      
    fd = fopen("/proc/meminfo", "r");  
    //MemTotal: 515164 kB  
    //MemFree: 7348 kB  
    //Buffers: 7892 kB  
    //Cached: 241852  kB  
    //SwapCached: 0 kB  
    //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里   
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name1, &m->MemTotal);  
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name2, &m->MemFree);  
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name3, &m->Buffers);  
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name4, &m->Cached);  
    fgets(buff, sizeof(buff), fd);   
    sscanf(buff, "%s %lu", m->name5, &m->SwapCached);  
      
    fclose(fd);     //关闭文件fd  
}

int cdataProcess::get_cpuoccupy(CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O  
{  
    FILE *fd;  
    char buff[256];  
    CPU_OCCUPY *cpu_occupy;  
    cpu_occupy = cpust;  
      
    fd = fopen("/proc/stat", "r");  
    fgets(buff, sizeof(buff), fd);  
      
    sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle, &cpu_occupy->lowait, &cpu_occupy->irq, &cpu_occupy->softirq);  
      
      
    fclose(fd);  
      
    return 0;  
}
void cdataProcess::cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n,float & util)  
{  
    unsigned long od, nd;  
    double cpu_use = 0;  
      
    od = (unsigned long)(o->user + o->nice + o->system + o->idle + o->lowait + o->irq + o->softirq);//第一次(用户+优先级+系统+空闲)的时间再赋给od  
    nd = (unsigned long)(n->user + n->nice + n->system + n->idle + n->lowait + n->irq + n->softirq);//第二次(用户+优先级+系统+空闲)的时间再赋给od  
    double sum = nd - od;  
    double idle = n->idle - o->idle;  
    cpu_use = idle / sum;  
    idle = n->user + n->system + n->nice - o->user - o->system - o->nice;  
    cpu_use = idle / sum;  
    printf("%.3f\n",cpu_use);
    XINFO("cal_cpuoccupy={}",cpu_use);
    util = cpu_use;
}

std::string cdataProcess::getCpuName() 
{
    std::ifstream ifs("/proc/cpuinfo");
    std::string line;
    std::string key = "model name";

    string cpuName = "";
    string siblings = "";
    string cpuCores = "";
    bool bIsFoundCpuName = false;
    int cpuCore[4] = {0};
    int cpuCoreMaxThreadId[4] = {0};

    while (std::getline(ifs, line)) {
        if (line.find(key) == 0)
        {
            cpuName = line.substr(key.length() + 2);
        }
        else if(line.find("siblings") == 0)
        {
            siblings = line.substr(key.length() + 1);
        }
        else if(line.find("cpu cores") == 0)
        {
            cpuCores = line.substr(key.length() + 1);
            break;
        }
        
    }
    //CMDEXEC::CmdRes res;
    //bool bret = CMDEXEC::Execute("cat /proc/cpuinfo |grep \'physical id\'|sort|uniq",res);
    //printf("%s\n",res.StdoutString.c_str());
    //cpu 颗数
    //auto obuf = sp::check_output({"cat", "/proc/cpuinfo|grep 'physical id'|sort|uniq|wc -l"});
    //auto obuf = sp::pipeline("cat /proc/cpuinfo", "grep \'physical id\'");//, "sort","uniq"

    auto cat = sp::Popen({"cat", "/proc/cpuinfo"}, sp::output{sp::PIPE});
    auto grep = sp::Popen({"grep", "physical id"}, sp::input{cat.output()}, sp::output{sp::PIPE});
    auto sort = sp::Popen({"sort"}, sp::input{grep.output()}, sp::output{sp::PIPE});
    auto uniq = sp::Popen({"uniq"}, sp::input{sort.output()}, sp::output{sp::PIPE});   
    auto cut = sp::Popen({"wc", "-l"}, sp::input{uniq.output()}, sp::output{sp::PIPE});
    auto res = cut.communicate().first;


    printf("%s\n",res.buf.data());
    //cpu cores
    string strCpuInfo = cpuName +" " + cpuCores + "核" + siblings + "线程 * ";
    if(res.buf.size() > 0)
        strCpuInfo = strCpuInfo + res.buf[0];
    return strCpuInfo;
}

std::string cdataProcess::get_cur_executable_path()
{
    char *p                 = NULL;

    const int len           = 256;
    /// to keep the absolute path of executable's path
    char arr_tmp[len]       = {0};

    int n                   = readlink("/proc/self/exe", arr_tmp, len);
    if (NULL                != (p = strrchr(arr_tmp,'/')))
        *p = '\0';
    else
    {
        printf("wrong process path");
        std::string("");
    }

    return std::string(arr_tmp);
}