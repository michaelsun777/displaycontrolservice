#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "UI/dlgurl.h"
#include "common.h"
#include <iostream>
#include <QSettings>
#include "cspdlog.h"
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread.hpp>
#include <errno.h>
#include "../3rd/json/include/nlohmann/json.hpp"
#include <map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <cstring>
#include "../qtcommon.h"

using namespace std;


namespace Ui {
class MainWindow;
}

class cdataProcess;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool QDlgShow(DlgUrl * qdlg,QtDlgInfo & info);
    void test();

public:
    void Init();    
    bool addDlg(QtDlgInfo & info);
    bool delDlg(QtDlgInfo & info);
    bool modifyDlg(QtDlgInfo & info);
    bool getDlgInfo(string dlgId,QtDlgInfo & info);
    bool getAllDlgInfo(std::vector<QtDlgInfo> & vInfo);
private:
    bool parseJsonToDlgInfo(QtDlgInfo * info,string str);
    bool dlgInfoToJson(QtDlgInfo * info,string & str);
    bool readSettings();
    bool writeSettings(string key,string value);
    bool deleteSettings(string key);
    

private slots:
    void on_pbtn_test_clicked();

public slots:
    void onInitSlots();
    bool showNewDlg(string dlgId);
    void onMouseEventRequested(int type,QVariant dlgInfo);
    

public:

signals:
    void initSignal();

private:
    //cdataProcess * m_process;
    std::map<std::string,DlgUrl *> m_mDlgs;
    std::map<std::string,QtDlgInfo *> m_mDlgProperty;
    

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
