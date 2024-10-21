#include "cdataProcess.h"


cdataProcess::cdataProcess(/* args */)
{
    
}

cdataProcess::~cdataProcess()
{
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
    auto p = Popen({args}, output{PIPE});

    return false;
}

bool cdataProcess::GetMonitorsInfo()
{
   

    return true;
}

bool cdataProcess::SetMonitorsInfo()
{
    // Display *display;
    // display = XOpenDisplay(NULL);
    // Window window = DefaultRootWindow(display);
    // int monitors = 0;
    // XRRMonitorInfo *monitor;
    // std::map<XID,XRRMonitorInfo *>::iterator iter = m_mMonitors.begin();
    // monitor = iter->second;
    // monitor->width = 1440;
    // monitor->height = 900;
    // monitor->mwidth = 600;
    // monitor->mheight = 340;
    // XRRSetMonitor(display, window, monitor);

//     Status XRRSetScreenConfig (Display *dpy,
// 			   XRRScreenConfiguration *config,
// 			   Drawable draw,
// 			   int size_index,
// 			   Rotation rotation,
// 			   Time timestamp);

// /* added in v1.1, sorry for the lame name */
// Status XRRSetScreenConfigAndRate (Display *dpy,
// 				  XRRScreenConfiguration *config,
// 				  Drawable draw,
// 				  int size_index,
// 				  Rotation rotation,
// 				  short rate,
// 				  Time timestamp);





    return false;
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