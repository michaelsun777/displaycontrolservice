#ifndef _CNV_CONTROL_EVENTS_H
#define _CNV_CONTROL_EVENTS_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


#include <pthread.h>
#include "cspdlog.h"
#include <atomic>

#include <X11/Xlib.h>
#include "NVCtrlLib.h"


#define EVENT_TYPE_START TARGET_ATTRIBUTE_CHANGED_EVENT
#define EVENT_TYPE_END TARGET_BINARY_ATTRIBUTE_CHANGED_EVENT

class cmyxrandr;

struct target_info {
    int type;
    int count;
    unsigned int * pIds; // If Non-NULL, is list of target ids.
};

class CNvControlEvents {
private:
    pthread_t m_thread;
    pthread_t m_threadDeal;
    Display * m_display;
    bool m_bRunning;
    int m_event_base;
    std::atomic<int> m_AtomicCounter;
    cmyxrandr * m_pcmyxrandr;
private:
    static void * workerThreadListen(void * p);
    static void * workerThread(void * p);
public:
    CNvControlEvents(cmyxrandr *p);
    CNvControlEvents();
    ~CNvControlEvents(void);
    const char *target2str(int n);
    bool init();
    void start();
};



#endif