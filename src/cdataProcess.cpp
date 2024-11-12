//#include "dlgManager.h"
#include "cdataProcess.h"


cdataProcess::cdataProcess(/* args */)
{
    m_nWidth = 0,m_nHight = 0;
    CConfig config;
    config.SetFilePath("config.ini");
    string value = "";
	string error = "";
    config.GetValue("screen","width",value,error);
    m_nWidth = atoi(value.c_str());
    value.clear();
    config.GetValue("screen","height",value,error);
    m_nHight = atoi(value.c_str());
    if(m_nWidth == 0)
    {
        m_nWidth = 1920;
        m_nHight = 1080;
    }
    value.clear();
    config.GetValue("screen","distribution_horizontal",value,error);
    m_ndistribution_w =  atoi(value.c_str());
    value.clear();
    config.GetValue("screen","distribution_vertical",value,error);
    m_ndistribution_h =  atoi(value.c_str());
    if(m_ndistribution_w ==  0 || m_ndistribution_h == 0)
    {
        XERROR("Please set up the Configuration file first!");
        exit(0);
    }
        
}

cdataProcess::~cdataProcess()
{
}

void cdataProcess::SetMainWindow(MainWindow * p)
{
    m_pMainWindow = p;
}

void cdataProcess::print_display_name(Display *dpy, int target_id, int attr,char *name)
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

    print_display_name(dpy, target_id, NV_CTRL_STRING_DISPLAY_DEVICE_NAME,
                       name_str);
}

bool cdataProcess::Output(string args)
{
    //auto p = Popen({args}, output{PIPE});

    return false;
}

bool cdataProcess::GetMonitorsInfo(string & strInfo)
{
    json js;
    string strDisplayName = ":0";
    cmyxrandr cxr(strDisplayName);
    XRRScreenSize * psize = cxr.getCurrentConfigSizes();
    CMYSIZE min,max;
    cxr.getScreenSizeRange(min,max);
    //cxr.getScreenInfo();
    //short rate = cxr.getScreenRate();

    unsigned short	rotation = cxr.getCurrentConfigRotation();
   
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
    list<RROutput> outputs = cxr.getOutputs();
    

    for (list<RRCrtc>::iterator it = outputs.begin(); it != outputs.end(); it++)   
    {
        RROutput output = *it;
        cxr.setOutPut(output);
        // cmyxrandr rroutput(strDisplayName, output);
        XRROutputInfo *outinfo = cxr.GetOutputInfo();
        

        if (outinfo->nmode > 0)
        {
            nNum++;
            cxr.setCrtc(outinfo->crtc);
            XRRCrtcInfo *rcrtinfo = cxr.getCrtcInfo();
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
            node["primary"] = cxr.isPrimary();



            if (strStatus == "Connected")
            {
                node["connected"] = true;
                json jmodes;
                list<MyModelInfoEX *> modes = cxr.getOutputModes();
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
    string strDisplayName = ":0";
    cmyxrandr cxr(strDisplayName);
    //XRRScreenSize * psize = cxr.getCurrentConfigSizes();
    CMYSIZE currentSize, maxSize;
    vector<MOutputInfo> vOutputInfo;  
    short shRet = cxr.getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
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
        js["horizontal"] = m_ndistribution_w;
        js["vertial"] = m_ndistribution_h;

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

        if(vOutputInfo.size() < m_ndistribution_w * m_ndistribution_h)
        {
            for (size_t i = vOutputInfo.size(); i < m_ndistribution_w * m_ndistribution_h; i++)
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
        string strDisplayName = ":0";
        cmyxrandr cxr(strDisplayName);

        for (size_t i = 0; i < vSetInfo->size(); i++)
        {
            cxr.setOutPut((*vSetInfo)[i].outputId);
            RRCrtc rrcrtc = cxr.getCrtc();
            cxr.setCrtc(rrcrtc);
            CMYSIZE size((*vSetInfo)[i].size.width, (*vSetInfo)[i].size.height);
            cxr.setMode(size, 447);
        }

        for (size_t i = 0; i < vSetInfo->size(); i++)
        {
            cxr.setOutPut((*vSetInfo)[i].outputId);
            RRCrtc rrcrtc = cxr.getCrtc();
            cxr.setCrtc(rrcrtc);
            //CMYSIZE size((*vSetInfo)[i].size.width, (*vSetInfo)[i].size.height);
            //CMYPOINT offset((*vSetInfo)[i].pos.xPos * m_nWidth, (*vSetInfo)[i].pos.yPos * m_nHight);
            CMYPOINT offset((*vSetInfo)[i].pos.xPos, (*vSetInfo)[i].pos.yPos);
            cxr.setOffset(offset);
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


bool cdataProcess::TestMonitorInfo()
{
    // string strDisplayName = ":0";
    // cmyxrandr cxr(strDisplayName);
    // vector<MOutputInfo> vOutputInfo;
    // CMYSIZE currentSize, maxSize;
    // cxr.getAllScreenInfoEx(vOutputInfo,currentSize, maxSize);
    // nvControlInfo nv;
    // MGPUINFOEX  gpu;
    // nv.getGpuInfo(gpu);

    DLGINFO dlg;
    dlg.height = 1080;
    dlg.width = 3840;
    dlg.postype = 1;
    dlg.name = "test";
    dlg.xPos = 200;
    dlg.yPos = 50;
    dlg.url = "https://www.baidu.com";

    //std::shared_ptr<dlgManager> pdlgManager = dlgManager::GetInstance();
    //pdlgManager->addDlg(dlg);

    //m_pMainWindow->test();
    
    


    return false;
}

bool cdataProcess::GetGpuInfo(json & js)
{
    //json js;
    MGPUINFOEX gpu;
    nvControlInfo nv;
    if(nv.getGpuInfo(gpu))
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
        //strInfo = js.dump().c_str();
        return true;
    }
    return false;
}


bool cdataProcess::InitOutputInfo()
{
    string strDisplayName = ":0";
    cmyxrandr cxr(strDisplayName);
    vector<MOutputInfo> vOutputInfo;
    CMYSIZE currentSize, maxSize;
    cxr.getAllScreenInfoEx(vOutputInfo,currentSize, maxSize);

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
                            XINFO("we find a mode,Name={}, modeId={}, width:{}, height:{}, rate:{}",vOutputInfo[i].modes[j].name,
                            vOutputInfo[i].modes[j].id,vOutputInfo[i].modes[j].width,vOutputInfo[i].modes[j].height,vOutputInfo[i].modes[j].rate);
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


    for (size_t i = 0,j = 0; i < vOutputInfo.size(); i++)
    {
        if(vOutputInfo[i].currentMode.id != lastDeterminedModeId)
        {
            if((i - j*m_nWidth) < m_nWidth)
            {
                start_x = i*m_nWidth;
                start_y = j*m_nHight;
            }
            else
            {
                start_y++;
                start_x = (i - j*m_nWidth)*m_nWidth;
                start_y = j*m_nHight;
            }
            
            char buf[20] = {0};
            sprintf(buf,"%dx%d",start_x,start_y);
            string strPos = buf;

            string strOutputName = vOutputInfo[i].name;            
            string strCmd = "xrandr --output ";
            strCmd += strOutputName;
            strCmd += " --mode ";
            strCmd += lastDeterminedModeName;
            strCmd += " --pos ";
            strCmd += strPos;
            strCmd += " --rate ";      
            strCmd += lastDeterminedModeRate;
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

        }
        else
            continue;        
    }
    
    return true;

}





bool cdataProcess::GetMonitorsInfo_N()
{

    Display *dpy;
    Bool ret;
    int screen, major, minor, len;
    char *str, *start, *str0, *str1;
    int *enabledDpyIds;

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open display '%s'.\n\n", XDisplayName(NULL));
        return 1;
    }
    
    screen = GetNvXScreen(dpy);

    ret = XNVCTRLQueryVersion(dpy, &major, &minor);
    if (ret != True)
    {
        fprintf(stderr, "The NV-CONTROL X extension does not exist "
                        "on '%s'.\n\n",
                XDisplayName(NULL));
        return 1;
    }

    printf("\nUsing NV-CONTROL extension %d.%d on %s\n\n",
           major, minor, XDisplayName(NULL));

    /*
     * query the enabled display devices on this X screen and print basic
     * information about each X screen.
     */

    ret = XNVCTRLQueryTargetBinaryData(dpy,
                                       NV_CTRL_TARGET_TYPE_X_SCREEN,
                                       screen,
                                       0,
                                       NV_CTRL_BINARY_DATA_DISPLAYS_ENABLED_ON_XSCREEN,
                                       (unsigned char **)&enabledDpyIds,
                                       &len);
    if (!ret || (len < sizeof(enabledDpyIds[0])))
    {
        fprintf(stderr, "Failed to query the enabled Display Devices.\n\n");
        return 1;
    }

    printf("Enabled Display Devices:\n");
    m_nAvailableMonitorsCount = enabledDpyIds[0];
    XINFO("Enabled Display Devices:",(int)enabledDpyIds[0]);


    for (int i = 0; i < enabledDpyIds[0]; i++) 
    {
        int dpyId = enabledDpyIds[i+1];       

        print_display_id_and_name(dpy, dpyId, "  ");
    }

    printf("\n");
    
    
    /*
     * perform the requested action, based on the specified
     * commandline option
     */

    if(m_nAvailableMonitorsCount < 1)
    {
        return false;
    }
        

    return true;
}