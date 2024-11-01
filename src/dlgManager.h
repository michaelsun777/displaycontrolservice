#ifndef _DLG_MANAGER_H
#define _DLG_MANAGER_H

#include <QApplication>
#include "UI/dlgurl.h"
#include <QSettings>

#include <iostream>
#include <functional>
#include "cspdlog.h"
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread.hpp>
#include <errno.h>
#include "../3rd/json/include/nlohmann/json.hpp"
#include <map>


#include "common.h"
#include <vector>
#include <map>

#include "3rd/json/include/nlohmann/json.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>



using namespace std;
using json = nlohmann::json;

static mutex g_dlgManagerMutex;

class dlgManager : public QObject
{
    Q_OBJECT
private:
    static std::shared_ptr<dlgManager> m_pdlgManager;
    std::map<string,DLGINFO> m_mDlgProperty;
    std::map<string,DlgUrl *> m_mDlgs;
private:
    dlgManager(/* args */);
    bool readSettings();
    bool writeSettings(string key,string value);
    bool QDlgShow(DlgUrl * qdlg,DLGINFO & dlg);
public:    
    ~dlgManager();
    static shared_ptr<dlgManager> GetInstance();
    void Init();
    bool addDlg(DLGINFO & dlg);
    bool delDlg(DLGINFO & dlg);
    bool modifyDlg(DLGINFO & dlg);
    bool getDlg(DLGINFO & dlg);
    bool getAllDlg(vector<DLGINFO> &vDlg);

signals:
    void mouseSignal(int dlgId);



};




#endif