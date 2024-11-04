#ifndef DLGURL_H
#define DLGURL_H

#include <QDialog>
#include <QPushButton>
//#include "../common.h"

#include <QWebEngineView>
#include <QVBoxLayout>
#include "../qtcommon.h"

#include <cstring>

using namespace std;


class dlgManager;

namespace Ui {
class DlgUrl;
}

class DlgUrl : public QDialog
{
    Q_OBJECT

public:
    explicit DlgUrl(string strDlgId,QWidget *parent = nullptr);
    ~DlgUrl();
    // bool SetConnect(dlgManager *dlg);//,int type,QPoint p,QSize size
    // bool DelConnect(dlgManager *dlg);
    void UpdateSetting(QtDlgInfo * dlg);
    void CloseDlg(string strDlgId);


public slots:
    void DataprocessSlots(string strDlgId,int cmdType = 0);
    void updateslots();

signals:
    void updateSignal();
    void closeDlgSignal(string strDlgId,int cmdType);

// signals:
//     void mouseSignal(int type,QPoint p,QSize size);

private:
    Ui::DlgUrl *ui;
    string m_dlgId;
    QWebEngineView * m_pWebView;
    QVBoxLayout * m_pLayoutv;
    QString m_qsUrl;
    QPoint m_pos;
    QSize m_size;
    QtDlgInfo m_QtDlgInfo;
};

#endif // DLGURL_H
