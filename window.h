#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include "./ui_window.h"
#include "./dbmanager.h"
#include "themepreview.h"
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
#include <QDesktopServices>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QVariantMap>
#include <QColorDialog>
#include <QRegularExpression>

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

    const int ENABLE_COMMANDS = 1;
    const int DISABLE_COMMANDS = 0;

    // default themes presets
    QString darkThemeStyleSheet =
        "QWidget#app { background: #2D2D2D; }"
        "QPushButton { background: #323232; color: #FFF; border: none; padding: 5px 5px; border-radius: 8px; }"
        "QPushButton:hover { background: #1E1E1E; }"
        "QPushButton:disabled { background: #505050; color: #777777; }"
        "QLineEdit { background: #404040; color: #FFF; border: 1px solid #666666; border-radius: 4px; padding: 8px; }"
        "QLineEdit::placeholder { color: #999; }"
        "QCheckBox { color: #FFF; font-size: 14px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; background: #747474; }"
        "QCheckBox::indicator:checked { background: #1e6145; }"
        "QLabel { color: #FFF; }"
        "QScrollBar:vertical { background: #2D2D2D; width: 10px; }"
        "QScrollBar:handle:vertical { background: #666666; border-radius: 5px; }"
        "QScrollBar:horizontal { background: #2D2D2D; width: 10px; }"
        "QScrollBar:handle:horizontal { background: #666666; border-radius: 5px; }"
        "QWidget#scrollThing { background: #404040; min-width: 630px; min-height: 580px; font-size: 17px; }"
        "QMessageBox { background: #404040; color: #FFF; } "
        "QRadioButton { color: #FFF; font-size: 14px; } "
        "QRadioButton:indicator { width: 16px; height: 16px; background: #747474; }"
        "QRadioButton:indicator:checked { background: #add8e6; }";

    QString lightThemeStyleSheet =
        "QWidget#app { background: #F0F0F0; }"
        "QPushButton { background: #E0E0E0; color: #000; border: none; padding: 5px 5px; border-radius: 8px; }"
        "QPushButton:hover { background: #CCCCCC; }"
        "QPushButton:disabled { background: #CCCCCC; color: #777777; }"
        "QLineEdit { background: #FFFFFF; color: #000; border: 1px solid #CCCCCC; border-radius: 4px; padding: 3px; }"
        "QLineEdit::placeholder { color: #666; }"
        "QCheckBox { color: #000; font-size: 14px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; background: #F0F0F0; }"
        "QCheckBox::indicator:checked { background: #1e6145; }"
        "QLabel { color: #000; }"
        "QScrollBar:vertical { background: #F0F0F0; width: 10px; }"
        "QScrollBar:handle:vertical { background: #CCCCCC; border-radius: 5px; }"
        "QScrollBar:horizontal { background: #F0F0F0; width: 10px; }"
        "QScrollBar:handle:horizontal { background: #CCCCCC; border-radius: 5px; }"
        "QWidget#scrollThing { background: #FFFFFF; min-width: 630px; min-height: 580px; font-size: 17px; }"
        "QMessageBox { background: #FFFFFF; color: #000; }"
        "QRadioButton { color: #000; font-size: 14px; } "
        "QRadioButton:indicator { width: 16px; height: 16px; background: #747474; }"
        "QRadioButton:indicator:checked { background: #add8e6; }";

    QString darkBlueThemeStyleSheet =
        "QWidget#app { background: #1E213A;  }"
        "QPushButton { background: #161A2B; color: #FFF; border: none; padding: 5px 5px; border-radius: 8px; }"
        "QPushButton:hover { background: #0E0F17; }"
        "QPushButton:disabled { background: #33364F; color: #777777; }"
        "QLineEdit { background: #2E3254; color: #FFF; border: 1px solid #3E426C; border-radius: 4px; padding: 8px; }"
        "QLineEdit::placeholder { color: #999; }"
        "QCheckBox { color: #FFF; font-size: 14px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; background: #464B7A; }"
        "QCheckBox::indicator:checked { background: #1e6145; }"
        "QLabel { color: #FFF; }"
        "QScrollBar:vertical { background: #2E213A; width: 10px; }"
        "QScrollBar:handle:vertical { background: #666666; border-radius: 5px; }"
        "QScrollBar:horizontal { background: #2E213A; width: 10px; }"
        "QScrollBar:handle:horizontal { background: #666666; border-radius: 5px; }"
        "QWidget#scrollThing { background: #2E3254; min-width: 630px; min-height: 580px; font-size: 17px; }"
        "QMessageBox { background: #2E3254; color: #FFF; }"
        "QRadioButton { color: #FFF; font-size: 14px; }"
        "QRadioButton:indicator { width: 16px; height: 16px; background: #464B7A; }"
        "QRadioButton:indicator:checked { background: #add8e6; }";

    QString lightBlueThemeStyleSheet =
        "QWidget#app { background: #EAF6FF; }"
        "QPushButton { background: #D3EAF7; color: #000; border: none; padding: 5px 5px; border-radius: 8px; }"
        "QPushButton:hover { background: #B8DDE9; }"
        "QPushButton:disabled { background: #CCCCCC; color: #777777; }"
        "QLineEdit { background: #FFFFFF; color: #000; border: 1px solid #CCCCCC; border-radius: 4px; padding: 3px; }"
        "QLineEdit::placeholder { color: #666; }"
        "QCheckBox { color: #000; font-size: 14px; }"
        "QCheckBox:indicator { width: 16px; height: 16px; background: #EAF6FF; }"
        "QCheckBox:indicator:checked { background: #1e6145; }"
        "QLabel { color: #000; }"
        "QScrollBar:vertical { background: #EAF6FF; width: 10px; }"
        "QScrollBar:handle:vertical { background: #CCCCCC; border-radius: 5px; }"
        "QScrollBar:horizontal { background: #EAF6FF; width: 10px; }"
        "QScrollBar:handle:horizontal { background: #CCCCCC; border-radius: 5px; }"
        "QWidget#scrollThing { background: #FFFFFF; min-width: 630px; min-height: 580px; font-size: 17px; }"
        "QMessageBox { background: #EAF6FF; color: #000; }"
        "QRadioButton { color: #000; font-size: 14px; }"
        "QRadioButton:indicator { width: 16px; height: 16px; background: #ffeafc; }"
        "QRadioButton:indicator:checked { background: #add8e6; }";

    // Core variables
    bool bShowPasswords = false;
    bool bAlwaysGeneratePass = true;
    const std::string updatesURL = "https://github.com/globbertot/simplePasswordMan/releases/latest";
    const std::string VERSION = "V0.3";
    std::string currentTheme = "DARK";
    std::map<std::string, std::string> themes = {
        {"DARK", darkThemeStyleSheet.toStdString()},
        {"LIGHT", lightThemeStyleSheet.toStdString()},
        {"DARKBLUE", darkBlueThemeStyleSheet.toStdString()},
        {"LIGHTBLUE", lightBlueThemeStyleSheet.toStdString()},
    };
    // 1 = the widget 2 = text to be rendered
    const std::map<std::string, std::string> editableWidgets = {
        {"QWidget#app", "Main Application Window"},
        {"QPushButton", "Buttons"},
        {"QPushButton:hover", "Button Hover State"},
        {"QPushButton:disabled", "Disabled Buttons"},
        {"QLineEdit", "Editable Text Field"},
        {"QLineEdit::placeholder", "Placeholder Text in Editable Field"},
        {"QCheckBox", "Checkboxes"},
        {"QCheckBox::indicator", "Checkbox Indicator"},
        {"QCheckBox::indicator:checked", "Checked Checkbox Indicator"},
        {"QLabel", "Plain Text/Label"},
        {"QWidget#scrollThing", "Scroll area"},
        {"QScrollBar:vertical", "Vertical Scrollbar"},
        {"QScrollBar:handle:vertical", "Vertical Scrollbar Handle"},
        {"QScrollBar:horizontal", "Horizontal Scrollbar"},
        {"QScrollBar:handle:horizontal", "Horizontal Scrollbar Handle"},
        {"QMessageBox", "Notification Message Box"},
        {"QRadioButton", "Radio Button"},
        {"QRadioButton:indicator", "Radio Button Indicator"},
        {"QRadioButton:indicator:checked", "Checked Radio Button Indicator"},
    };
    std::map<std::string, std::string> widgetProperties = {
        {"background", "ColorPicker"},
        {"color", "ColorPicker"},
        {"border", "LineEdit"},
        {"padding", "LineEdit"},
        {"border-radius", "LineEdit"},
        {"min-width", "LineEdit"},
        {"min-height", "LineEdit"},
        {"width", "LineEdit"},
        {"height", "LineEdit"},
        {"font-size", "LineEdit"}
    };

    QString customTheme;
    bool bSavedTheme = false;
    bool bCheckForUpdates = true;

    // core logic
    void create();
    void _delete();
    void search();
    void updatePass();
    void settings();
    void showPassPolicy();
    void showThemePicker();
    void themeEditor(std::string themePreset);
    void deleteTheme(std::string themeName);
    void updateProperty(std::string widget, std::string property, std::string value);
    QSettings setts;

    void handleActionComplete(bool success, std::string action, std::string errMsg = "", QList<std::string> result = {}, std::string stylesheet = "");

    // helper functions
    template<typename T>
    QList<T*> grabWidgets(QLayout* layout);
    void startPropertyEditor(std::shared_ptr<QButtonGroup> widgets, std::string themePreset);
    void saveTheme(std::string themeName, std::string themePreset);
    void enableDisableCommands(int mode);
    void clearScreen();
    bool isDefaultTheme(std::string themeName);
    void resetSettings();
    void loadSettings();
    void checkForUpdate(QNetworkReply* reply);
    std::shared_ptr<QButtonGroup> renderWidgets(std::map<std::string, std::string> widgetsToRender, std::string textToRender, std::string objName);

    // Top bar
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
private:
    Ui::MainWindow *ui;
    dbManager* db;
    themePreview* preview;

    bool bIsMousePressed;
    QPoint dragStart;

    QNetworkAccessManager* netManager;
    QNetworkRequest req;
};
#endif // WINDOW_H
