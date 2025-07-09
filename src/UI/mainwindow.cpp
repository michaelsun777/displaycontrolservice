#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../cdataProcess.h"

#include "../common.h"
#include "../3rd/json/include/nlohmann/json.hpp"
#include <QTimer>
#include <chrono>
using namespace nlohmann;

static std::string GetCurrentDateTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);
    std::ostringstream oss;
    oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_isOpen(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    readSettings();
    connect(this,&MainWindow::initSignal,this,&MainWindow::onInitSlots);

}

MainWindow::~MainWindow()
{
    for (std::map<std::string,QCefWidget *> ::iterator it = m_mDlgs.begin(); it != m_mDlgs.end(); it++)
    {
        delete it->second;
    }

    for (std::map<std::string,QtDlgInfo *> ::iterator it = m_mDlgProperty.begin(); it != m_mDlgProperty.end(); it++)
    {
        delete it->second;
    }    

    delete ui;
}

bool MainWindow::parseJsonToDlgInfo(QtDlgInfo * info,string str)
{
    try
    {
        json jdata = json::parse(str);
        info->name = jdata["name"].template get<std::string>();
        info->dlgId = jdata["id"].template get<std::string>();
        info->url = jdata["url"].template get<std::string>();
        info->path = jdata["path"].template get<std::string>();
        info->param = jdata["param"].template get<std::string>();
        info->type = jdata["type"].template get<int>();
        info->posType = jdata["posType"].template get<int>();
        info->xVirtual = jdata["xVirtual"].template get<int>();
        info->yVirtual = jdata["yVirtual"].template get<int>();
        info->xPos = jdata["xPos"].template get<int>();
        info->yPos = jdata["yPos"].template get<int>();
        info->height = jdata["height"].template get<int>();
        info->width = jdata["width"].template get<int>();
        info->order = jdata["order"].template get<int>();

    }
    catch(...)
    {
        return false;
    }
    return true;
}

bool MainWindow::dlgInfoToJson(QtDlgInfo * info,string & str)
{
    json jdata;
    jdata["name"] = info->name;
    jdata["id"] = info->dlgId;
    jdata["url"] = info->url;
    jdata["path"] = info->path;
    jdata["param"] = info->param;
    jdata["type"] = info->type;
    jdata["posType"] = info->posType;
    jdata["xVirtual"] = info->xVirtual;
    jdata["yVirtual"] = info->yVirtual;
    jdata["xPos"] = info->xPos;
    jdata["yPos"] = info->yPos;
    jdata["height"] = info->height;
    jdata["width"] = info->width;
    jdata["order"] = info->order;
    jdata["show"] = info->show;
    str = jdata.dump();
    return true;
}

bool MainWindow::readSettings()
{
    m_mDlgProperty.clear();
    QSettings settings("./dlgconfig.ini", QSettings::IniFormat);
    settings.beginGroup("Group");
    if(settings.allKeys().size() > 0)
    {
        QStringList keys = settings.allKeys(); // 获取所有键
        for (const QString &key : keys)
        {
            // 根据键的类型获取相应的值
            QVariant value = settings.value(key);
            std::cout << key.toStdString() << ": " << value.toString().toStdString() << std::endl;
            QtDlgInfo * info = new QtDlgInfo;
            parseJsonToDlgInfo(info,value.toString().toStdString());
            
            info->createTime = GetCurrentDateTime();
            m_mDlgProperty.insert(make_pair(info->dlgId,info));
        }
    }    
    settings.endGroup();

    return true;
}

bool MainWindow::writeSettings(string key,string value)
{
    QSettings settings("./dlgconfig.ini", QSettings::IniFormat);
    settings.beginGroup("Group");
    settings.setValue(key.c_str(), value.c_str());
    settings.endGroup();
    settings.sync();
    return true;
}

bool MainWindow::deleteSettings(string key)
{
    QSettings settings("./dlgconfig.ini", QSettings::IniFormat);
    settings.beginGroup("Group");
    settings.remove(key.c_str());    
    settings.endGroup();
    settings.sync();
    return true;
}

bool MainWindow::QDlgShow(QCefWidget * qdlg,QtDlgInfo & info)
{
    qdlg->setWindowFlags(windowFlags() & ~Qt::WindowFullScreen | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::SubWindow);
    qdlg->UpdateSetting(&info);
    return true;

}

void MainWindow::test()
{    
    //emit testSignal();
}
void MainWindow::on_pbtn_test_clicked()
{
    //emit testSignal();
    // QCefWidget * qdlg = new QCefWidget();
    // QDlgShow(qdlg);
}

bool cmp(const QtDlgInfo * a, QtDlgInfo * b){	
	return a->order < b->order;			//按照值从小到大排序 
}



void MainWindow::onInitSlots()
{
    std::vector<QtDlgInfo *> vInfo;
    for (std::map<std::string,QtDlgInfo *> ::iterator it = m_mDlgProperty.begin(); it != m_mDlgProperty.end(); it++)
    {
        vInfo.push_back(it->second);
        // QCefWidget *qdlg = new QCefWidget(it->second->dlgId);
        // QDlgShow(qdlg, *it->second);        
        // m_mDlgs.insert(make_pair(it->second->dlgId, qdlg));
    }

    sort(vInfo.begin(),vInfo.end(),cmp);
    for (size_t i = 0; i < vInfo.size(); i++)
    {
        QCefWidget *qdlg = new QCefWidget();         
        QDlgShow(qdlg, *vInfo[i]);        
        m_mDlgs.insert(make_pair(vInfo[i]->dlgId, qdlg));
    }
    

}

void MainWindow::onMouseEventRequested(int type,QVariant dlgInfo)
{
    QtDlgInfo dlg = dlgInfo.value<QtDlgInfo>();
    
    // QDlgShow(qdlg);
    // std::cout << "type = " << type << std::endl;
    if(type == 1)
    {
        showNewDlg(dlg.dlgId);
    }
    else if(type == 2)
    {
        std::map<std::string, QCefWidget *>::iterator it = m_mDlgs.find(dlg.dlgId);
        if(it != m_mDlgs.end())
        {
            // it->second->CloseDlg(dlg.dlgId);
            it->second->close();
            delete it->second;
            m_mDlgs.erase(it);
        }
    }
    else if(type == 3)
    {
        showNewDlg(dlg.dlgId);//modifyDlg(dlg);
        
    }
    else
    {
        ;
    }

}

void MainWindow::onOpenTitleWindow()
{
    onCloseTitleWindow();

    int multiple = 1;
    cmyxrandr* pcmxrandr =  cmyxrandr::GetInstance();
    CMYSIZE currentSize, maxSize;
    vector<MOutputInfo> vOutputInfo;  
    short shRet = pcmxrandr->getAllScreenInfoXrandr(vOutputInfo,currentSize,maxSize);

    for(auto &v : vOutputInfo)
    {
        if(v.nIndex < 0)
            continue;
        std::string name = std::to_string(v.nIndex);
        name += ",";
        name += v.name;
        MyMainWindow *w = new MyMainWindow(name);
        w->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowStaysOnTopHint);
        w->createWindow(v.pos.xPos,v.pos.yPos,v.size.width/multiple,v.size.height/multiple);
        m_titleWindows[v.name]=w;
    }
    m_isOpen = true;
    // 创建一个一次性定时器，设置间隔为5分钟
    QTimer::singleShot(300000, this, &MainWindow::onCloseTitleWindow);
}

void MainWindow::onCloseTitleWindow()
{
    m_isOpen = false;
    auto itr = m_titleWindows.begin();
    for(;itr!=m_titleWindows.end();itr++)
    {
        auto w = itr->second;
        w->close();
        delete w;
    }
    m_titleWindows.clear();
}

void MainWindow::Init()
{
    emit initSignal();
}

bool MainWindow::showNewDlg(string dlgId)
{
    std::map<std::string,QtDlgInfo *> ::iterator itp = m_mDlgProperty.find(dlgId);
    if(itp != m_mDlgProperty.end())
    {
        std::map<std::string, QCefWidget *>::iterator it = m_mDlgs.find(dlgId);
        if (it != m_mDlgs.end())
        {            
            it->second->updateWindow(itp->second);       
            return true;
        }
        else
        {
            QCefWidget *qdlg = new QCefWidget();

            qdlg->setWindowFlags(windowFlags() & ~Qt::WindowFullScreen | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::SubWindow);
            qdlg->UpdateSetting(itp->second);
            // QDlgShow(qdlg, *itp->second);
            m_mDlgs.insert(make_pair(dlgId, qdlg));
        }
    }
    return false;
   
}

bool MainWindow::addDlg(QtDlgInfo & dlg)
{
    if(dlg.dlgId.empty())
    {
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        string uuid_string = boost::uuids::to_string(a_uuid);
        dlg.dlgId = uuid_string;        
        QtDlgInfo * info = new QtDlgInfo;
        if(dlg.order == 0)
            dlg.order = m_mDlgProperty.size() + 1;
        *info = dlg;
        info->createTime = GetCurrentDateTime();
        m_mDlgProperty.insert(make_pair(dlg.dlgId,info));
        string strJson;
        dlgInfoToJson(&dlg,strJson);
        writeSettings(dlg.dlgId,strJson);
    }
    else
    {
        if(m_mDlgProperty.find(dlg.dlgId) != m_mDlgProperty.end())
        {
            return false;
        }
    }     

    return true;
}

bool MainWindow::modifyDlg(QtDlgInfo & info)
{
    if(!info.dlgId.empty())
    {
        std::map<std::string,QtDlgInfo *> ::iterator itp = m_mDlgProperty.find(info.dlgId);
        if(itp != m_mDlgProperty.end())
        {
            std::string time = itp->second->createTime;
            info.createTime = time;
            *itp->second = info;
            info.order = itp->second->order;
            string strJson;
            dlgInfoToJson(&info, strJson);
            writeSettings(info.dlgId, strJson);
        }
        else
        {
            return false;
        }


        // std::map<std::string, QCefWidget *>::iterator it = m_mDlgs.find(info.dlgId);
        // if(it != m_mDlgs.end())
        // {
        //     it->second->UpdateSetting(&info);
        // }
        // else
        // {
        //     QCefWidget *qdlg = new QCefWidget(info.dlgId);            
        //     QDlgShow(qdlg, info);
        //     m_mDlgs.insert(make_pair(info.dlgId, qdlg));
        // }
    }

    return true;
}

bool MainWindow::delDlg(QtDlgInfo & info)
{
    if(!info.dlgId.empty())
    {
        std::map<std::string,QtDlgInfo *> ::iterator itp = m_mDlgProperty.find(info.dlgId);
        if(itp == m_mDlgProperty.end())
            return false;
        else
        {
            deleteSettings(info.dlgId);
            delete itp->second;
            m_mDlgProperty.erase(info.dlgId);
        }
        return true;
    }
    else
    {
        return false;
    }

}

bool MainWindow::getDlgInfo(string dlgId,QtDlgInfo & info)
{
    if(!dlgId.empty())
    {
        std::map<std::string,QtDlgInfo *> ::iterator itp = m_mDlgProperty.find(dlgId);
        if(itp != m_mDlgProperty.end())
        {
            info = *itp->second;
            return true;
         
        }
        else
        {
            return false;
        }
    }
    
    return false;
}
bool MainWindow::getAllDlgInfo(std::vector<QtDlgInfo> & vInfo)
{
    for (std::map<std::string,QtDlgInfo *> ::iterator it = m_mDlgProperty.begin(); it != m_mDlgProperty.end(); it++)
    {
        QtDlgInfo info = *it->second;
        vInfo.push_back(info);
    } 

    return true;
}

bool MainWindow::checkOrder(int order, std::string id)
{
    for (std::map<std::string,QtDlgInfo *> ::iterator it = m_mDlgProperty.begin(); it != m_mDlgProperty.end(); it++)
    {
        if(it->second->order == order)
        {
            if(!id.empty() && it->second->dlgId == id)
                return true;
            return false;
        }
    }
    return true;
}