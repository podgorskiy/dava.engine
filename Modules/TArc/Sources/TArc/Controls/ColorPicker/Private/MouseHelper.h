#pragma once

#include <QObject>
#include <QWidget>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class MouseHelper : public QObject
{
    Q_OBJECT

signals:
    void mousePress(const QPoint& pos);
    void mouseMove(const QPoint& pos);
    void mouseRelease(const QPoint& pos);
    void clicked();
    void mouseEntered();
    void mouseLeaved();

    void mouseWheel(int delta);

public:
    explicit MouseHelper(QWidget* w);

    bool IsPressed() const;

private:
    bool eventFilter(QObject* obj, QEvent* e);

    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseWheelEvent(QWheelEvent* event);

    QPointer<QWidget> w;
    QPoint pos;
    QPoint clickPos;
    bool isHover;
    bool isPressed;
    int clickDist;
    int dblClickDist;
};
}
}
