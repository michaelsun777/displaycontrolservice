#include "CefViewWidget.h"

#include <QColor>
#include <QDebug>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QWindow>
#include <QMouseEvent>
#include "DownloadManager.h"

CefViewWidget::CefViewWidget(const QString url, const QCefSetting *setting, QWidget *parent /* = 0*/)
    : QCefView(url, setting, parent)
{
  setStyleSheet("background: blue;");
  setFocusPolicy(Qt::StrongFocus);         // 设置焦点策略为强聚焦，以便接收键盘事件
  // installEventFilter(this);                // 启用事件过滤器
  setAttribute(Qt::WA_InputMethodEnabled); // 启用输入法支持
  // setMouseTracking(true);
  connect(this, &CefViewWidget::draggableRegionChanged, this, &CefViewWidget::onDraggableRegionChanged);
  connect(this, &CefViewWidget::nativeBrowserCreated, this, &CefViewWidget::onNativeBrowserWindowCreated);
}

CefViewWidget::~CefViewWidget() {}

void CefViewWidget::onScreenChanged(QScreen *screen)
{
  if (!m_pCefWindow)
    return;

  updateMask();
}

void CefViewWidget::onNativeBrowserWindowCreated(QWindow *window)
{
  m_pCefWindow = window;
  if (!m_pCefWindow)
    return;

  connect(this->window()->windowHandle(), SIGNAL(screenChanged(QScreen *)), this, SLOT(onScreenChanged(QScreen *)));

  updateMask();
}

void CefViewWidget::onDraggableRegionChanged(const QRegion &draggableRegion, const QRegion &nonDraggableRegion)
{
  m_draggableRegion = draggableRegion;
  m_nonDraggableRegion = nonDraggableRegion;
}

bool CefViewWidget::onNewPopup(const QCefFrameId &sourceFrameId,
                               const QString &targetUrl,
                               QString &targetFrameName,
                               QCefView::CefWindowOpenDisposition targetDisposition,
                               QRect &rect,
                               QCefSetting &settings,
                               bool &disableJavascriptAccess)
{
  // create new QCefView as popup browser
  settings.setBackgroundColor(Qt::red);
  return false;
}

void CefViewWidget::onNewDownloadItem(const QSharedPointer<QCefDownloadItem> &item, const QString &suggestedName)
{
  // keep the item into list or map, and call item->start() to allow the download

  DownloadManager::getInstance().AddNewDownloadItem(item);
}

void CefViewWidget::onUpdateDownloadItem(const QSharedPointer<QCefDownloadItem> &item)
{

  // control the download by invoking item->pause(), item->resume(), item->cancel()

  DownloadManager::getInstance().UpdateDownloadItem(item);
}

void CefViewWidget::resizeEvent(QResizeEvent *event)
{
  // update mask first, because the new mask will be
  // used in the QCefView::resizeEvent
  updateMask();

  QCefView::resizeEvent(event);
}

void CefViewWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    activateWindow(); // 激活当前窗口
    raise();          // 置顶显示
    // QCefView::setFocus();
    qDebug() << "Left click at:" << event->pos();
  }
  QCefView::mousePressEvent(event);
}

void CefViewWidget::mouseMoveEvent(QMouseEvent *event)
{
  QCefView::mouseMoveEvent(event);
}

void CefViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    qDebug() << "Left release at:" << event->pos();
  }
  QCefView::mouseReleaseEvent(event);
}

void CefViewWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  qDebug() << "mouseDoubleClickEvent:" << event->pos();
  QCefView::mouseDoubleClickEvent(event);
}

void CefViewWidget::wheelEvent(QWheelEvent *event)
{
  qDebug() << "wheelEvent:" << event->pos();
  QCefView::wheelEvent(event);
}

void CefViewWidget::keyPressEvent(QKeyEvent *event)
{
  qDebug() << "keyPressEvent at:" << event->key();
  QCefView::keyPressEvent(event);
}

void CefViewWidget::keyReleaseEvent(QKeyEvent *event)
{
  qDebug() << "keyReleaseEvent at:" << event->key();
  QCefView::keyReleaseEvent(event);
}

void CefViewWidget::focusInEvent(QFocusEvent *event)
{
  qDebug() << "focusInEvent";
  QCefView::focusInEvent(event);
  grabKeyboard(); // 获取键盘独占权
}

void CefViewWidget::focusOutEvent(QFocusEvent *event)
{
  qDebug() << "focusOutEvent";
  releaseKeyboard(); // 释放键盘独占权
  QCefView::focusOutEvent(event);
}

void CefViewWidget::inputMethodEvent(QInputMethodEvent *event)
{
  qDebug() << "inputMethodEvent";
  QCefView::inputMethodEvent(event);
}

// bool CefViewWidget::eventFilter(QObject *watched, QEvent *event)
// {
//   if (event->type() == QEvent::Paint)
//   {
//     return false;
//   }
//   if (this->isActiveWindow())
//   {
//     qDebug() << "eventFilter: ActiveWindow";
//   }
//   if (this->hasFocus())
//   {
//     qDebug() << "eventFilter: HasFocus";
//   }
//   qDebug() << "eventFilter: " << event->type();
//   if (event->type() == QEvent::FocusIn)
//   {
//     qDebug() << "eventFilter: FocusIn";
//   }
//   if (event->type() == QEvent::KeyPress)
//   {
//     QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//     qDebug() << "eventFilter:" << keyEvent->text();
//   }
//   return false;
// }

void CefViewWidget::updateMask()
{
  // create a rect with rounded corner (50px radius) as mask
  QPainterPath path;
  path.addRoundedRect(rect(), m_rounded, m_rounded);
  QRegion mask = QRegion(path.toFillPolygon().toPolygon());

  // apply the mask
  setMask(mask);
}

void CefViewWidget::setRounded(int value)
{
  m_rounded = value;
}
