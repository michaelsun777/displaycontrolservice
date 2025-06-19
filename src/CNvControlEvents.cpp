#include "cdataProcess.h"
#include <QApplication>
#include "CNvControlEvents.h"
#include "cmyxrandr.h"
#include <systemd/sd-journal.h>




CNvControlEvents::CNvControlEvents()
{
    m_AtomicCounter = 0;
    m_display = NULL;
}

CNvControlEvents::~CNvControlEvents(void)
{
    if(m_display)
    {
        XCloseDisplay(m_display);
        m_display = NULL;
    }
}

/*
 * target2str() - translate a target type into a string
 */
const char *CNvControlEvents::target2str(int n)
{
    static char unknown[24];

    switch (n)
    {
    case NV_CTRL_TARGET_TYPE_X_SCREEN:
        return "X Screen";
    case NV_CTRL_TARGET_TYPE_GPU:
        return "GPU";
    case NV_CTRL_TARGET_TYPE_DISPLAY:
        return "Display";
    case NV_CTRL_TARGET_TYPE_FRAMELOCK:
        return "Frame Lock";
    case NV_CTRL_TARGET_TYPE_COOLER:
        return "Cooler";
    case NV_CTRL_TARGET_TYPE_THERMAL_SENSOR:
        return "Thermal Sensor";
    case NV_CTRL_TARGET_TYPE_3D_VISION_PRO_TRANSCEIVER:
        return "3D Vision Pro Transceiver";
    default:
        snprintf(unknown, 24, "Unknown (%d)", n);
        return unknown;
    }
}

bool CNvControlEvents::init()
{
    //Display *dpy;
    Bool ret;
    //int event_base, 
    int error_base;
    int i, j, k;
    int sources = 0;
    struct target_info info[] = {
        {.type = NV_CTRL_TARGET_TYPE_X_SCREEN},
        {.type = NV_CTRL_TARGET_TYPE_GPU},
        {.type = NV_CTRL_TARGET_TYPE_DISPLAY},
        {.type = NV_CTRL_TARGET_TYPE_FRAMELOCK},
        {.type = NV_CTRL_TARGET_TYPE_COOLER},
        {.type = NV_CTRL_TARGET_TYPE_THERMAL_SENSOR},
        {.type = NV_CTRL_TARGET_TYPE_3D_VISION_PRO_TRANSCEIVER},
    };

    static const int num_target_types = sizeof(info) / sizeof(*info);

    int c;
    char *dpy_name = NULL;
    Bool anythingEnabled;

#define EVENT_TYPE_ENTRY(_x) [_x] = {false, #_x}

    struct
    {
        Bool enabled;
        char *description;
    } eventTypes[] = {
        {false, "TARGET_ATTRIBUTE_CHANGED_EVENT"},
        {false, "TARGET_ATTRIBUTE_AVAILABILITY_CHANGED_EVENT"},
        {false, "TARGET_STRING_ATTRIBUTE_CHANGED_EVENT"},
        {false, "TARGET_BINARY_ATTRIBUTE_CHANGED_EVENT"},
        // EVENT_TYPE_ENTRY(TARGET_ATTRIBUTE_CHANGED_EVENT),
        // EVENT_TYPE_ENTRY(TARGET_ATTRIBUTE_AVAILABILITY_CHANGED_EVENT),
        // EVENT_TYPE_ENTRY(TARGET_STRING_ATTRIBUTE_CHANGED_EVENT),
        // EVENT_TYPE_ENTRY(TARGET_BINARY_ATTRIBUTE_CHANGED_EVENT),
    };

    eventTypes[TARGET_ATTRIBUTE_AVAILABILITY_CHANGED_EVENT].enabled = True;

    anythingEnabled = false;
    for (i = EVENT_TYPE_START; i <= EVENT_TYPE_END; i++)
    {
        if (eventTypes[i].enabled)
        {
            anythingEnabled = True;
            break;
        }
    }

    /*
     * Open a display connection, and make sure the NV-CONTROL X
     * extension is present on the screen we want to use.
     */
    if(m_display)
    {
        XCloseDisplay(m_display);
        m_display = NULL;
    }

    m_display = XOpenDisplay(dpy_name);
    if (!m_display)
    {
        fprintf(stderr, "Cannot open display '%s'.\n", XDisplayName(dpy_name));
        XERROR("Cannot open display '{}'.\n",XDisplayName(dpy_name));
        XINFO("CNvControlEvents::init exec systemctl restart gdm start!");
        CMDEXEC::CmdRes res;
        bool bret = CMDEXEC::Execute("systemctl restart gdm",res);
        if (!bret)
        {
            XINFO("CNvControlEvents::init exec systemctl restart gdm end!");
            exit(0);
            //return false;
        }
        return 1;
    }

    /*
     * check if the NV-CONTROL X extension is present on this X server
     */

    ret = XNVCTRLQueryExtension(m_display, &m_event_base, &error_base);
    if (ret != True)
    {
        fprintf(stderr, "The NV-CONTROL X extension does not exist on '%s'.\n", XDisplayName(dpy_name));
        XERROR("The NV-CONTROL X extension does not exist on '{}'.\n", XDisplayName(dpy_name));
        XCloseDisplay(m_display);
        return 1;
    }

    /* Query target counts */
    for (i = 0; i < num_target_types; i++)
    {

        struct target_info *tinfo = &info[i];

        if (tinfo->type == NV_CTRL_TARGET_TYPE_DISPLAY)
        {
            ret = XNVCTRLQueryTargetBinaryData(m_display, NV_CTRL_TARGET_TYPE_X_SCREEN,
                                               0, 0,
                                               NV_CTRL_BINARY_DATA_DISPLAY_TARGETS,
                                               (unsigned char **)&(tinfo->pIds),
                                               &(tinfo->count));
            if (ret != True)
            {
                fprintf(stderr, "Failed to query %s target count on '%s'.\n", target2str(tinfo->type), XDisplayName(dpy_name));
                XERROR("Failed to query {} target count on '{}'.\n", target2str(tinfo->type), XDisplayName(dpy_name));
                XCloseDisplay(m_display);
                return 1;
            }
            tinfo->count = tinfo->pIds[0];
        }
        else
        {
            ret = XNVCTRLQueryTargetCount(m_display, tinfo->type, &tinfo->count);
            if (ret != True)
            {
                fprintf(stderr, "Failed to query %s target count on '%s'.\n", target2str(tinfo->type), XDisplayName(dpy_name));
                XERROR("Failed to query {} target count on '{}'.\n", target2str(tinfo->type), XDisplayName(dpy_name));
                XCloseDisplay(m_display);
                return 1;
            }
        }
    }

    printf("Registering to receive events...\n");
    XINFO("Registering to receive events...\n");
    fflush(stdout);

    /* Register to receive events on all targets */

    for (i = 0; i < num_target_types; i++)
    {
        struct target_info *tinfo = &info[i];

        for (j = 0; j < tinfo->count; j++)
        {
            int target_id;

            if (tinfo->pIds)
            {
                target_id = tinfo->pIds[1 + j];
            }
            else
            {
                target_id = j;
            }

            for (k = EVENT_TYPE_START; k <= EVENT_TYPE_END; k++)
            {
                if (!eventTypes[k].enabled)
                {
                    continue;
                }

                if ((k == TARGET_ATTRIBUTE_CHANGED_EVENT) &&
                    (tinfo->type == NV_CTRL_TARGET_TYPE_X_SCREEN))
                {

                    /*
                     * Only register to receive events if this screen is
                     * controlled by the NVIDIA driver.
                     */
                    if (!XNVCTRLIsNvScreen(m_display, target_id))
                    {
                        printf("- The NV-CONTROL X not available on X screen %d of '%s'.\n", i, XDisplayName(dpy_name));
                        XINFO("- The NV-CONTROL X not available on X screen {} of '{}'.\n", i, XDisplayName(dpy_name));
                        continue;
                    }

                    /*
                     * - Register to receive ATTRIBUTE_CHANGE_EVENT events.
                     *   These events are specific to attributes set on X
                     *   Screens.
                     */

                    ret = XNVCtrlSelectNotify(m_display, target_id, ATTRIBUTE_CHANGED_EVENT,
                                              True);
                    if (ret != True)
                    {
                        printf("- Unable to register to receive NV-CONTROL events on '%s'.\n", XDisplayName(dpy_name));
                        XINFO("- Unable to register to receive NV-CONTROL events on '{}'.\n", XDisplayName(dpy_name));
                        continue;
                    }

                    printf("+ Listening on X screen %d for ATTRIBUTE_CHANGED_EVENTs.\n", target_id);
                    XINFO("+ Listening on X screen {} for ATTRIBUTE_CHANGED_EVENTs.\n", target_id);
                    sources++;
                }

                /*
                 * - Register to receive TARGET_ATTRIBUTE_CHANGED_EVENT events.
                 *   These events are specific to attributes set on various
                 *   devices and structures controlled by the NVIDIA driver.
                 *   Some possible targets include X Screens, GPUs, and Frame
                 *   Lock boards.
                 */

                ret = XNVCtrlSelectTargetNotify(m_display,
                                                tinfo->type, /* target type */
                                                target_id,   /* target ID */
                                                k,           /* eventType */
                                                True);
                if (ret != True)
                {
                    printf("- Unable to register on %s %d for %ss.\n", target2str(tinfo->type), target_id, eventTypes[k].description);
                    XINFO("- Unable to register on {} {} for {}s.\n", target2str(tinfo->type), target_id, eventTypes[k].description);
                    continue;
                }

                printf("+ Listening on %s %d for %ss.\n", target2str(tinfo->type), target_id, eventTypes[k].description);
                XINFO("+ Listening on {} {} for {}s.\n", target2str(tinfo->type), target_id, eventTypes[k].description);

                sources++;
            }
        }
    }

    //printf("\n");
    //printf("Listening on %d sources for NV-CONTROL X Events...\n", sources);
    XINFO("Listening on %d sources for NV-CONTROL X Events...\n", sources);

    return true;
}

void * CNvControlEvents::workerThreadListen(void * p)
{
    CNvControlEvents * pThis = (CNvControlEvents *)p;
    try
    {
        sleep(30);
        while (pThis->m_bRunning)
        {
            XEvent event;
            const char *target_str;

            XNextEvent(pThis->m_display, &event);

            if (event.type == (pThis->m_event_base + ATTRIBUTE_CHANGED_EVENT) 
            || event.type == (pThis->m_event_base + TARGET_ATTRIBUTE_CHANGED_EVENT)
            ||event.type == (pThis->m_event_base + TARGET_ATTRIBUTE_AVAILABILITY_CHANGED_EVENT)
            ||event.type == (pThis->m_event_base + TARGET_STRING_ATTRIBUTE_CHANGED_EVENT)
            ||event.type == (pThis->m_event_base + TARGET_BINARY_ATTRIBUTE_CHANGED_EVENT)
            )
            {  
                XINFO("CNvControlEvents::workerThreadListen event.type={}\n",event.type);
                pThis->m_AtomicCounter.fetch_add(1);
            }

        }       
        return 0;
    }
    catch (...)
    {
        XERROR("CNvControlEvents::workerThreadListen error,exit");
    }
    return 0; 

}

void * CNvControlEvents::xcb_Listen(void * p)
{
    xcb_connection_t *connection = NULL;
    CNvControlEvents * pThis = (CNvControlEvents *)p;
    try
    {
        connection = xcb_connect(NULL, NULL);
        const xcb_setup_t *setup = xcb_get_setup(connection);
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
        xcb_screen_t *pscreen = iter.data;

        // 获取RandR扩展版本
        xcb_randr_query_version_reply_t *version_reply = xcb_randr_query_version_reply(connection, xcb_randr_query_version(connection, 1, 2), NULL);
        if (version_reply) {
            free(version_reply);
        }


    

        //xcb_get_extension_data
        // const xcb_query_extension_reply_t *randr_ext = xcb_get_extension_data(connection, &xcb_randr_id);
        // if (!randr_ext->present)
        // {
        //     printf("X server does not support the RANDR extension.\n");
        //     return 0;
        // }

        // 
        // XCB_RANDR_NOTIFY
        // 获取RANDR扩展信息
        // int major_opcode, first_event, first_error;
        // xcb_connection_t *c, xcb_extension_t *ext;
        // int status = xcb_get_extension_data();

        
        
        //XCB_RANDR_NOTIFY_RESIZE
        uint32_t mask = XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE;
        // 请求配置通知
        xcb_void_cookie_t cookie = xcb_randr_select_input_checked(connection, pscreen->root, mask);
        xcb_flush(connection);
        // 检查请求是否成功
        xcb_generic_error_t* error = xcb_request_check(connection, cookie);
        if (error) {
            std::cerr << "Failed to select RANDR input event" << std::endl;
            XERROR("Failed to select RANDR input event");
            //xcb_free_error(error);
            //delete error;
            free(error);
            xcb_disconnect(connection);
            return 0;
        }

        
        //XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE
        //xcb_randr_select_input_checked(connection, pscreen->root, XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE);
        //xcb_flush(connection);
        

        xcb_generic_event_t *event;
        while ((event = xcb_wait_for_event(connection)))
        {
            switch (event->response_type & ~0x80)
            {
            // case XCB_RANDR_SCREEN_CHANGE_NOTIFY:
            // {
            //     // 解析屏幕变更事件
            //     xcb_randr_screen_change_notify_event_t *scne = (xcb_randr_screen_change_notify_event_t *)event;
            //     printf("Screen resolution changed to %dx%d\n", scne->width, scne->height);
            //     break;
            // }
            default:
                 {//XCB_EVENT_MASK_EXPOSURE
                    //xcb_randr_notify_event_t* notify_event = reinterpret_cast<xcb_randr_notify_event_t*>(event);
                    if(event->response_type == 89)
                    {
                        printf("%d,", event->response_type);
                        xcb_randr_screen_change_notify_event_t *xcbevent = (xcb_randr_screen_change_notify_event_t *)event;
                        printf("22222222222分辨率改变:%dx%d\n", xcbevent->width, xcbevent->height);
                        pThis->m_AtomicCounter.fetch_add(1);
                    }
                }
                break;
            }

            
            free(event);
        }
        return 0;
    }
    catch(...)
    {
        XERROR("CNvControlEvents::xcb_Listen error,exit");
        xcb_disconnect(connection);
    }
    return 0;

}

void * CNvControlEvents::workerThread(void * p)
{
    CNvControlEvents * pThis = (CNvControlEvents *)p;
    try
    {
        sleep(30);
        while (pThis->m_bRunning)
        {
            int n = pThis->m_AtomicCounter;
            if (n > 0)
            {
                sleep(2);
            }                
            else
            {
                sleep(3);
                continue;
            }

            if (n == pThis->m_AtomicCounter)
            {
                usleep(5000 * 1000);
                cdataProcess * pcdataProcess = cdataProcess::GetInstance();
                //pcdataProcess->OnCheckAndUpdate();                
                pThis->m_AtomicCounter = 0;                
            }           
        }
        return 0;
    }
    catch (...)
    {
        XERROR("CNvControlEvents::workerThread error,exit");
    }
    return 0;
}

void * CNvControlEvents::workerThreadForSystemLog(void * p)
{
    CNvControlEvents * pThis = (CNvControlEvents *)p;
    sd_journal *journal;
    try
    {        
        sleep(5);
        int ret;

        // 打开 journald 日志
        ret = sd_journal_open(&journal, SD_JOURNAL_LOCAL_ONLY);
        if (ret < 0)
        {
            std::cerr << "无法打开 journald 日志: " << strerror(-ret) << std::endl;
            return 0;
        }

        // 跳转到日志末尾
        ret = sd_journal_seek_tail(journal);
        if (ret < 0)
        {
            std::cerr << "无法跳转到日志末尾: " << strerror(-ret) << std::endl;
            sd_journal_close(journal);
            return 0;
        }
        list<string> _stringlist;

        // 监听新日志条目
        while (true)
        {
            ret = sd_journal_next(journal);
            if (ret < 0)
            {
                std::cerr << "读取日志条目失败: " << strerror(-ret) << std::endl;
                break;
            }

            if (ret == 0)
            {
                if(_stringlist.size() > 0)
                {
                    size_t sloop = 0;
                    bool bCaseConnected = false; 
                    bool bCaseDisconnected = false; 
                    for (list<string>::iterator it = _stringlist.begin(); it != _stringlist.end(); it++)
                    {
                        if(sloop > 1)
                            break;

                        if(sloop == 0 && it->find("NVIDIA(GPU") != string::npos && it->find(": connected") != string::npos)
                        {
                            bCaseConnected = true;
                            sloop++;
                            continue;
                        }
                        else if(sloop == 0 && it->find("NVIDIA(GPU") != string::npos && it->find(": disconnected") != string::npos)
                        {
                            bCaseDisconnected = true;
                            sloop++;
                            continue;
                        }

                        if (bCaseConnected || bCaseDisconnected)
                        {
                            if (it->find("NVIDIA(GPU") != string::npos && it->find(": Internal TMDS") != string::npos) // 插入新
                            {
                                pThis->m_AtomicCounter.fetch_add(1);
                                if (bCaseConnected)
                                    XERROR("显示器插入，{}\n", *it);
                                else
                                    XERROR("显示器拨出，{}\n", *it);
                            }
                        }
                        sloop++;
                        std::cout << *it << std::endl;
                    }
                    _stringlist.clear();
                }
                
                // 没有新日志，等待
                sleep(1);
                continue;
            }

            // 获取日志数据
            const void *data;
            size_t length;
            ret = sd_journal_get_data(journal, "MESSAGE", &data, &length);
            if (ret < 0)
            {
                std::cerr << "获取日志数据失败: " << strerror(-ret) << std::endl;
                continue;
            }

            // 输出日志消息
            string strData(reinterpret_cast<const char *>(data));
            _stringlist.push_back(strData);
            std::cout.write(reinterpret_cast<const char *>(data), length);
            std::cout << std::endl;
        }

        

        // 关闭 journald 日志
        sd_journal_close(journal);
    }
    catch(...)
    {
        sd_journal_close(journal);
        XERROR("workerThreadForSystemLog error,exit");
    }
    return 0;    
}




void CNvControlEvents::start()
{
    m_bRunning = true;
    pthread_t _tthread;
    // if(pthread_create(&m_threadlistenXcb, NULL, xcb_Listen, (void *)this) != 0)
    // {
    //     printf("Failed to create thread\n");
    // }
    if(pthread_create(&m_threadlistenNv, NULL, workerThreadListen, (void *)this) != 0)
    {
        printf("Failed to create thread\n");
    }

    if(pthread_create(&m_threadDeal, NULL, workerThread, (void *)this) != 0)
    {
        printf("Failed to create thread\n");
    }

    if(pthread_create(&m_threadlistenLog, NULL, workerThreadForSystemLog, (void *)this) != 0)
    {
        printf("Failed to create thread\n");
    }

}

bool CNvControlEvents::setIsChanged()
{
    m_AtomicCounter.fetch_add(1);
    return true;
}