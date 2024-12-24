#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>
#include <QHBoxLayout>
#include "../3rd/QCefView/example/QCefViewTest/CefViewWidget.h"
#include "ui_mymainwindow.h"


namespace Ui {
class MyMainWindow;
}

class MyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MyMainWindow(std::string name, QWidget *parent = nullptr);
    ~MyMainWindow();
    void createWindow(int x,int y,int w, int h);

protected:
  void createRightCefView();
  void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MyMainWindow m_ui;
    QVBoxLayout * m_layout;
    CefViewWidget* m_pRightCefViewWidget = nullptr;
    std::string m_name;
};

#endif // MYMAINWINDOW_H
