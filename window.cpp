#include "./window.h"

////////////////////////////////////////////////////
// CONSTRUCTOR AND CORE LOGIC
////////////////////////////////////////////////////
window::window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , setts("globbertot, simplePassMan")
{
    ui->setupUi(this);
    db = new dbManager() ;
    ui->title->setText("SIMPLE PASSWORD MAN - "+QString::fromStdString(VERSION)+" EARLY ACCESS \nPlease report any bugs to help.simplepassman@gmail.com");
    loadSettings();
    if (!bRememberUser) { window::showMKMenu(); }

    netManager = new QNetworkAccessManager;
    netManager->connect(netManager, &QNetworkAccessManager::finished, this, &window::checkForUpdate);
    req.setUrl(QString::fromStdString(updatesURL));
    netManager->get(req);

    // Connect each command to its proper function
    connect(ui->createBtn, &QPushButton::clicked, this, [this]() { window::create(); });
    connect(ui->deleteBtn, &QPushButton::clicked, this, [this]() { window::_delete(); });
    connect(ui->searchBtn, &QPushButton::clicked, this, [this]() { window::search(); });
    connect(ui->showAllBtn, &QPushButton::clicked, this, [this]() { db->showAll(); });
    connect(ui->updatePassBtn, &QPushButton::clicked, this, [this]() { window::updatePass(); });
    connect(ui->settingsBtn, &QPushButton::clicked, this, [this]() { window::settings(); });

    connect(db, &dbManager::databaseActionCompleted, this, &window::handleActionComplete);

    connect(ui->exitBtn, &QPushButton::clicked, this, exit);
}

window::~window() { delete db;delete netManager;delete ui; }

template<typename widgetType>
widgetType* window::createWidget(const QString& text, const QString& placeHolder,
                                 const QString& objName, const QFont& font, const bool& wordWrap) {
    widgetType* w = new widgetType();
    if constexpr (!std::is_same_v<widgetType, QLineEdit> && !std::is_same_v<widgetType, QScrollArea>) {
        w->setText(text);
        if constexpr (!std::is_same_v<widgetType, QPushButton> && !std::is_same_v<widgetType, QCheckBox>) {
            w->setWordWrap(wordWrap);
        }
    }
    else {
        if constexpr (!std::is_same_v<widgetType, QScrollArea>) {
            w->setPlaceholderText(placeHolder);
        }
    }
    w->setObjectName(objName);
    w->setFont(font);
    return w;
}

void window::showMKMenu() {
    enableDisableCommands(DISABLE_COMMANDS);clearScreen();

    // Title label
    QLabel* title = createWidget<QLabel>(
        "Master key Manager\nPlease enter a phrase or something you can easily remember as without it,\nyour passwords will be lost.\nIf you've already created a master key, you may enter it below.", "", "titleLabel"
        );

    // Master key line edit
    QLineEdit* MKLineEdit = createWidget<QLineEdit>(
        "", "Enter the master key here", "MKLineEdit"
    );

    // Submit button
    QPushButton* submit = createWidget<QPushButton>(
        "Submit", "", "submitBtn"
    );

    // Connections
    MKLineEdit->connect(MKLineEdit, &QLineEdit::returnPressed, this, [this, submit]() { if (submit) { submit->click(); } } );
    submit->connect(submit, &QPushButton::clicked, this, [this, MKLineEdit]() {
        if (!MKLineEdit->text().isEmpty()) {
            db->MKCreator(MKLineEdit->text().toStdString());
        }
    });

    // Add them to the ui
    ui->Managment->addWidget(title);
    ui->Managment->addWidget(MKLineEdit);
    ui->Managment->addWidget(submit);
}

void window::checkForUpdate(QNetworkReply* reply) {
    if (!bCheckForUpdates) { return; }
    if (reply->error()) {
        window::handleActionComplete(false, "CHECKUPDATE", reply->errorString().toStdString());return;
    }

    int versionTag = reply->url().toString().lastIndexOf('/');
    if (versionTag != -1) {
        std::string foundVersion = reply->url().toString().mid(versionTag + 1).toStdString();
        if (foundVersion != VERSION) { // or in other words, if theres an update
            QMessageBox::StandardButton rep;
            rep = QMessageBox::question(this, "Update found!", "Hey! turns out theres a new update, we suggest installing it\nThe application will close automaticly if you choose to install it\nFurther instructions on installation are on the README file", QMessageBox::Ok | QMessageBox::No);
            if (rep == QMessageBox::Ok) {
                QDesktopServices::openUrl(QString::fromStdString(updatesURL));
                exit(0);
            }
        }
    }
}

void window::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && ui->TOPBAR->geometry().contains(e->pos())) {
        bIsMousePressed = true;dragStart = e->pos();
    }
}

void window::mouseMoveEvent(QMouseEvent* e) {
    if (bIsMousePressed) {
        move(pos() + e->pos() - dragStart);
    }
}

void window::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) { bIsMousePressed = false; }
}

void window::handleActionComplete(bool success, std::string action, std::string errMsg, QList<std::string> results, std::string styleSheet) {
    if (styleSheet.empty() && action != "MKCREATION" && action != "UPDATE_THEME") {
        enableDisableCommands(ENABLE_COMMANDS);
        clearScreen();
    } else if (action == "MKCREATION" && success) {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    } else {
        // clear only the results
        QLayoutItem* i;
        while ((i = ui->Results->takeAt(0)) != nullptr) {
            delete i->widget();
            delete i;
        }
    }

    ui->Results->addWidget(createWidget<QLabel>("RESULTS: ", "", "titleLabel", QFont("Arial", 15, QFont::Thin)), 0, Qt::AlignHCenter | Qt::AlignHCenter);

    if (action == "showAll") {
        auto scrollArea = createWidget<QScrollArea>();
        ui->Results->addWidget(scrollArea);
        QWidget* scrollContent = new QWidget();
        scrollContent->setObjectName("scrollThing");
        QVBoxLayout* services = new QVBoxLayout(scrollContent);

        for (int i = 0; i < results.count(); ++i) {
            QLabel* service = createWidget<QLabel>(QString::number(i) + ") " + QString::fromStdString(results.at(i)));
            services->addWidget(service);
        }

        if (!services->isEmpty()) {
            scrollArea->setWidget(scrollContent);
            ui->Results->addWidget(scrollArea);
        } else {
            delete scrollContent;
            delete scrollArea;
        }
        return;
    }

    QLabel* detail = createWidget<QLabel>(
        ((success) ? "SUCCESS!" : QString::fromStdString(errMsg))
    );
    if (action == "KILLSWITCH") {
        detail->setText((success) ? "Everything has been erased, a restart of the program is required." : QString::fromStdString(errMsg));
    }

    if (action == "MKCREATION") {
        detail->setText((success) ? "Logged in!" : QString::fromStdString(errMsg));
    }

    if (action == "UPDATE_THEME") {
        detail->setText((success) ? "Updated widget stylesheet: \n" + QString::fromStdString(styleSheet) : QString::fromStdString(errMsg));
    }

    if (action == "SEARCH") {
        detail->setText((success) ? ((bShowPasswords) ? QString::fromStdString(results.first()) : "REDACTED") : QString::fromStdString(errMsg));

        QPushButton* copyToClipboardBtn = createWidget<QPushButton>("Copy to clipboard", "", "", QFont("Arial", 13), false);
        ui->Results->addWidget(copyToClipboardBtn, 0, Qt::AlignTop | Qt::AlignRight);
        connect(copyToClipboardBtn, &QPushButton::clicked, this, [this, results] { QApplication::clipboard()->setText(QString::fromUtf8(results.first())); });
    }

    ui->Results->addWidget(detail, 0, Qt::AlignHCenter | Qt::AlignTop);
}

////////////////////////////////////////////////////
// DATABASE MANAGEMENT FUNCTIONS
///////////////////////////////////////////////////
void window::create() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    // Create button
    QPushButton* createBtn = createWidget<QPushButton>(
        "Create!", "", "createBtn"
    );

    // Password textline
    QLineEdit* password = createWidget<QLineEdit>(
        "", "Enter the actual password", "passwordLine"
    );

    // Service textline
    QLineEdit* service = createWidget<QLineEdit>(
        "", "Enter the service you want this password to be for", "serviceLine"
    );

    // Password create automatically checkbox
    QCheckBox* autoPassGen = createWidget<QCheckBox>(
        "Create a password for me", "", "passAutoGen"
    );

    // Return button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Connections
    createBtn->connect(createBtn, &QPushButton::clicked, this, [this, service, password, autoPassGen]() {
        if (autoPassGen->isChecked()) {
            std::string pass = db->generatePass();
            if (!service->text().isEmpty()) {
                db->create(service->text().toStdString(), pass);
            }
        }
        else {
            if (!service->text().isEmpty() && !password->text().isEmpty()) {
                db->create(service->text().toStdString(), password->text().toStdString());
            }
        }
    });
    password->connect(password, &QLineEdit::returnPressed, this, [this, createBtn]() { if (createBtn) {createBtn->click();} });
    service->connect(service, &QLineEdit::returnPressed, this, [this, password]() { if (password) {password->setFocus(); } });
    autoPassGen->setChecked(bAlwaysGeneratePass);
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen();enableDisableCommands(ENABLE_COMMANDS); });

    // Add them to the ui
    ui->Managment->addWidget(service);
    ui->Managment->addWidget(password);
    ui->Managment->addWidget(autoPassGen);
    ui->Managment->addWidget(createBtn);
    ui->Managment->addWidget(goBack);
}

void window::_delete() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    // Service edit line
    QLineEdit* service = createWidget<QLineEdit>(
        "", "Enter the password's service you want to delete", "serviceLine"
    );

    // Delete button
    QPushButton* deleteBtn = createWidget<QPushButton>(
        "Delete", "", "deleteBtn"
    );

    // Return button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Connections
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    });

    service->connect(service, &QLineEdit::returnPressed, this, [this, deleteBtn]() {if (deleteBtn) {deleteBtn->click();} });

    deleteBtn->connect(deleteBtn, &QPushButton::clicked, this, [this, service]() {
        if (!service->text().isEmpty()) {
            db->_delete(service->text().toStdString());
        }
    });

    // Adding them to the ui
    ui->Managment->addWidget(service);
    ui->Managment->addWidget(deleteBtn);
    ui->Managment->addWidget(goBack);
}

void window::search() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    // Service line edit
    QLineEdit* service = createWidget<QLineEdit>(
        "", "Enter the service of the password you want to grab", "serviceLine"
    );

    // Search button
    QPushButton* searchBtn = createWidget<QPushButton>(
        "Read", "", "searchBtn"
    );

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Connections
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    });

    service->connect(service, &QLineEdit::returnPressed, this, [searchBtn]() {if (searchBtn) {searchBtn->click();} });

    searchBtn->connect(searchBtn, &QPushButton::clicked, this, [this, service](){
        if (!service->text().isEmpty()) {
            db->search(service->text().toStdString());
        }
    });

    // Add them to the ui
    ui->Managment->addWidget(service);
    ui->Managment->addWidget(searchBtn);
    ui->Managment->addWidget(goBack);
}

void window::updatePass() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    // Service line edit
    QLineEdit* service = createWidget<QLineEdit>(
        "", "Enter the service of the password you want to change", "serviceLine"
    );

    // Password line edit
    QLineEdit* pass = createWidget<QLineEdit>(
        "", "Enter the updated password", "passLine"
    );

    // Update button
    QPushButton* updateBtn = createWidget<QPushButton>(
        "Update", "", "updateBtn"
    );

    // Automatically generate passwords checkbox
    QCheckBox* autoPassGen = createWidget<QCheckBox>(
        "Create a password for me", "passAutoGen"
    );
    autoPassGen->setChecked(bAlwaysGeneratePass);

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Connections
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    });

    service->connect(service, &QLineEdit::returnPressed, this, [this, pass]() {if (pass) {pass->setFocus();} });
    pass->connect(pass, &QLineEdit::returnPressed, this, [this, updateBtn]() {if (updateBtn) {updateBtn->click();} });

    updateBtn->connect(updateBtn, &QPushButton::clicked, this, [this, service, pass, autoPassGen]() {
        if (autoPassGen->isChecked()) {
            std::string pass = db->generatePass();
            if (!service->text().isEmpty()) {
                db->updatePassword(service->text().toStdString(), pass);
            }
        }
        else {
            if (!service->text().isEmpty() && !pass->text().isEmpty()) {
                db->updatePassword(service->text().toStdString(), pass->text().toStdString());
            }
        }
    });

    // Adding them to the ui
    ui->Managment->addWidget(service);
    ui->Managment->addWidget(pass);
    ui->Managment->addWidget(updateBtn);
    ui->Managment->addWidget(autoPassGen);
    ui->Managment->addWidget(goBack);
}

////////////////////////////////////////////////////
// SETTINGS RELATED FUNCTIONS
////////////////////////////////////////////////////
void window::loadSettings() {
    // Simple constants
    bShowPasswords = setts.value("showPasswords", false).toBool();
    bAlwaysGeneratePass = setts.value("alwaysGeneratePasswords", true).toBool();
    bCheckForUpdates = setts.value("checkForUpdates", true).toBool();
    bRememberUser = setts.value("rememeberUser", false).toBool();

    // dynamic theme
    currentTheme = setts.value("currTheme").toString().toStdString();
    QVariantMap themesMap = setts.value("themes").toMap();
    for (auto it = themesMap.begin(); it != themesMap.end(); ++it) {
        themes[it.key().toStdString()] = it.value().toString().toStdString();
    }
    this->setStyleSheet(QString::fromStdString(themes[currentTheme]));
}

void window::resetSettings() {
    // simple variables
    setts.setValue("showPasswords", false);
    setts.setValue("alwaysGeneratePasswords", true);
    setts.setValue("checkForUpdates", true);
    setts.setValue("rememeberUser", false);

    // database reset
    QSqlDatabase::database("main").close();
    QFile dbToDelete(db->dbPath+"/simplePassMan.db");
    if (!dbToDelete.remove()) {
        window::handleActionComplete(false, "KILLSWITCH", dbToDelete.errorString().toStdString());
    }
    window::handleActionComplete(true, "KILLSWITCH");

    // theme reset
    QVariantMap themesMap;
    for (const auto& theme : themes) {
        if (!isDefaultTheme(theme.first)) { continue; }
        themesMap[QString::fromStdString(theme.first)] = QString::fromStdString(theme.second);
    }
    setts.setValue("themes", QVariant(themesMap));
    setts.setValue("currTheme", "DARK");
}

void window::settings() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    // Pass policy edit button
    QPushButton* passPolicyEditBtn = createWidget<QPushButton>(
        "Edit password policy", "", "passPolicyEdit"
    );

    // Theme picker button
    QPushButton* themePicker = createWidget<QPushButton>(
        "Theme picker (WIP)", "", "themePicker"
    );

    // Clear data button
    QPushButton* clearDataBtn = createWidget<QPushButton>(
        "DELETE EVERYTHING", "", "clearDataBtn"
    );

    // Hide passwords checkbox
    // Thus only allow passwords to be copied to the clipboard
    QCheckBox* hidePasswordsBtn = createWidget<QCheckBox>(
        "Show Passwords", "", "hidePasswords"
    );
    hidePasswordsBtn->setChecked(bShowPasswords);

    // Always generate passwords checkbox
    QCheckBox* alwaysGenPasswordsBtn = createWidget<QCheckBox>(
        "Always generate passwords automatically", "", "alwaysGenPasswords"
    );
    alwaysGenPasswordsBtn->setChecked(bAlwaysGeneratePass);

    // Check for updates checkbox
    QCheckBox* checkForUpdatesBtn = createWidget<QCheckBox>(
        "Check for updates", "", "checkForUpdates"
    );
    checkForUpdatesBtn->setChecked(bCheckForUpdates);

    QCheckBox* rememberMeBtn = createWidget<QCheckBox>(
        "Remember me (not advised as this completely removes the master key system)", "", "rememberBtn"
    );
    rememberMeBtn->setChecked(bRememberUser);

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Connections
    passPolicyEditBtn->connect(passPolicyEditBtn, &QPushButton::clicked, this, [this]() {showPassPolicy();} );

    themePicker->connect(themePicker, &QPushButton::clicked, this, [this]() { showThemePicker(); });

    clearDataBtn->connect(clearDataBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton rep;
        rep = QMessageBox::question(this, "WARNING", "(!!) Are you sure you want to delete everything, all your data will be gone including custom themes. (!!)", QMessageBox::Yes | QMessageBox::No);
        if (rep == QMessageBox::Yes) {
            // Procceed deletion
            db->resetSettings();
            resetSettings();
        }
    });

    hidePasswordsBtn->connect(hidePasswordsBtn, &QCheckBox::clicked, this, [this, hidePasswordsBtn]() {
        bShowPasswords = !bShowPasswords;
        hidePasswordsBtn->setChecked(bShowPasswords);
        setts.setValue("showPasswords", bShowPasswords);
    } );

    alwaysGenPasswordsBtn->connect(alwaysGenPasswordsBtn, &QCheckBox::clicked, this, [this, alwaysGenPasswordsBtn]() {
        bAlwaysGeneratePass = !bAlwaysGeneratePass;
        alwaysGenPasswordsBtn->setChecked(bAlwaysGeneratePass);
        setts.setValue("alwaysGeneratePasswords", bAlwaysGeneratePass);
    });

    checkForUpdatesBtn->connect(checkForUpdatesBtn, &QCheckBox::clicked, this, [this, checkForUpdatesBtn]() {
        bCheckForUpdates = !bCheckForUpdates;
        checkForUpdatesBtn->setChecked(bCheckForUpdates);
        setts.setValue("checkForUpdates", bCheckForUpdates);
    });

    rememberMeBtn->connect(rememberMeBtn, &QCheckBox::clicked, this, [this, rememberMeBtn]() {
        bRememberUser = !bRememberUser;
        rememberMeBtn->setChecked(bRememberUser);
        setts.setValue("rememeberUser", bRememberUser);
    });

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen();enableDisableCommands(ENABLE_COMMANDS); });


    // Adding them to the ui
    ui->Managment->addWidget(passPolicyEditBtn);
    ui->Managment->addWidget(themePicker);
    ui->Managment->addWidget(clearDataBtn);
    ui->Managment->addWidget(hidePasswordsBtn);
    ui->Managment->addWidget(alwaysGenPasswordsBtn);
    ui->Managment->addWidget(checkForUpdatesBtn);
    ui->Managment->addWidget(rememberMeBtn);
    ui->Managment->addWidget(goBack);
}

void window::showPassPolicy() {
    window::clearScreen();

    // Title label
    QLabel* title = createWidget<QLabel>(
        "Password policy customizer\nPick the variable you wish to edit:", "", "titleLabel"
    );

    // Pick button
    QPushButton* pick = createWidget<QPushButton>(
        "Select", "", "selectBtn"
    );

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen();settings(); } );

    // Add the first menu's widgets
    ui->Managment->addWidget(title);
    // Grabbing the buttons we need and renderning them
    std::map<std::string, std::string> passPolicyString;
    for (const auto& entry : db->passPolicy) {
        passPolicyString[entry.first] = std::to_string(entry.second);
    }
    auto buttons = window::renderWidgets(passPolicyString, "first", "first");
    // Add some missing properties
    for (auto it = db->passPolicy.begin(); it!=db->passPolicy.end();++it) {
        QRadioButton* btn = new QRadioButton();
        btn->setText(QString::fromStdString(it->first));
        btn->setObjectName(QString::fromStdString(it->first));

        ui->Managment->addWidget(btn);
        buttons->addButton(btn);
    }
    ui->Managment->addWidget(pick);
    ui->Managment->addWidget(goBack, 5, Qt::AlignRight | Qt::AlignBottom);

    // Second menu - Selecting a variable to edit
    pick->connect(pick, &QPushButton::clicked, this, [this, buttons = std::move(buttons)]() {
        QRadioButton* selected = qobject_cast<QRadioButton*>(buttons->checkedButton());
        if (selected) {
            std::string objName = selected->objectName().toStdString();
            clearScreen();

            // Title label
            QLabel* title = createWidget<QLabel>(
                "Editing '"+QString::fromStdString(objName)+"'", "", "titleLabel"
            );

            // Description label
            QLabel* desc = createWidget<QLabel>(
                "CURRENT VALUE: "+QString::number(db->passPolicy[objName]), "", "descLabel", QFont("Arial", 14)
            );

            // Updated value line edit
            QLineEdit* newValue = createWidget<QLineEdit>(
                "", "Enter the new value here", "updatedValueLineEdit", QFont("Arial", 12)
            );

            // Update button
            QPushButton* updateBtn = createWidget<QPushButton>(
                "Update!", "", "updateBtn"
            );

            // Go back button
            QPushButton* goBack = createWidget<QPushButton>(
                "Nevermind, go back", "", "goBack"
            );

            // Connections
            updateBtn->connect(updateBtn, &QPushButton::clicked, this, [this, objName, newValue]() { if (newValue->text().toInt()) {db->updatePassPolicy(objName, newValue->text().toInt());clearScreen();settings(); } } );
            goBack->connect(goBack, &QPushButton::clicked, this, [this]() { showPassPolicy(); });

            // Adding them to the ui
            ui->Managment->addWidget(title);
            ui->Managment->addWidget(desc, 5, Qt::AlignTop);
            ui->Managment->addWidget(newValue, Qt::AlignCenter);
            ui->Managment->addWidget(updateBtn, 5, Qt::AlignCenter | Qt::AlignHCenter);
            ui->Managment->addWidget(goBack, 5, Qt::AlignRight | Qt::AlignBottom);
        }
    });
}

void window::showThemePicker() {
    // Clear the screen before showing the theme picker
    window::clearScreen();

    // Title label
    QLabel* title = createWidget<QLabel>(
        "Theme picker", "", "titleLabel", QFont("Arial", 16)
    );

    // Pick button
    QPushButton* pick = createWidget<QPushButton>(
        "Select", "", "selectBtn"
    );

    // Export button
    QPushButton* exportBtn = createWidget<QPushButton>(
        "Export", "", "exportBtn"
    );

    // Import button
    QPushButton* importBtn = createWidget<QPushButton>(
        "Import", "", "importBtn"
    );

    // Remove theme button
    QPushButton* removeThemeBtn = createWidget<QPushButton>(
        "Delete selected theme", "", "deleteSelected"
    );

    // Create new theme button
    QPushButton* createNew = createWidget<QPushButton>(
        "(ADVANCED) Create a new theme by a preset (USE AT YOUR OWN RISK)", "", "createNewThemeBtn"
    );

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Adding them to the ui
    ui->Managment->addWidget(title);
    auto themesBtns = window::renderWidgets(themes, "first", "first");
    ui->Managment->addWidget(pick);
    ui->Managment->addWidget(exportBtn);
    ui->Managment->addWidget(importBtn);
    ui->Managment->addWidget(removeThemeBtn);
    ui->Managment->addWidget(createNew);
    ui->Managment->addWidget(goBack);

    // Connections
    connect(pick, &QPushButton::clicked, this, [this, themesBtns]() {
        QRadioButton* selected = qobject_cast<QRadioButton*>(themesBtns->checkedButton());
        if (selected) {
            // Set the current theme and update the style sheet
            currentTheme = selected->text().toStdString();
            setts.setValue("currTheme", QString::fromStdString(currentTheme));
            this->setStyleSheet(QString::fromStdString(themes[currentTheme]));
        }
    });

    connect(createNew, &QPushButton::clicked, this, [this, themesBtns]() {
        QRadioButton* checkedButton = qobject_cast<QRadioButton*>(themesBtns->checkedButton());
        if (checkedButton) {
            // Set up custom theme and start the theme editor
            customTheme = QString::fromStdString(themes[checkedButton->text().toStdString()]);
            themeEditor(checkedButton->text().toStdString());
        }
    });

    exportBtn->connect(exportBtn, &QPushButton::clicked, this, [this, themesBtns]() {
        QRadioButton* selected = qobject_cast<QRadioButton*>(themesBtns->checkedButton());
        if (selected) {
            QApplication::clipboard()->setText(QString::fromUtf8(themes[selected->text().toStdString()]));
            handleActionComplete(true, "EXPORT");
        }
    });

    removeThemeBtn->connect(removeThemeBtn, &QPushButton::clicked, this, [this, themesBtns]() {
        QRadioButton* selected = qobject_cast<QRadioButton*>(themesBtns->checkedButton());
        if (selected) {
            // Call function to delete the selected theme
            deleteTheme(selected->text().toStdString());
        }
    });

    importBtn->connect(importBtn, &QPushButton::clicked, this, &window::showImportThemeMenu);
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen(); settings(); });
}

void window::showImportThemeMenu() {
    window::clearScreen();

    // Theme name line edit
    QLineEdit* themeName = createWidget<QLineEdit>(
        "", "Enter the imported theme's name here", "themeName"
    );

    // Theme style line edit
    QLineEdit* themeStyle = createWidget<QLineEdit>(
        "", "Paste the theme here", "themeStyle"
    );

    // Submit button
    QPushButton* submit = createWidget<QPushButton>(
        "Submit", "", "submitBtn"
    );

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Connections
    submit->connect(submit, &QPushButton::clicked, this, [this, themeName, themeStyle]() {
        if (!themeName->text().isEmpty() && !themeStyle->text().isEmpty()) {
            preview = new themePreview();
            preview->move(this->geometry().right() - 20, this->geometry().top());
            preview->setStyleSheet(themeStyle->text());
            preview->show();

            QMessageBox::StandardButton confirm;
            confirm = QMessageBox::question(this, "Are you sure?", "As you can see, this is a preview of what the theme you just paste will look like.\nDo you really want to create it?", QMessageBox::Yes | QMessageBox::No);
            if (confirm == QMessageBox::Yes) {
                customTheme = themeStyle->text();
                saveTheme(themeName->text().toStdString());
                preview->close();
            } else {
                preview->close();
            }
        }
    });
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen(); showThemePicker(); });

    // Adding them to the ui
    ui->Managment->addWidget(themeName);
    ui->Managment->addWidget(themeStyle);
    ui->Managment->addWidget(submit);
    ui->Managment->addWidget(goBack);
}

void window::deleteTheme(std::string themeName) {
    if (!isDefaultTheme(themeName)) {
        if (themes.count(themeName) > 0) {
            themes.erase(themeName);
            QVariantMap themesMap;
            for (const auto& theme : themes) {
                themesMap[QString::fromStdString(theme.first)] = QString::fromStdString(theme.second);
            }
            setts.setValue("themes", QVariant(themesMap));
            if (currentTheme == themeName) {
                currentTheme = "DARK";
                setts.setValue("currTheme", "DARK");
                this->setStyleSheet(QString::fromStdString(themes[currentTheme]));
            }
            handleActionComplete(true, "REMOVETHEME");
        } else {
            handleActionComplete(false, "REMOVETHEME", "theme not found");
        }
    } else {
        handleActionComplete(false, "REMOVETHEME", "Cannot remove default themes");
    }
}

void window::saveTheme(std::string themeName) {
    if (!themeName.empty()) {
        if (themes.find(themeName) == themes.end()) {
            // Add it to the list since it doesnt exist
            bSavedTheme = true;
            themes.insert({themeName, customTheme.toStdString()});

            // Save the theme
            currentTheme = customTheme.toStdString();
            setts.setValue("currTheme", QString::fromStdString(themeName));
            this->setStyleSheet(QString::fromStdString(currentTheme));
            QVariantMap themesMap;
            for (const auto& pair : themes) {
                themesMap[QString::fromStdString(pair.first)] = QString::fromStdString(pair.second);
            }
            setts.setValue("themes", themesMap);

            window::clearScreen();
            window::showThemePicker();
        } else {
            // It already exists fail
            window::handleActionComplete(false, "saveTheme", "Theme already exists", {}, customTheme.toStdString());
        }
    } else {
        // Theme name must not be empty fail
        window::handleActionComplete(false, "saveTheme", "Theme name must not be empty", {}, customTheme.toStdString());
    }
}

void window::startPropertyEditor(std::shared_ptr<QButtonGroup> widgets, std::string themePreset) {
    // Create a preview window and position it to the top right corner of the main window
    preview = new themePreview();
    preview->move(this->geometry().right() - 20, this->geometry().top());

    // Retrieve the name of the widget to be edited
    std::string widgetToEdit;
    QAbstractButton* s = widgets->checkedButton();
    if (s) {
        widgetToEdit = s->objectName().toStdString();
    }
    window::clearScreen();
    auto properties = window::renderWidgets(widgetProperties, "first", "first");

    // Create an 'Edit' button to trigger property editing
    // Edit button
    QPushButton* edit = createWidget<QPushButton>(
        "Edit", "", "editBtn"
    );

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back (save changes)", "", "goBack"
    );

    // Connections
    // Second menu / editing menu
    edit->connect(edit, &QPushButton::clicked, this, [this, widgetToEdit, properties]() {
        // Determine the type of property being edited
        std::string propertyToEdit;
        std::string propertyType;
        QAbstractButton* s2 = properties->checkedButton();
        if (s2) {
            propertyToEdit = s2->objectName().toStdString();
            auto it = widgetProperties.find(propertyToEdit);
            if (it != widgetProperties.end()) {
                propertyType = it->second;
            }
        }

        // If the property type is a ColorPicker, open a color dialog for selection
        if (propertyType == "ColorPicker") {
            QColorDialog dialog;
            QColor color = dialog.getColor();
            if (color.isValid()) {
                // Update the selected property with the chosen color
                updateProperty(widgetToEdit, propertyToEdit, color.name().toStdString());
                // Apply changes to the preview window
                preview->setStyleSheet(customTheme);
            }
        }
        // If the property type is a LineEdit, create a text input field for editing
        else if (propertyType == "LineEdit") {
            // Clear any existing LineEdit widgets
            QList<QLineEdit*> previousItems = window::grabWidgets<QLineEdit>(ui->Managment);
            if (!previousItems.empty()) {
                for (QLineEdit* i : previousItems) {
                    delete i;
                }
            }
            // Create a new LineEdit widget for text input
            QLineEdit* edit = new  QLineEdit();
            edit->setPlaceholderText("Enter the new value here");
            edit->connect(edit, &QLineEdit::editingFinished, this, [this, edit, propertyToEdit, widgetToEdit]() {
                // Update the selected property with the entered text
                updateProperty(widgetToEdit, propertyToEdit, edit->text().toStdString());
                // Apply changes to the preview window
                preview->setStyleSheet(customTheme);
            });
            // Add the LineEdit widget to the layout
            ui->Managment->addWidget(edit);
        }
    });

    goBack->connect(goBack, &QPushButton::clicked, this, [this, themePreset]() {
        // Close the preview window and return to the theme editor
        preview->close();
        clearScreen();
        themeEditor(themePreset);
    });

    // Adding the first menu widgets onto the ui
    ui->Managment->addWidget(edit);
    ui->Managment->addWidget(goBack);

    // Apply the current custom theme to the preview window and show it
    preview->setStyleSheet(customTheme);
    preview->show();
}

void window::themeEditor(std::string themePreset) {
    // Clear the screen and reset the flag for saved theme
    window::clearScreen();
    bSavedTheme = false;

    // Title label
    QLabel* title = createWidget<QLabel>(
        "THEME EDITOR 0.1\nYou are using "+QString::fromStdString(themePreset)+
        " theme as a preset\nYou may update properties from the widgets below", "", "titleLabel", QFont("Arial", 9)
    );
    ui->Managment->addWidget(title);

    // Render all the editable widgets
    auto widgets = window::renderWidgets(editableWidgets, "second", "first");

    // Theme Name line edit
    QLineEdit* themeName = createWidget<QLineEdit>(
        "", "Enter your theme's name here", "themeName"
    );

    // Save button
    QPushButton* save = createWidget<QPushButton>(
        "Save", "", "saveBtn"
    );

    // Edit button
    QPushButton* edit = createWidget<QPushButton>(
        "Edit", "", "editBtn"
    );

    // Go back button
    QPushButton* goBack = createWidget<QPushButton>(
        "Nevermind, go back", "", "goBack"
    );

    // Connections
    save->connect(save, &QPushButton::clicked, this, [this, themeName]() { saveTheme(themeName->text().toStdString()); });
    edit->connect(edit, &QPushButton::clicked, this, std::bind(&window::startPropertyEditor, this, widgets, themePreset));

    goBack->connect(goBack, &QPushButton::clicked, this, [this, save]() {
        if (!bSavedTheme) {
            // Prompt user if the theme hasn't been saved before going back
            QMessageBox::StandardButton questionSave;
            questionSave = QMessageBox::question(this, "Warning!", "You haven't saved the theme you are currently editing\nWould you like to save it now and go back or go back without saving, all progress will be gone if you choose the latter.", QMessageBox::Save | QMessageBox::Ignore);
            if (questionSave == QMessageBox::Save) {
                save->click();
                return;
            }
        }
        // Clear the screen and go back to theme picker
        window::clearScreen();
        window::showThemePicker();
    });

    // Add widgets to the layout
    ui->Managment->addWidget(themeName);
    ui->Managment->addWidget(save);
    ui->Managment->addWidget(edit);
    ui->Managment->addWidget(goBack);
}

void window::updateProperty(std::string widget, std::string property, std::string value) {
    QString widgetStyle;

    // Find the starting position of the widget in the custom theme string
    size_t start = customTheme.trimmed().indexOf(QString::fromStdString(widget).trimmed());

    if (start != std::string::npos) {
        // Find the ending position of the widget's style block
        size_t end = customTheme.trimmed().indexOf("}", start);

        if (end != std::string::npos) {
            // Extract the style block of the widget
            widgetStyle = customTheme.mid(start, end - start + 1);

            // Check if the property already exists in the widget's style
            size_t edit = widgetStyle.trimmed().indexOf(QString::fromStdString(property).trimmed());
            if (edit != std::string::npos) {
                // Property exists, replace it with the new value
                size_t propEnd = widgetStyle.trimmed().indexOf(";", edit);
                widgetStyle.replace(edit, propEnd - edit, QString::fromStdString(property + ": " + value));

                customTheme.replace(start, end - start+1, widgetStyle);
            } else {
                // Property does not exist, add it to the style
                size_t insertPos = widgetStyle.lastIndexOf(";");
                if (insertPos != std::string::npos) {
                    // Insert the new property as the last one
                    widgetStyle.insert(insertPos+1, " " + QString::fromStdString(property + ": " + value + ";"));
                } else {
                    // There are no properties, add the new property to the end
                    widgetStyle.append(" " + QString::fromStdString(property + ": " + value + ";"));
                }
                customTheme.replace(start, end - start + 1, widgetStyle);
            }
        }
    } else {
        // Widget does not exist in the theme, add it with the provided property and value
        customTheme.append(QString::fromStdString(widget + "{ " + property + ": " + value + "; }"));
        widgetStyle = customTheme;
    }
    window::handleActionComplete(true, "UPDATE_THEME", "", {}, widgetStyle.toStdString());
}

////////////////////////////////////////////////////
// HELPER FUNCTIONS
////////////////////////////////////////////////////
std::shared_ptr<QButtonGroup> window::renderWidgets(std::map<std::string, std::string> widgets, std::string textToRender, std::string objName) {
    auto properties = std::make_shared<QButtonGroup>();
    for (auto it = widgets.begin(); it != widgets.end(); ++it) {
        QRadioButton* btn = new QRadioButton();
        btn->setText(QString::fromStdString((textToRender == "first") ? it->first : it->second));
        btn->setObjectName(QString::fromStdString((objName == "first") ? it->first : it->second));

        ui->Managment->addWidget(btn);
        properties->addButton(btn);
    }
    return properties;
}

// With T being widget type //
template<typename T>
QList<T*> window::grabWidgets(QLayout* layout) {
    QList<T*> widgetList;

    if (!layout)
        return widgetList;

    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem* item = layout->itemAt(i);
        if (item) {
            QWidget* widget = item->widget();
            if (!widget) {
                QLayout* subLayout = item->layout();
                if (subLayout) {
                    widgetList.append(grabWidgets<T>(subLayout));
                }
            } else if (T* castedWidget = qobject_cast<T*>(widget)) {
                widgetList.append(castedWidget);
            }
        }
    }

    return widgetList;
}

void window::enableDisableCommands(int mode) {
    // 0 for disable, 1 to enable, anything else disables
    QList<QPushButton*> commands = window::grabWidgets<QPushButton>(ui->COMMANDS);
    bool final = (mode == 1) ? true : false;
    for (QPushButton* cmd : commands) { cmd->setEnabled(final); }
}

void window::clearScreen() {
    QLayoutItem* i;
    while ((i = ui->Managment->takeAt(0)) != nullptr) {
        delete i->widget();delete i;
    }
    while ((i = ui->Results->takeAt(0)) != nullptr) {
        delete i->widget();delete i;
    }
}

bool window::isDefaultTheme(std::string themeName) {
    return (themeName == "DARK" || themeName == "LIGHT" || themeName == "DARKBLUE" || themeName == "LIGHTBLUE" ||
            themeName == "GREEN" || themeName == "NONE");
}
