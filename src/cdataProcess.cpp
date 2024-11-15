//#include "dlgManager.h"
#include "cdataProcess.h"
#include <sys/utsname.h>
#include <sys/statfs.h>
#include <QFile>


cdataProcess::cdataProcess(/* args */)
{
    m_nWidth = 0,m_nHight = 0;
    m_layout_w = 0,m_layout_h = 0;

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
    m_layout_w = settings.value("screen/layout_horizontal",0).toInt();
    m_layout_h = settings.value("screen/layout_vertical",0).toInt();
    m_allLayouts = settings.value("screen/allResolution","").toString().toStdString();

    if(m_nWidth == 0)
    {
        m_nWidth = 1920;
        m_nHight = 1080;
    }
  
    if(m_layout_w ==  0 || m_layout_h == 0)
    {
        XERROR("请先在config.ini中配置layout_horizontal和layout_vertical");
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
        js["horizontal"] = m_layout_w;
        js["vertial"] = m_layout_h;

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

        if(vOutputInfo.size() < m_layout_w * m_layout_h)
        {
            for (size_t i = vOutputInfo.size(); i < m_layout_w * m_layout_h; i++)
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

bool cdataProcess::GetOutputsInfo_shell(json & js)
{
    string strDisplayName = ":0";
    cmyxrandr cxr(strDisplayName);
    //XRRScreenSize * psize = cxr.getCurrentConfigSizes();
    CMYSIZE currentSize, maxSize;
    vector<MOutputInfo> vOutputInfo;  
    short shRet = cxr.getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
    if(shRet == 0)
    {
        int sortBuf[m_layout_h * m_layout_w] = {0};
        char bufferLayout[40] = {0};
        sprintf(bufferLayout, "%dx%d", m_layout_w,m_layout_h);
        string layoutName = bufferLayout;
        js["layoutName"] = layoutName;
        char buffer[40] = {0};
        sprintf(buffer, "%dx%d", m_nWidth, m_nHight);
        string resolution = buffer;
        js["resolution"] = resolution;
        js["allResolution"] = m_allLayouts;
        js["layout_horizontal"] = m_layout_h;
        js["layout_vertical"] = m_layout_w;
        js["num"] = vOutputInfo.size();
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
            node["name"] = vOutputInfo[i].name;
            node["coordinateOrderX"] = (vOutputInfo[i].pos.xPos / m_nWidth);
            node["coordinateOrderY"] = (vOutputInfo[i].pos.yPos / m_nHight);
            int id = (vOutputInfo[i].pos.xPos / m_nWidth) + (vOutputInfo[i].pos.yPos / m_nHight) * m_layout_w;
            sortBuf[id] = 1;
            node["id"] = id;
            node["primary"] = vOutputInfo[i].primary;
            if(vOutputInfo[i].connected)
            {
                node["connected"] = true;                
            }
            jsdata.push_back(node);
        }

        if(vOutputInfo.size() < m_layout_w * m_layout_h)
        {
            for (size_t i = vOutputInfo.size(); i < m_layout_w * m_layout_h; i++)
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

bool cdataProcess::SetOutputsInfo(json & js)
{
    XINFO("{}",js.dump().c_str());
    if(js.find("layoutName") == js.end())
    {
        XERROR("layoutName is not exist");
        return false;
    }
        
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

    string layoutName = js["layoutName"].get<std::string>();
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
    int _layout_w = js["layout_horizontal"].get<int>();
    int _layout_h = js["layout_vertical"].get<int>();

    string strDisplayName = ":0";
    cmyxrandr cxr(strDisplayName);
    CMYSIZE currentSize, maxSize;
    vector<MOutputInfo> vOutputInfo;  
    short shRet = cxr.getAllScreenInfoEx(vOutputInfo,currentSize,maxSize);
    if(shRet != 0)
    {
        return false;
    }


    for (json::iterator it = jarry.begin();it!=jarry.end();it++)
    {
        string output = (*it)["output"].template get<std::string>();
        int nCoordinateX = 0;
        int nCoordinateY = 0;
        if((*it).find("coordinateOrderX") != (*it).end() && (*it).find("coordinateOrderY") != (*it).end())
        {
            nCoordinateX = (*it)["coordinateOrderX"].template get<int>();
            nCoordinateY = (*it)["coordinateOrderY"].template get<int>();
            nCoordinateX = nCoordinateX * _width;
            nCoordinateY = nCoordinateY * _hight;
        }
        else
        {
            int nId = (*it)["id"].template get<int>();
            nCoordinateX = (nId % _layout_w) * _width;
            nCoordinateY = (nId / _layout_w) * _hight;
        }               
        
        bool primary = (*it)["primary"].template get<bool>();
        for (size_t i = 0; i < vOutputInfo.size(); i++)
        {
            if(output.compare(vOutputInfo[i].name) == 0)
            {
                MyModelInfoEX * pMode = &vOutputInfo[i].currentMode;
                if(vOutputInfo[i].currentMode.name.compare(resolution) != 0)
                {
                    for (size_t i = 0; i < vOutputInfo[i].modes.size(); i++)
                    {
                        if(vOutputInfo[i].modes[i].name.compare(resolution) == 0)
                        { 
                            pMode = &vOutputInfo[i].modes[i];    
                            if(setOutputMode(vOutputInfo[i].name, vOutputInfo[i].modes[i].name,vOutputInfo[i].modes[i].rate))
                            {
                                return false;
                            }
                            break;
                        }
                    }
                }

                if (vOutputInfo[i].pos.xPos != nCoordinateX || vOutputInfo[i].pos.yPos != nCoordinateY)
                {
                    //if(!setOutputPos(vOutputInfo[i].name, nCoordinateX, nCoordinateY))
                    if(!setOutputModeAndPos(vOutputInfo[i].name, pMode->name,pMode->rate,nCoordinateX, nCoordinateY))
                    {
                        return false;
                    }
                }
                break;
            }
        }
    }

    m_layout_h = _layout_h;
    m_layout_w = _layout_w;
    m_nWidth = _width;
    m_nHight = _hight;
    m_allLayouts = allResolution;

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

bool cdataProcess::setOutputMode(string &strOutputName, string &strModeName,string & strRate)
{

    string strCmd = "xrandr --output ";
    strCmd += strOutputName;
    strCmd += " --mode ";
    strCmd += strModeName;
    // strCmd += " --pos ";
    // strCmd += strPos;
    strCmd += " --rate ";
    strCmd += strRate;
    XINFO("exec :{}", strCmd.c_str());
    CMDEXEC::CmdRes res;
    bool bRet = CMDEXEC::Execute(strCmd, res);
    if (!bRet)
    {
        XERROR("cdataProcess::setOutputMode CMDEXEC::Execute errono:{},error:{}", res.ExitCode, res.StderrString);
    }
    usleep(500000);
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
    XINFO("exec :{}", strCmd.c_str());
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

bool cdataProcess::setOutputModeAndPos(string &strOutputName,string &strModeName,string & strRate, int start_x,int start_y)
{    
    char buf[20] = {0};
    sprintf(buf, "%dx%d", start_x, start_y);
    string strPos = buf;
    string strCmd = "xrandr --output ";
    strCmd += strOutputName;
    strCmd += " --mode ";
    strCmd += strModeName;
    strCmd += " --pos ";
    strCmd += strPos;
    strCmd += " --rate ";
    strCmd += strRate;
   
    XINFO("exec :{}", strCmd.c_str());

    CMDEXEC::CmdRes res;
    bool bRet = CMDEXEC::Execute(strCmd, res);
    if (!bRet)
    {
        XERROR("cdataProcess::setOutputModeAndPos CMDEXEC::Execute errono:{},error:{}", res.ExitCode, res.StderrString);
        return false;
    }
    return true;
}

bool cdataProcess::InitOutputInfo()
{
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
            SetOutputsInfo(jdata);
            return true;
        }
    }

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
        if(vOutputInfo[i].currentMode.id != lastDeterminedModeId)//先设置mode
        {
            string strOutputName = vOutputInfo[i].name;            
            string strCmd = "xrandr --output ";
            strCmd += strOutputName;
            strCmd += " --mode ";
            strCmd += lastDeterminedModeName;
            //strCmd += " --pos ";
            //strCmd += strPos;
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

        {

            if ((i - (j * m_layout_w)) >= m_layout_w)
            {
                j++;
            }

            start_x = (i - (j * m_layout_w)) * m_nWidth;
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
            strCmd += " --rate ";
            strCmd += lastDeterminedModeRate;
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

       
    }
    
    return true;

}





bool cdataProcess::GetOutputAndGpuName(json & js)
{
        
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

    /* Get the number of gpus in the system */
    int num_gpus = 0;
    ret = XNVCTRLQueryTargetCount(dpy, NV_CTRL_TARGET_TYPE_GPU,&num_gpus);
    if (!ret)
    {
        XERROR("Failed to query number of gpus\n\n");
        XCloseDisplay(dpy);
        return false;
    }
    XINFO("number of GPUs: {}\n", num_gpus);

    /* Probe and list the Display devices */

    for (int i = 0; i < num_gpus; i++)
    {
        json node;

        int deprecated;
        int *pData;

        /* Get the gpu name */
        char *gpuName = nullptr;
        ret = XNVCTRLQueryTargetStringAttribute(dpy, NV_CTRL_TARGET_TYPE_GPU, i, 0,NV_CTRL_STRING_PRODUCT_NAME, &gpuName);
        if (!ret)
        {
            XERROR("Failed to query gpu name\n\n");
            XCloseDisplay(dpy);
            return false;
        }

        /* Probe the GPU for new/old display devices */
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
        

        /* Report results */
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