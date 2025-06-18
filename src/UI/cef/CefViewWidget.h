#ifndef CUSTOMCEFVIEW_H
#define CUSTOMCEFVIEW_H

#include <QScreen>
#include <QCefView.h>
#include <QPoint>
/// <summary>
/// Represents the customized QCefView
/// </summary>
class CefViewWidget : public QCefView
{
  Q_OBJECT

public:
  CefViewWidget(const QString url, const QCefSetting* setting, QWidget* parent = 0);

  ~CefViewWidget();

  void setRounded(int value);

protected slots:
  void onScreenChanged(QScreen* screen);

  void onNativeBrowserWindowCreated(QWindow* window);

  void onDraggableRegionChanged(const QRegion& draggableRegion, const QRegion& nonDraggableRegion);

protected:
  bool onNewPopup(const QCefFrameId& sourceFrameId,
                  const QString& targetUrl,
                  QString& targetFrameName,
                  QCefView::CefWindowOpenDisposition targetDisposition,
                  QRect& rect,
                  QCefSetting& settings,
                  bool& disableJavascriptAccess) override;

  void onNewDownloadItem(const QSharedPointer<QCefDownloadItem>& item, const QString& suggestedName) override;

  void onUpdateDownloadItem(const QSharedPointer<QCefDownloadItem>& item) override;

protected:
  void resizeEvent(QResizeEvent* event) override;
  // 鼠标事件
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  // 键盘事件
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  // 焦点
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;

  void inputMethodEvent(QInputMethodEvent* event) override;
  // bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void updateMask();

private:
  QWindow* m_pCefWindow = nullptr;

  int m_iCornerRadius = 50;

  QRegion m_draggableRegion;

  QRegion m_nonDraggableRegion;

  int m_rounded = 0;
};

#endif // CUSTOMCEFVIEW_H
