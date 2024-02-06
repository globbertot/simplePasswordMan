#ifndef CUSTOMTITLEBAR_H
#define CUSTOMTITLEBAR_H

#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPoint>

class CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    CustomTitleBar(QWidget* parent);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
private:
    QWidget* _parent;
    QPoint startPos;
    QPoint windowPos;
};

#endif // CUSTOMTITLEBAR_H
