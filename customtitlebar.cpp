#include "customtitlebar.h"

CustomTitleBar::CustomTitleBar(QWidget* parent) : QWidget() {
    if (parent) {
        QLabel* title = new QLabel(parent->windowTitle());
        QPushButton* exitBtn = new QPushButton("X");

        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(title);
        layout->addWidget(exitBtn);

        connect(exitBtn, &QPushButton::clicked, parent, &QWidget::close);
    }
}

void CustomTitleBar::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        startPos = e->globalPosition();
        windowPos = parentWidget()->pos();
    }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* e) {
    if (e->button() & Qt::LeftButton) {
        parentWidget()->move(windowPos + e->globalPosion() - startPos);
    }
}
