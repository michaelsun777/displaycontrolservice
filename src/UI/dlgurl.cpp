#include "dlgurl.h"
#include "ui_dlgurl.h"
#include <QMouseEvent>
#include <iostream>

DlgUrl::DlgUrl(string strDlgId,QWidget *parent) : m_qsUrl(""),
    QDialog(parent),
    ui(new Ui::DlgUrl)
{
    ui->setupUi(this);
    //qRegisterMetaType<DlgUrl::mouseSignal>("DlgUrl::mouseSignal");
    //setAttribute(Qt::WA_DeleteOnClose);
    m_dlgId = strDlgId;
    connect(this,&DlgUrl::updateSignal,this,&DlgUrl::updateslots);
    connect(this,&DlgUrl::closeDlgSignal,this,&DlgUrl::DataprocessSlots);
    //dlgManager * dlg = (dlgManager*)parent;
    //connect(dlg,&dlgManager::mouseSignal,this,&DlgUrl::DataprocessSignal);
    
}

DlgUrl::~DlgUrl()
{
 
    delete ui;
}

// bool DlgUrl::SetMouseSignal(dlgManager *dlg,int type,QPoint p,QSize size)
// {
//     m_pos = p;
//     m_size = size;
//     connect(dlg,&dlgManager::mouseSignal,this,&DlgUrl::testSignal);
// }
// bool DlgUrl::SetConnect(dlgManager *dlg)
// {
//     connect(dlg,&dlgManager::mouseSignal,this,&DlgUrl::testSignal);
// }


// bool DlgUrl::DelConnect(dlgManager *dlg)
// {
//     disconnect(dlg,&dlgManager::mouseSignal,this,&DlgUrl::testSignal);
// }

void DlgUrl::UpdateSetting(QtDlgInfo * dlg)
{
    m_QtDlgInfo = *dlg;
    // m_qsUrl = "https://www.baidu.com";    
    ui->webEngineView->load(QUrl(m_QtDlgInfo.url.c_str()));
    m_pos = QPoint(m_QtDlgInfo.xPos,m_QtDlgInfo.yPos);
    m_size = QSize(m_QtDlgInfo.width,m_QtDlgInfo.height);
    emit updateSignal();
}

void DlgUrl::updateslots()
{
    move(m_pos);
    resize(m_size);
    //ui->vl_VerticalLayout->resize(m_size);
    ui->webEngineView->move(0,0);
    ui->webEngineView->resize(m_size);
    update();
}

void DlgUrl::CloseDlg(string strDlgId)
{
    emit closeDlgSignal(strDlgId,0);
}

void DlgUrl::DataprocessSlots(string strDlgId,int cmdType)
{
    if(m_dlgId.compare(strDlgId) == 0)
    {
        std::cout<<"testSignal,id=" << strDlgId<<std::endl;
        if(cmdType == 0)
        {
            close();
        }
    }
}


// void DlgUrl::onMouseEventRequested(int type,QPoint p,QSize size)
// {
//     if(type ==1)
//     {
//         //QPoint p = m_pbtnResize->rect().center();
//         //QMouseEvent mevent(QEvent::MouseButtonPress,p,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
//         //QApplication::sendEvent(m_pbtnResize,&mevent);
//     }
//     else if(type == 2)
//     {
//         //QPoint p = m_pbtnResize->rect().center();
//         //QMouseEvent mevent(QEvent::MouseButtonRelease,p,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
//         //QApplication::sendEvent(m_pbtnResize,&mevent);
//     }

// } 