
#include "dlgManager.h"

std::shared_ptr<dlgManager> dlgManager::m_pdlgManager = NULL;
std::shared_ptr<dlgManager> dlgManager::GetInstance()
{
    if (m_pdlgManager == NULL)
    {
        lock_guard<mutex> m_lock(g_dlgManagerMutex);
        if (m_pdlgManager == NULL)
        {
            static std::once_flag s_flag;
            call_once(s_flag,[&]()
            {
                m_pdlgManager.reset(new dlgManager);
                m_pdlgManager->Init();
                //m_pdlgManager->writeSettings("key2", "value1,122,1,111,/home/sa/test/abc");
                m_pdlgManager->readSettings();


            });

            // if(m_pdlgManager->start())
            // {
            //     return m_pdlgManager;
            // }
            // else
            // {
            //     return NULL;
            // }
            
        }
        return m_pdlgManager;
    }
    return m_pdlgManager;
}



dlgManager::dlgManager(/* args */)
{
}

dlgManager::~dlgManager()
{
}

bool dlgManager::readSettings()
{
    /*
    QSettings settings("./dlgconfig.ini", QSettings::IniFormat);
    settings.beginGroup("Group");
    QStringList keys = settings.allKeys(); // 获取所有键
    for (const QString &key : keys) {
        // 根据键的类型获取相应的值
        QVariant value = settings.value(key);
        std::cout << key.toStdString() << ": " << value.toString().toStdString()<<std::endl;
    }
    settings.endGroup();
*/

}

bool dlgManager::writeSettings(string key,string value)
{
    QSettings settings("./dlgconfig.ini", QSettings::IniFormat);
    settings.beginGroup("Group");
    settings.setValue(key.c_str(), value.c_str());
    settings.endGroup();
    settings.sync();


}


void dlgManager::Init()
{
    /*

    QSettings settings("./dlgconfig.ini", QSettings::IniFormat);

    // Example: Adding some data to a group in QSettings
    settings.beginGroup("Group");
    settings.setValue("key1", "value1,122,1,111,http://192.168.110.238:8000:/begin?abc=123,1920,1080,3840,2160,value1,122,1,111,\
http://192.168.110.238:8000:/begin?abc=123,1920,1080,3840,2160\
http://192.168.110.238:8000:/begin?abc=123,1920,1080,3840,2160,\
value1,122,1,111,http://192.168.110.238:8000:/begin?abc=123,1920,1080,3840,2160");
    settings.setValue("key2", "value1,122,1,111,/home/sa/test/abc,-d|-c|-f,1920,1080,3840,2160,value1,122,1,111,/home/sa/test/abc,-d|-c|-f,1920,1080,3840,2160");   
    settings.endGroup();
    settings.sync();
  */

}

bool dlgManager::QDlgShow(DlgUrl * qdlg,DLGINFO & dlg)
{
    //qdlg->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    qdlg->show();
    // QPoint p;
    // p.setX(dlg.xPos),p.setY(dlg.yPos);
    // QSize size;
    // size.setWidth(dlg.width),size.setHeight(dlg.height);
    // qdlg->SetMouseSignal(this,1,p,size);
    // qdlg->UpdateUrl(dlg.url.c_str());
    //qdlg->SetConnect(this);
    //emit mouseSignal();
    //qdlg->DelConnect(this);
    //qdlg->onMouseEventRequested(1,p,size);
    //usleep(200);
    //qdlg->onMouseEventRequested(2,p,size);
    //send sigal;
    //emit mouseSignal(atoi(dlg.dlgId.c_str()));
    usleep(500);
    //qdlg->UpdateUrl();

}

bool dlgManager::addDlg(DLGINFO &dlg)
{
    // if(dlg.dlgId.empty())
    // {
    //     boost::uuids::uuid a_uuid = boost::uuids::random_generator()(); 
    //     string uuid_string = boost::uuids::to_string(a_uuid);
    //     dlg.dlgId = uuid_string;
    // }
    // else
    // {
    //     if(m_mDlgProperty.find(dlg.dlgId) != m_mDlgProperty.end())
    //     {
    //         return false;
    //     }
    // }
    // m_mDlgProperty.insert(make_pair(dlg.dlgId,dlg));
    // DlgUrl * qdlg = new DlgUrl(atoi(dlg.dlgId.c_str()));
    // //qdlg->exec();
    // QDlgShow(qdlg,dlg);
    // m_mDlgs.insert(make_pair(dlg.dlgId,qdlg));
    


}
bool dlgManager::delDlg(DLGINFO &dlg)
{
}
bool dlgManager::modifyDlg(DLGINFO &dlg)
{
}
bool dlgManager::getDlg(DLGINFO &dlg)
{
}
bool dlgManager::getAllDlg(vector<DLGINFO> &vDlg)
{
}
