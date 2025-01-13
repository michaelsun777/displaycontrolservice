#include "qcefwidget.h"

QCefWidget::QCefWidget(QWidget *parent) :
    QMainWindow(parent)
    //,    ui(new Ui::QCefWidget)
{
    m_ui.setupUi(this);
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);
    // 去掉边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

}

QCefWidget::~QCefWidget()
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
    //delete ui;
}


void QCefWidget::createRightCefView()
{
    if (m_pRightCefViewWidget)
    {
        if(m_layout)
        {
            m_layout->removeWidget(m_pRightCefViewWidget);
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

void QCefWidget::UpdateSetting(QtDlgInfo * dlg)
{
    m_QtDlgInfo = *dlg;
    createRightCefView();
    setGeometry(dlg->xPos, dlg->yPos, dlg->width, dlg->height);
    show();
}

void QCefWidget::resizeEvent(QResizeEvent *event)
{
    QSize newSize = event->size();

    m_pRightCefViewWidget->resize(newSize);

    QWidget::resizeEvent(event); 
}

void QCefWidget::createWindow(int x, int y, int w, int h)
{
    createRightCefView();
    setGeometry(x, y, w, h);
    show();
}