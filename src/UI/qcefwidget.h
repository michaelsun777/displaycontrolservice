#ifndef QCEFWIDGET_H
#define QCEFWIDGET_H

#include <QMainWindow>
#include <QResizeEvent>
#include <QHBoxLayout>
// #include "../3rd/QCefView/example/QCefViewTest/CefViewWidget.h"
#include "cef/CefViewWidget.h"
#include "ui_qcefwidget.h"
#include "../qtcommon.h"


namespace Ui {
class QCefWidget;
}

class QCefWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit QCefWidget(QWidget *parent = nullptr);
    ~QCefWidget();
    void createWindow(int x,int y,int w, int h);
    void UpdateSetting(QtDlgInfo * dlg);

protected:
  void createRightCefView();
  void resizeEvent(QResizeEvent *event) override;

private:
    Ui::QCefWidget m_ui;
    QVBoxLayout * m_layout;
    QtDlgInfo m_QtDlgInfo;
    CefViewWidget* m_pRightCefViewWidget = nullptr;
};

#endif // MYMAINWINDOW_H
