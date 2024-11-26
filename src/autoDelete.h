#ifndef AUTO_DELETE_H
#define AUTO_DELETE_H


#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <string.h>

#include <list>
#include "cspdlog.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>


using namespace std;


struct NeedDeleteInfo
{
    std::string fileFolder;
    // 删除x天前的文件
    int32_t fileSaveDays;

};


static mutex g_CAutoDeleteManagerMutex;
class CAutoDeleteManager
{
private:
    std::list<NeedDeleteInfo> m_NeedDeleteInfoList;
    boost::mutex m_mutex;
    static shared_ptr<CAutoDeleteManager> m_pCAutoDeleteManager;
    pthread_t m_threadProcess;
    std::atomic<bool> m_StopThread;

private:    
    CAutoDeleteManager();

    bool Init();
    static void * AutoDeleteThreadProcess(void * p);
public:
    ~CAutoDeleteManager();
    static shared_ptr<CAutoDeleteManager> GetInstance();    
    bool AddNeedDeleteFileInfo(string fileDirectory,int32_t fileSaveDays);
    static bool auto_delete_from_directory(std::string & fileDirectory,size_t length,int32_t fileSaveDays);
    
    

};








#endif