
#include "CNvControlEvents.h"
#include "cmyxrandr.h"



CNvControlEvents::CNvControlEvents(cmyxrandr *p) : m_pcmyxrandr(p)
{
}

CNvControlEvents::CNvControlEvents()
{

}

CNvControlEvents::~CNvControlEvents(void)
{
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

    m_display = XOpenDisplay(dpy_name);
    if (!m_display)
    {
        fprintf(stderr, "Cannot open display '%s'.\n", XDisplayName(dpy_name));
        return 1;
    }

    /*
     * check if the NV-CONTROL X extension is present on this X server
     */

    ret = XNVCTRLQueryExtension(m_display, &m_event_base, &error_base);
    if (ret != True)
    {
        fprintf(stderr, "The NV-CONTROL X extension does not exist on '%s'.\n",
                XDisplayName(dpy_name));
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
                fprintf(stderr, "Failed to query %s target count on '%s'.\n",
                        target2str(tinfo->type), XDisplayName(dpy_name));
                return 1;
            }
            tinfo->count = tinfo->pIds[0];
        }
        else
        {
            ret = XNVCTRLQueryTargetCount(m_display, tinfo->type, &tinfo->count);
            if (ret != True)
            {
                fprintf(stderr, "Failed to query %s target count on '%s'.\n",
                        target2str(tinfo->type), XDisplayName(dpy_name));
                return 1;
            }
        }
    }

    printf("Registering to receive events...\n");
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
                        printf("- The NV-CONTROL X not available on X screen "
                               "%d of '%s'.\n",
                               i, XDisplayName(dpy_name));
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
                        printf("- Unable to register to receive NV-CONTROL"
                               "events on '%s'.\n",
                               XDisplayName(dpy_name));
                        continue;
                    }

                    printf("+ Listening on X screen %d for "
                           "ATTRIBUTE_CHANGED_EVENTs.\n",
                           target_id);
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
                    printf("- Unable to register on %s %d for %ss.\n",
                           target2str(tinfo->type), target_id,
                           eventTypes[k].description);
                    continue;
                }

                printf("+ Listening on %s %d for %ss.\n",
                       target2str(tinfo->type), target_id, eventTypes[k].description);

                sources++;
            }
        }
    }

    printf("\n");
    printf("Listening on %d sources for NV-CONTROL X Events...\n", sources);

    return true;
}

void * CNvControlEvents::workerThreadListen(void * p)
{
    CNvControlEvents * pThis = (CNvControlEvents *)p;
    try
    {
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
                XINFO("CNvControlEvents::workerThreadListen event.type=%d",event.type);
                pThis->m_AtomicCounter.fetch_add(1);
            }

        }       

    }
    catch (...)
    {
        XERROR("CNvControlEvents::workerThreadListen error,exit");
    }


   

}

void * CNvControlEvents::workerThread(void * p)
{
    CNvControlEvents * pThis = (CNvControlEvents *)p;
    try
    {
        while (pThis->m_bRunning)
        {
            if (pThis->m_AtomicCounter > 0)
            {
                int n = pThis->m_AtomicCounter;
                sleep(2);
                if (n == pThis->m_AtomicCounter)
                {
                    pThis->m_AtomicCounter = 0;
                    pThis->m_pcmyxrandr->OnUpdate();
                }
                else
                {
                    continue;
                }
            }
        }
    }
    catch (...)
    {
        XERROR("CNvControlEvents::workerThread error,exit");
    }
}




void CNvControlEvents::start()
{
    m_bRunning = true;
    if(pthread_create(&m_thread, NULL, workerThreadListen, (void *)this) != 0)
    {
        printf("Failed to create thread\n");
    }

    if(pthread_create(&m_thread, NULL, workerThread, (void *)this) != 0)
    {
        printf("Failed to create thread\n");
    }

}
