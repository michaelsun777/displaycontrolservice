#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../cdataProcess.h"

#include "../common.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //m_process = p;
    //connect(this,&MainWindow::testSignal,this,&MainWindow::onMouseEventRequested);
    //connect(p,&cdataProcess::testSignal,this,&MainWindow::onMouseEventRequested);

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::QDlgShow(DlgUrl * qdlg,QtDlgInfo & info)
{
    qdlg->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    qdlg->show();    
    qdlg->UpdateSetting(info);

}

void MainWindow::test()
{    
    //emit testSignal();
}
void MainWindow::on_pbtn_test_clicked()
{
    //emit testSignal();
    // DlgUrl * qdlg = new DlgUrl();
    // QDlgShow(qdlg);
}

void MainWindow::onMouseEventRequested(int type,QVariant dlgInfo)
{
    QtDlgInfo dlg = dlgInfo.value<QtDlgInfo>();
    
    // QDlgShow(qdlg);
    // std::cout << "type = " << type << std::endl;
    if(type == 1)
    {
        addDlg(dlg);
    }
}

bool MainWindow::addDlg(QtDlgInfo & dlg)
{
    if(dlg.dlgId.empty())
    {
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        string uuid_string = boost::uuids::to_string(a_uuid);
        dlg.dlgId = uuid_string;
        DlgUrl *qdlg = new DlgUrl(dlg.dlgId);
        QDlgShow(qdlg, dlg);
        m_mDlgs.insert(make_pair(dlg.dlgId, qdlg));
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