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
    void UpdateSetting(QtDlgInfo & dlg);
//protected slots:
//    void onMouseEventRequested(string strDlgId,QPoint p,QSize size);


public slots:
    void DataprocessSlots(string strDlgId);
    void updateslots();

signals:
    void updateSignal();

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
