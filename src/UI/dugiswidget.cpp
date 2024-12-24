#include "dugiswidget.h"
#include "ui_dugiswidget.h"

DuGisWidget::DuGisWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DuGisWidget)
{
    ui->setupUi(this);
    initWidget();

    



}

void DuGisWidget::initWidget()
{
    //QString htmlPath = QCoreApplication::applicationDirPath() + "/html/";
    QString htmlPath = "./html/";
    QString htmlFile = htmlPath + "index.html";
    qDebug() << htmlFile; /* 获取你要显示网页的路径 */
    QFile file(htmlFile);
    if(!file.exists())
        qDebug() << "html file is not exist";
    /* 创建一个与网页交互的通道 */
    QWebChannel *webChannel = new QWebChannel(ui->wg_map->page());
    ui->wg_map->page()->setWebChannel(webChannel);
    /* 注册通道，ID 为 JSInterface，其将在JS文件这引用 */
    webChannel->registerObject(QString("JSInterface"), ui->wg_map);
    /* 加载网页，注意加载网页必须在通道注册之后，其有有一个注册完成的信号，
       可根据需要调用 */
    ui->wg_map->page()->load(QUrl("file:///" + htmlFile));

}

DuGisWidget::~DuGisWidget()
{
    delete ui;
}
