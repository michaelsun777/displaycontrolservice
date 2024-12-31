#include "mymainwindow.h"

MyMainWindow::MyMainWindow(std::string name, QWidget *parent) :
    QMainWindow(parent),m_name(name)
    //,    ui(new Ui::MyMainWindow)
{
    m_ui.setupUi(this);
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);
    // 去掉边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

}

MyMainWindow::~MyMainWindow()
{
    //delete ui;
}


void MyMainWindow::createRightCefView()
{
    if (m_pRightCefViewWidget)
    {
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
    setting.setBackgroundColor(Qt::lightGray);
    std::string url = "CefView://";
    url += m_name;
    // create the QCefView widget and add it to the layout container
    m_pRightCefViewWidget = new CefViewWidget(url.c_str(), &setting, this);
    m_pRightCefViewWidget->resize(500, 500);    
    m_pRightCefViewWidget->setContextMenuPolicy(Qt::DefaultContextMenu);
    //m_ui.Container->layout()->addWidget(m_pRightCefViewWidget);
    //m_pRightCefViewWidget->setRounded(20);
    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_pRightCefViewWidget);

    //QString uri ="https://map.baidu.com/";
    //QCefView * cefViewWidget = new QCefView(uri, &setting, this);
    // QVBoxLayout *layout = new QVBoxLayout(this);
    // cefViewWidget->resize(500, 500);
    // layout->addWidget(cefViewWidget);
    // cefViewWidget->setContextMenuPolicy(Qt::DefaultContextMenu);
   

    // 

    // allow show context menu for both OSR and NCW mode
    
}



void MyMainWindow::resizeEvent(QResizeEvent *event)
{
    QSize newSize = event->size();

    m_pRightCefViewWidget->resize(newSize);

    QWidget::resizeEvent(event); 
}

void MyMainWindow::createWindow(int x, int y, int w, int h)
{
    createRightCefView();
    setGeometry(x, y, w, h);
    show();
}