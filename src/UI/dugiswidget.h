#ifndef DUGISWIDGET_H
#define DUGISWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebChannel>
#include <QDebug>
#include <QFile>

namespace Ui {
class DuGisWidget;
}

class DuGisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DuGisWidget(QWidget *parent = nullptr);
    ~DuGisWidget();

    void initWidget();

private:
    Ui::DuGisWidget *ui;
};

#endif // DUGISWIDGET_H
