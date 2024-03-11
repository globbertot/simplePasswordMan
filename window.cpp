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
    ui->title->setText("SIMPLE PASSWORD MAN - "+QString::fromStdString(VERSION)+" EARLY ACCESS \nPlease report any bugs to help.simplepassman@gmail.com");

    loadSettings();
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

    db = new dbManager() ;
    connect(db, &dbManager::databaseActionCompleted, this, &window::handleActionComplete);

    connect(ui->exitBtn, &QPushButton::clicked, this, exit);
}

window::~window() { delete db;delete netManager;delete ui; }

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

// Needs a rewrite
void window::handleActionComplete(bool success, std::string action, std::string errMsg, QList<std::string> results, std::string styleSheet) {
    if (styleSheet.empty()) { window::enableDisableCommands(ENABLE_COMMANDS);window::clearScreen();} else {
        QLayoutItem* i;
        while ((i = ui->Results->takeAt(0)) != nullptr) {
            delete i->widget();delete i;
        }
    }
    // Title Label
    QLabel* titleLabel = new QLabel();
    titleLabel->setText("RESULTS: ");
    titleLabel->setFont(QFont("Arial", 15, QFont::Thin));
    titleLabel->setObjectName("titleLabel");

    ui->Results->addWidget(titleLabel, 0, Qt::AlignHCenter | Qt::AlignTop);

    if (action == "showAll") {
        // Make the scroll work
        QScrollArea* scrollArea = new QScrollArea();
        QWidget* scrollContent = new QWidget();scrollContent->setObjectName("scrollThing");
        QVBoxLayout* services = new QVBoxLayout(scrollContent);

        for (int i=0; i<results.count();++i) {
            QLabel* service = new QLabel();
            service->setText(QString::number(i)+") "+QString::fromStdString(results.at(i)));
            services->addWidget(service);
        }
        if (!services->isEmpty()) {
            scrollArea->setWidget(scrollContent);
            ui->Results->addWidget(scrollArea);
        } else { delete scrollContent;delete scrollArea; }return;
    }

    QLabel* detail = new QLabel();
    detail->setFont(QFont("Arial", 13));
    detail->setWordWrap(true);
    detail->setText((success) ? "SUCCESS" : QString::fromStdString(errMsg));

    if (action == "KILLSWITCH") {
        detail->setText((success) ? "Everything has been erased, a restart of the program is required." : QString::fromStdString(errMsg));
    }

    if (action == "UPDATE_THEME") {
        detail->setText((success) ? "Updated widget stylesheet: \n" + QString::fromStdString(styleSheet) : QString::fromStdString(errMsg) );
    }

    if (action == "SEARCH") {
        // Set text accordingly:
        // if it succeeded,
        // check if showPass is true
        // if yes, display the pass
        // if not, display REDACTED
        // if it didnt succeed, log the err msg
        detail->setText((success) ? ((bShowPasswords) ? QString::fromStdString(results.first()) : "REDACTED") : QString::fromStdString(errMsg));
        QPushButton* copyToClipboardBtn = new QPushButton();
        copyToClipboardBtn->setText("Copy to clipboard");
        copyToClipboardBtn->setFixedSize(150, 30);
        copyToClipboardBtn->setFont(QFont("Arial", 13));

        copyToClipboardBtn->connect(copyToClipboardBtn, &QPushButton::clicked, this, [this, results] { QApplication::clipboard()->setText(QString::fromUtf8(results.first())); });
        ui->Results->addWidget(copyToClipboardBtn, 0, Qt::AlignTop | Qt::AlignRight);
    }

    ui->Results->addWidget(detail, 0, Qt::AlignHCenter | Qt::AlignTop);
}

////////////////////////////////////////////////////
// DATABASE MANAGEMENT FUNCTIONS
///////////////////////////////////////////////////
void window::create() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    // Service textline
    QLineEdit* service = new QLineEdit();
    service->setPlaceholderText("Enter the service you want this password to be for");
    service->setObjectName("serviceLine");

    // Password textline
    QLineEdit* password = new QLineEdit();
    password->setPlaceholderText("Enter the actual password");
    password->setObjectName("passwordLine");

    // Password create automatically checkbox
    QCheckBox* autoPassGen = new QCheckBox();
    autoPassGen->setObjectName("passAutoGen");
    autoPassGen->setText("Create a password for me");
    autoPassGen->setChecked(bAlwaysGeneratePass);

    // Create button
    QPushButton* createBtn = new QPushButton();
    createBtn->setObjectName("createBtn");
    createBtn->setText("Create!");

    // Return button
    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("go back");
    goBack->setText("Nevermind, go back");

    service->connect(service, &QLineEdit::returnPressed, this, [this, password]() {password->setFocus();});
    password->connect(password, &QLineEdit::returnPressed, this, [this, createBtn]() {createBtn->click();});

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

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    });

    // Add them to the ui
    ui->Managment->addWidget(service);
    ui->Managment->addWidget(password);
    ui->Managment->addWidget(autoPassGen);
    ui->Managment->addWidget(createBtn);
    ui->Managment->addWidget(goBack);
}

void window::_delete() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    // Service textline
    QLineEdit* service = new QLineEdit();
    service->setPlaceholderText("Enter the password's service you want to delete");
    service->setObjectName("serviceLine");

    // Delete button
    QPushButton* deleteBtn = new QPushButton();
    deleteBtn->setObjectName("deleteBtn");
    deleteBtn->setText("Delete");

    // Return button
    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("go back");
    goBack->setText("Nevermind, go back");

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    });

    service->connect(service, &QLineEdit::returnPressed, this, [this, deleteBtn]() {deleteBtn->click();});

    deleteBtn->connect(deleteBtn, &QPushButton::clicked, this, [this, service]() {
        if (!service->text().isEmpty()) {
            db->_delete(service->text().toStdString());
        }
    });

    ui->Managment->addWidget(service);
    ui->Managment->addWidget(deleteBtn);
    ui->Managment->addWidget(goBack);
}

void window::search() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    QLineEdit* service = new QLineEdit();
    service->setPlaceholderText("Enter the service of the password you want to grab");
    service->setObjectName("serviceLine");

    QPushButton* searchBtn = new QPushButton();
    searchBtn->setObjectName("searchBtn");
    searchBtn->setText("Read");

    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("go back");
    goBack->setText("Nevermind, go back");

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    });

    service->connect(service, &QLineEdit::returnPressed, this, [searchBtn]() {searchBtn->click();});

    searchBtn->connect(searchBtn, &QPushButton::clicked, this, [this, service](){
        if (!service->text().isEmpty()) {
            db->search(service->text().toStdString());
        }
    });

    ui->Managment->addWidget(service);
    ui->Managment->addWidget(searchBtn);
    ui->Managment->addWidget(goBack);
}

void window::updatePass() {
    window::enableDisableCommands(DISABLE_COMMANDS);

    QLineEdit* service = new QLineEdit();
    service->setPlaceholderText("Enter the service of the password you want to change");
    service->setObjectName("serviceLine");

    QLineEdit* pass = new QLineEdit();
    pass->setPlaceholderText("Enter the updated password");
    pass->setObjectName("passLine");

    QPushButton* updateBtn = new QPushButton();
    updateBtn->setText("Update");
    updateBtn->setObjectName("updateBtn");

    QCheckBox* autoPassGen = new QCheckBox();
    autoPassGen->setObjectName("passAutoGen");
    autoPassGen->setText("Create a password for me");
    autoPassGen->setChecked(bAlwaysGeneratePass);

    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("go back");
    goBack->setText("Nevermind, go back");

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(ENABLE_COMMANDS);
    });

    service->connect(service, &QLineEdit::returnPressed, this, [this, pass]() {pass->setFocus();});
    pass->connect(pass, &QLineEdit::returnPressed, this, [this, updateBtn]() {updateBtn->click();});

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

    QPushButton* passPolicyEditBtn = new QPushButton();
    passPolicyEditBtn->setObjectName("passPolicyEdit");
    passPolicyEditBtn->setText("Edit password policy");

    QPushButton* themePicker = new QPushButton();
    themePicker->setObjectName("themePicker");
    themePicker->setText("Theme picker (WIP)");

    QPushButton* clearDataBtn = new QPushButton();
    clearDataBtn->setObjectName("clearDataBtn");
    clearDataBtn->setText("DELETE EVERYTHING");

    QCheckBox* hidePasswordsBtn = new QCheckBox(); // and only allow copying them to the clipboard.
    hidePasswordsBtn->setObjectName("hidePasswords");
    hidePasswordsBtn->setText("Show passwords");
    hidePasswordsBtn->setChecked(bShowPasswords);

    QCheckBox* alwaysGenPasswordsBtn = new QCheckBox();
    alwaysGenPasswordsBtn->setObjectName("alwaysGenpasswords");
    alwaysGenPasswordsBtn->setText("Always generate passwords automatically");
    alwaysGenPasswordsBtn->setChecked(bAlwaysGeneratePass);

    QCheckBox* checkForUpdatesCheckbox = new QCheckBox();
    checkForUpdatesCheckbox->setObjectName("checkForUpdates");
    checkForUpdatesCheckbox->setText("Check for updates");
    checkForUpdatesCheckbox->setChecked(bCheckForUpdates);

    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("goBACK");
    goBack->setText("Nevermind, go back");

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

    checkForUpdatesCheckbox->connect(checkForUpdatesCheckbox, &QCheckBox::clicked, this, [this, checkForUpdatesCheckbox]() {
        bCheckForUpdates = !bCheckForUpdates;
        checkForUpdatesCheckbox->setChecked(bCheckForUpdates);
        setts.setValue("checkForUpdates", bCheckForUpdates);
    });

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen();enableDisableCommands(ENABLE_COMMANDS); });

    ui->Managment->addWidget(passPolicyEditBtn);
    ui->Managment->addWidget(themePicker);
    ui->Managment->addWidget(clearDataBtn);
    ui->Managment->addWidget(hidePasswordsBtn);
    ui->Managment->addWidget(alwaysGenPasswordsBtn);
    ui->Managment->addWidget(checkForUpdatesCheckbox);
    ui->Managment->addWidget(goBack);
}

void window::showPassPolicy() {
    window::clearScreen();

    QLabel* title = new QLabel();
    title->setFont(QFont("Arial", 16));
    title->setText("Password policy customizer\nPick the variable you wish to edit:");
    ui->Managment->addWidget(title);

    // Convert int to string and render the widgets
    std::map<std::string, std::string> passPolicyString;
    for (const auto& entry : db->passPolicy) {
        passPolicyString[entry.first] = std::to_string(entry.second);
    }
    auto buttons = window::renderWidgets(passPolicyString, "first", "first");

    for (auto it = db->passPolicy.begin(); it!=db->passPolicy.end();++it) {
        QRadioButton* btn = new QRadioButton();
        btn->setText(QString::fromStdString(it->first));
        btn->setObjectName(QString::fromStdString(it->first));

        ui->Managment->addWidget(btn);
        buttons->addButton(btn);
    }
    QPushButton* pick = new QPushButton();
    pick->setText("Select");
    pick->setObjectName("selectBtn");

    QPushButton* goBack = new QPushButton();
    goBack->setText("Nevermind, go back");
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen();settings(); } );

    pick->connect(pick, &QPushButton::clicked, this, [this, buttons = std::move(buttons)]() {
        QRadioButton* selected = qobject_cast<QRadioButton*>(buttons->checkedButton());
        if (selected) {
            std::string objName = selected->objectName().toStdString();
            clearScreen();

            QLabel* title = new QLabel();
            title->setText("Editing "+ QString::fromStdString(objName));
            title->setFont(QFont("Arial", 16));

            QLabel* desc = new QLabel();
            desc->setText("CURRENT VALUE: "+QString::number(db->passPolicy[objName]));
            desc->setFont(QFont("Arial", 14));

            QLineEdit* newValue = new QLineEdit();
            newValue->setPlaceholderText("Enter the new value here");
            newValue->setFont(QFont("Arial", 12));

            QPushButton* updateBtn = new QPushButton();
            updateBtn->setText("Update!");
            // call the update policy function as long as the newValue is a digit
            updateBtn->connect(updateBtn, &QPushButton::clicked, this, [this, objName, newValue]() { if (newValue->text().toInt()) {db->updatePassPolicy(objName, newValue->text().toInt());clearScreen();settings(); } } );

            QPushButton* goBack = new QPushButton();
            goBack->setText("Nevermind, go back");
            goBack->connect(goBack, &QPushButton::clicked, this, [this]() { showPassPolicy(); });

            ui->Managment->addWidget(title);
            ui->Managment->addWidget(desc, 5, Qt::AlignTop);
            ui->Managment->addWidget(newValue, Qt::AlignCenter);
            ui->Managment->addWidget(updateBtn, 5, Qt::AlignCenter | Qt::AlignHCenter);
            ui->Managment->addWidget(goBack, 5, Qt::AlignRight | Qt::AlignBottom);
        } else {
            clearScreen();
            enableDisableCommands(ENABLE_COMMANDS);
            handleActionComplete(false, "UPDATEPOLICY", "Please select a variable first");
        }
    });

    ui->Managment->addWidget(pick);
    ui->Managment->addWidget(goBack, 5, Qt::AlignRight | Qt::AlignBottom);
}

void window::showThemePicker() {
    // Clear the screen before showing the theme picker
    window::clearScreen();

    // Create and set up the title label
    QLabel* title = new QLabel();
    title->setFont(QFont("Arial", 16));
    title->setText("Theme picker");
    ui->Managment->addWidget(title);

    // Render all the themes
    auto themesBtns = window::renderWidgets(themes, "first", "first");

    // Set up buttons for selecting, removing, creating, and going back
    QPushButton* pick = new QPushButton();
    pick->setText("Select");
    pick->setObjectName("selectBtn");

    QPushButton* removeThemeBtn = new QPushButton();
    removeThemeBtn->setText("Delete selected theme");
    removeThemeBtn->setObjectName("deleteSelected");
    removeThemeBtn->connect(removeThemeBtn, &QPushButton::clicked, this, [this, themesBtns]() {
        QRadioButton* selected = qobject_cast<QRadioButton*>(themesBtns->checkedButton());
        if (selected) {
            // Call function to delete the selected theme
            deleteTheme(selected->text().toStdString());
        }
    });

    QPushButton* createNew = new QPushButton();
    createNew->setText("(ADVANCED) Create a new theme by a preset (USE AT YOUR OWN RISK)");
    createNew->setObjectName("selectBtn");

    QPushButton* goBack = new QPushButton();
    goBack->setText("Nevermind, go back");
    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen(); settings(); });

    // Add buttons to the layout
    ui->Managment->addWidget(pick);
    ui->Managment->addWidget(removeThemeBtn);
    ui->Managment->addWidget(createNew);
    ui->Managment->addWidget(goBack);

    // Connect signals for button actions
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

void window::saveTheme(std::string themeName, std::string themePreset) {
    if (!themeName.empty()) {
        if (themes.find(themeName) == themes.end() || themes[themePreset] != customTheme.toStdString()) {
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
    // Create a preview window for the theme properties
    preview = new themePreview();
    // Position the preview window to the top right corner of the main window
    preview->move(this->geometry().right() - 20, this->geometry().top());

    // Retrieve the name of the widget to be edited
    std::string widgetToEdit;
    QAbstractButton* s = widgets->checkedButton();
    if (s) {
        widgetToEdit = s->objectName().toStdString();
    }

    // Clear the screen before rendering new property editor elements
    window::clearScreen();

    // Render property editing options based on the widget type
    auto properties = window::renderWidgets(widgetProperties, "first", "first");

    // Create an 'Edit' button to trigger property editing
    QPushButton* edit = new QPushButton();
    edit->setText("Edit");
    edit->setObjectName("edit");
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

    // Create a 'Go Back' button to cancel property editing and return to the theme editor
    QPushButton* goBack = new QPushButton();
    goBack->setText("Nevermind, go back (save changes)");
    goBack->connect(goBack, &QPushButton::clicked, this, [this, themePreset]() {
        // Close the preview window and return to the theme editor
        preview->close();
        clearScreen();
        themeEditor(themePreset);
    });

    // Add the 'Edit' and 'Go Back' buttons to the layout
    ui->Managment->addWidget(edit);
    ui->Managment->addWidget(goBack);

    // Apply the current custom theme to the preview window
    preview->setStyleSheet(customTheme);
    // Show the preview window
    preview->show();
}

void window::themeEditor(std::string themePreset) {
    // Clear the screen and reset the flag for saved theme
    window::clearScreen();
    bSavedTheme = false;

    // Create and configure title label
    QLabel* title = new QLabel();
    title->setText("THEME EDITOR 0.1\nYou are using " + QString::fromStdString(themePreset) + " theme as a preset\nYou may update properties from the widgets below");
    title->setObjectName("title");
    title->setWordWrap(true);
    title->setFont(QFont("Arial", 9));
    ui->Managment->addWidget(title);

    // Render all the editable widgets
    auto widgets = window::renderWidgets(editableWidgets, "second", "first");

    // Create and configure theme name input field
    QLineEdit* themeName = new QLineEdit();
    themeName->setPlaceholderText("Enter your theme's name here");
    themeName->setObjectName("themeName");

    // Create and configure save button
    QPushButton* save = new QPushButton();
    save->setText("Save");
    save->setObjectName("save");
    save->connect(save, &QPushButton::clicked, this, [this, themeName, themePreset]() { saveTheme(themeName->text().toStdString(), themePreset); });

    // Create and configure edit button
    QPushButton* edit = new QPushButton();
    edit->setText("Edit");
    edit->setObjectName("edit");
    edit->connect(edit, &QPushButton::clicked, this, std::bind(&window::startPropertyEditor, this, widgets, themePreset));

    // Create and configure go back button
    QPushButton* goBack = new QPushButton();
    goBack->setText("Nevermind, go back");
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
    return (themeName == "DARK" || themeName == "LIGHT" || themeName == "DARKBLUE" || themeName == "LIGHTBLUE");
}
