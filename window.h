#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include "./ui_window.h"
#include "./dbmanager.h"
#include <QPushButton>
#include <QDebug>
#include <QList>
#include <QScrollArea>
#include <QLineEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLayoutItem>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSettings>
#include <QCheckBox>
#include <QClipboard>
#include <QMouseEvent>
#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui {
class window;
}
QT_END_NAMESPACE

class window : public QMainWindow
{
    Q_OBJECT

public:
    window(QWidget *parent = nullptr);
    ~window();

    // Core variables
    bool showPasswords = false;

    // core logic
    void create();
    void _delete();
    void search();
    void updatePass();
    void settings();
    void showPassPolicy();
    QSettings setts;

    void handleActionComplete(bool success, std::string action, std::string errMsg = "", QList<std::string> result = {});

    // helper functions
    QList<QPushButton*> grabButtons();
    void enableDisableCommands(int mode);
    void clearScreen();

    // Top bar
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
private:
    Ui::MainWindow *ui;
    dbManager* db;

    bool isMousePressed;
    QPoint dragStart;
};
#endif // WINDOW_H
