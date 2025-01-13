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
    if (m_pRightCefViewWidget)
    {
        delete m_pRightCefViewWidget;
        m_pRightCefViewWidget =nullptr;
    }
    if (m_layout)
    {
        delete m_layout;
        m_layout = nullptr;
    }
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
    // ui->webEngineView->load(QUrl(m_QtDlgInfo.url.c_str()));
    // m_pos = QPoint(m_QtDlgInfo.xPos,m_QtDlgInfo.yPos);
    // m_size = QSize(m_QtDlgInfo.width,m_QtDlgInfo.height);
    // emit updateSignal();
    createRightCefView();
    setGeometry(dlg->xPos, dlg->yPos, dlg->width, dlg->height);
    show();
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

void DlgUrl::createRightCefView()
{
    if (m_pRightCefViewWidget)
    {
        if (m_layout)
        {
            delete m_layout;
            m_layout = nullptr;
        }
        m_pRightCefViewWidget->deleteLater();
        m_pRightCefViewWidget = nullptr;
    }

    ///*
    // build settings for per QCefView
    QCefSetting setting;

#if CEF_VERSION_MAJOR < 100
    setting.setPlugins(false);
#endif

    setting.setWindowlessFrameRate(60);
    // setting.setBackgroundColor(QColor::fromRgba(qRgba(255, 255, 220, 255)));
    // setting.setBackgroundColor(QColor::fromRgb(0, 0, 255));
    // setting.setBackgroundColor(Qt::lightGray);
    // create the QCefView widget and add it to the layout container
    m_pRightCefViewWidget = new CefViewWidget(m_QtDlgInfo.url.c_str(), &setting, this);
    m_pRightCefViewWidget->resize(500, 500);    
    m_pRightCefViewWidget->setContextMenuPolicy(Qt::DefaultContextMenu);
    //m_ui.Container->layout()->addWidget(m_pRightCefViewWidget);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_pRightCefViewWidget);
    
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