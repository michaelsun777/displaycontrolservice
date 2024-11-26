
#include "autoDelete.h"


shared_ptr<CAutoDeleteManager> CAutoDeleteManager::m_pCAutoDeleteManager = NULL;

CAutoDeleteManager::CAutoDeleteManager()
{

}

CAutoDeleteManager::~CAutoDeleteManager()
{
    m_StopThread.exchange(true);

}



shared_ptr<CAutoDeleteManager> CAutoDeleteManager::GetInstance()
{
    if (m_pCAutoDeleteManager == NULL)
    {
        //使用资源管理类，在抛出异常的时候，资源管理类对象会被析构，析构总是发生的无论是因为异常抛出还是语句块结束。
        lock_guard<mutex> m_lock(g_CAutoDeleteManagerMutex); 
        static std::once_flag s_flag;
        call_once(s_flag, [&](){ m_pCAutoDeleteManager.reset(new CAutoDeleteManager); });

        if(m_pCAutoDeleteManager->Init())
        {
            return m_pCAutoDeleteManager;
        }
        else
            return NULL;
        
    }
    return m_pCAutoDeleteManager;
}

bool CAutoDeleteManager::AddNeedDeleteFileInfo(string fileDirectory,int32_t fileSaveDays)
{
    if(fileDirectory.length() <= 0)
        return false;



    NeedDeleteInfo dinfo;
    dinfo.fileFolder = fileDirectory.c_str();
    dinfo.fileSaveDays = fileSaveDays;

    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        m_NeedDeleteInfoList.push_back(dinfo);
    }

    

    return true;
}

bool CAutoDeleteManager::Init()
{
    m_StopThread.exchange(false);
    if(!m_threadProcess)
        pthread_create(&m_threadProcess,NULL,AutoDeleteThreadProcess,this);

    return true;
}

void * CAutoDeleteManager::AutoDeleteThreadProcess(void * p)
{
    CAutoDeleteManager *pThis = (CAutoDeleteManager *)p;
    bool bStop = true;
    bool bRunStop = false;
    while (!pThis->m_StopThread.compare_exchange_strong(bStop, bRunStop))
    {
        bStop = true;

        try
        {
            timeval now;
            gettimeofday(&now, NULL);
            int64_t nTime = now.tv_sec;
            nTime = nTime % 3600;//4320/天
            if (nTime == 0)
            {
                {
                    boost::unique_lock<boost::mutex> lock(pThis->m_mutex);
                    std::list<NeedDeleteInfo>::iterator it = pThis->m_NeedDeleteInfoList.begin();
                    for (; it != pThis->m_NeedDeleteInfoList.end(); it++)
                    {
                        pThis->auto_delete_from_directory(it->fileFolder, it->fileFolder.length(), it->fileSaveDays);
                    }
                }

                sleep(2);
            }
            else
            {
                sleep(1);
            }
        }
        catch(...)
        {
            XERROR("删除文件出错,error={}",errno);
        }        
    }
    return 0;
}


bool CAutoDeleteManager::auto_delete_from_directory(std::string & fileDirectory,size_t length,int32_t fileSaveDays)
{
    try
    {
        try
        {
            char szCmdBuf[1024] = {0};           // 用于存放Linux命令
            //char szLocalFileSaveDir[1000] = {0}; // 用于存放本地目录
            //int32_t iFileSaveDays = 15;          // 文件保留天数, 可由配置项决定, 这里设为0

            // memcpy(szLocalFileSaveDir, "/home/zhouzx/TestDir", strlen("/home/zhouzx/TestDir"));      // 文件的存放路径
            //memcpy(szLocalFileSaveDir, fileDirectory, length); // 文件的存放路径
            // 注意该删除命令的格式
            //snprintf(szCmdBuf, sizeof(szCmdBuf) - 1, "find %s -name \"*.txt\" -ctime +%d -exec rm -f {} \\;", szLocalFileSaveDir, iFileSaveDays);
            snprintf(szCmdBuf, sizeof(szCmdBuf) - 1, "find %s -name \"*.txt\" -ctime +%d -exec rm -f {} \\;", fileDirectory.c_str(), fileSaveDays);

            printf("Delete the file(s), exec: %s\n", szCmdBuf);

            system(szCmdBuf); // 执行删除命令

            return true;
        }
        catch (const std::exception &e)
        {
            std::string erromsg = e.what();
            XERROR("auto_delete_from_directory erro ={},erromsg :{}",errno,erromsg);
            return false;
        }
    }
    catch (...)
    {
        return false;
    }

    return false;
}