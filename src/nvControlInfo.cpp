
#include "nvControlInfo.h"


nvControlInfo::nvControlInfo(/* args */)
{
}

nvControlInfo::~nvControlInfo()
{
}

bool nvControlInfo::getGpuInfo(MGPUINFOEX & gpu)
{
    bool bRet = getGpuTempture(gpu.vgpus);
    if(!bRet)
        return bRet;

    Bool ret;
    Display *dpy;
    /*
     * open a connection to the X server indicated by the DISPLAY
     * environment variable
     */
    dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        fprintf(stderr, "Cannot open display '%s'.\n", XDisplayName(NULL));
        return false;
    }

    /*
     * check if the NV-CONTROL X extension is present on this X server
     */

    ret = XNVCTRLQueryExtension(dpy, &m_event_base, &m_error_base);
    if (ret != True)
    {
        fprintf(stderr, "The NV-CONTROL X extension does not exist on '%s'.\n",
                XDisplayName(NULL));
        return false;
    }

    /*
     * query the major and minor extension version
     */

    ret = XNVCTRLQueryVersion(dpy, &m_major, &m_minor);
    if (ret != True)
    {
        fprintf(stderr, "The NV-CONTROL X extension does not exist on '%s'.\n",
                XDisplayName(NULL));
        return false;
    }

    /*
     * print statistics thus far
     */

    XINFO("NV-CONTROL X extension present\n");
    XINFO("  version        : {}.{}\n", m_major, m_minor);
    XINFO("  event base     : {}\n", m_event_base);
    XINFO("  error base     : {}\n", m_error_base);

    m_screensNum = ScreenCount(dpy);
    char *str;
    for (int i = 0; i < m_screensNum; i++)
    {
        if (XNVCTRLIsNvScreen(dpy, i))
        {
            XINFO("Screen {} supports the NV-CONTROL X extension\n", i);

            ret = XNVCTRLQueryStringAttribute(dpy, i,
                                              0, /* XXX not currently used */
                                              NV_CTRL_STRING_PRODUCT_NAME,
                                              &str);
            if (ret)
            {
                XINFO("  GPU            : {}", str);
                string Tmp(str);
                m_vGpuName.push_back(Tmp);
                XFree(str);
            }

            ret = XNVCTRLQueryStringAttribute(dpy, i,
                                              0, /* XXX not currently used */
                                              NV_CTRL_STRING_VBIOS_VERSION,
                                              &str);

            if (ret)
            {
                XINFO("  VideoBIOS      : {}\n", str);
                string Tmp(str);
                m_videoBIOS = str;
                XFree(str);
            }

            ret = XNVCTRLQueryStringAttribute(dpy, i,
                                              0, /* XXX not currently used */
                                              NV_CTRL_STRING_NVIDIA_DRIVER_VERSION,
                                              &str);

            if (ret)
            {
                XINFO("  Driver version : {}\n", str);
                string Tmp(str);
                m_gpuDriverVersion = str;
                XFree(str);
            }

        }
    }

    return true;
}


bool nvControlInfo::getGpuTempture(std::vector<MGPUINFO> &vMGPUINFO)
{
    //return true;
    CMDEXEC::CmdRes res;
    bool bret = CMDEXEC::Execute("nvidia-smi",res);
    if(!bret)
        return false;

    printf("%s\n",res.StdoutString.c_str());
    vector<string> vString;
    CMDEXEC::Stringsplit(res.StdoutString,'\n',vString);
    bool bIsIndex = false;
    for (size_t i = 0; i < vString.size(); i++)
    {
        
        if(CMDEXEC::StartsWith(vString[i].c_str(), "|==="))
        {
            bIsIndex = !bIsIndex;
            continue;
        }

        if( bIsIndex == true && CMDEXEC::StartsWith(vString[i].c_str(), "| "))
        {
            
            if(vString[i].find(" NVIDIA ") != string::npos)
            {
                MGPUINFO gInfo;
                {
                    string strLine = vString[i].c_str();
                    XINFO("{}", strLine);
                    strLine = CMDEXEC::StripBrackets(strLine, '|', '|');
                    vector<string> vLineTmp = CMDEXEC::Split(strLine, '|');
                    string strItem = vLineTmp[0];
                    strItem = CMDEXEC::Strip(strItem);
                    if (strItem.find(' ') != string::npos)
                    {
                        size_t nPos = strItem.find(' ');
                        string strGpuNum = strItem.substr(0, nPos);
                        size_t nPosEnd = strItem.find("    ");
                        string strGpuName = strItem.substr(nPos + 1, nPosEnd);
                        gInfo.name = CMDEXEC::Strip(strGpuName);
                        gInfo.id = atoi(strGpuNum.c_str());
                        XINFO("GPU num:{},Name:{}", strGpuNum, strGpuName);
                    }
                }
                
                
                string strLineNext = vString[i+1].c_str();
                strLineNext = CMDEXEC::StripBrackets(strLineNext,'|','|');
                XINFO("{}",strLineNext);
                //vector<string> vLineTmpNext = CMDEXEC::Split(strLineNext,'|');
                //XINFO("itme0:{}",vLineTmpNext[0]);
                {
                    //strLineNext = CMDEXEC::StripBrackets(strLineNext,'|','|');
                    vector<string> vLineTmp = CMDEXEC::Split(strLineNext,'|');

                    string strFirst = vLineTmp[0];
                    strFirst = CMDEXEC::Strip(strFirst);
                    if (strFirst.find(' ') != string::npos)
                    {
                        size_t nPos = strFirst.find(' ');
                        string strGpuFan = strFirst.substr(0, nPos);

                        size_t nPosEnd = strFirst.find("   ",nPos+1);
                        string strGpuTempture = strFirst.substr(nPos + 1, nPosEnd);
                        size_t nPosEndPerf = strFirst.find("   ",nPosEnd+1);
                        string strPerf = strFirst.substr(nPosEnd,nPosEndPerf);
                        string strlast = strFirst.substr(nPosEndPerf+strPerf.length()+1,strFirst.length()-1);
                        string strMem = vLineTmp[1];
                        string strUtil = "";
                        
                        string util = CMDEXEC::Strip(vLineTmp[2]);
                        if(util.find('%') != string::npos)
                        {
                            size_t nUtilPos = util.find('%');
                            strUtil = util.substr(0,nUtilPos+1);

                        }
                        
                        
                        


                        gInfo.Mem = CMDEXEC::Strip(strMem);
                        gInfo.Pwr = CMDEXEC::Strip(strlast);
                        gInfo.Perf = CMDEXEC::Strip(strPerf);
                        gInfo.Temp = CMDEXEC::Strip(strGpuTempture);
                        gInfo.Fan = CMDEXEC::Strip(strGpuFan);
                        gInfo.Util = CMDEXEC::Strip(strUtil);


                        vMGPUINFO.push_back(gInfo);
                        XINFO("GPU Fan:{},Tempture:{},strPerf:{},pwr:{},strMem:{},strUtil:{}", strGpuFan, strGpuTempture,strPerf,strlast,strMem,strUtil);
                        i++;
                    }

                }

            }

        }
        else
            continue;

    }
    
    return true;

}