#include "./window.h"

window::window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , setts("globbertot, simplePassMan")
{
    ui->setupUi(this);
    db = new dbManager();
    netManager = new QNetworkAccessManager;
    QObject::connect(netManager, &QNetworkAccessManager::finished, this, [this](QNetworkReply* reply) {
        if (reply->error()) {
            qDebug() << reply->errorString();return;
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
    });
    req.setUrl(QString::fromStdString(updatesURL));
    netManager->get(req);

    showPasswords = setts.value("showPasswords", false).toBool();
    alwaysGeneratePass = setts.value("alwaysGeneratePasswords", true).toBool();

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

window::~window() { delete ui; }

void window::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && ui->TOPBAR->geometry().contains(e->pos())) {
        isMousePressed = true;dragStart = e->pos();
    }
}

void window::mouseMoveEvent(QMouseEvent* e) {
    if (isMousePressed) {
        move(pos() + e->pos() - dragStart);
    }
}

void window::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) { isMousePressed = false; }
}

QList<QPushButton*> window::grabButtons() {
    QList<QPushButton*> buttonList;
    QFormLayout* commandsList = ui->COMMANDS;

    for (int i = 0; i < commandsList->count(); ++i) {
        QLayoutItem* item = commandsList->itemAt(i);
        if (item) {
            QPushButton* button = qobject_cast<QPushButton*>(item->widget());

            if (button) {
                buttonList.append(button);
            }
        }
    }

    return buttonList;
}

void window::enableDisableCommands(int mode) {
    // 0 for disable, 1 to enable, anything else disables
    QList<QPushButton*> commands = window::grabButtons();
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

void window::handleActionComplete(bool success, std::string action, std::string errMsg, QList<std::string> results) {
    window::enableDisableCommands(1);
    window::clearScreen();

    // Title Label
    QLabel* titleLabel = new QLabel();
    titleLabel->setText("RESULTS: ");
    titleLabel->setFont(QFont("Arial", 15, QFont::Thin));
    titleLabel->setObjectName("titleLabel");

    ui->Results->addWidget(titleLabel, 0, Qt::AlignHCenter | Qt::AlignTop);

    if (action == "showAll") {
        // Make the scroll work
        QScrollArea* scrollArea = new QScrollArea();
        QWidget* scrollContent = new QWidget();
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

    if (action == "SEARCH") {
        // Set text accordingly:
        // if it succeeded,
            // check if showPass is true
                // if yes, display the pass
                // if not, display REDACTED
        // if it didnt succeed, log the err msg
        detail->setText((success) ? ((showPasswords) ? QString::fromStdString(results.first()) : "REDACTED") : QString::fromStdString(errMsg));
        QPushButton* copyToClipboardBtn = new QPushButton();
        copyToClipboardBtn->setText("Copy to clipboard");
        copyToClipboardBtn->setFixedSize(150, 30);
        copyToClipboardBtn->setFont(QFont("Arial", 13));

        copyToClipboardBtn->connect(copyToClipboardBtn, &QPushButton::clicked, this, [this, results] { QApplication::clipboard()->setText(QString::fromUtf8(results.first())); });
        ui->Results->addWidget(copyToClipboardBtn, 0, Qt::AlignTop | Qt::AlignRight);
    }

    ui->Results->addWidget(detail, 0, Qt::AlignHCenter | Qt::AlignTop);
}

void window::create() {
    window::enableDisableCommands(0);

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
    autoPassGen->setChecked(alwaysGeneratePass);

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
        enableDisableCommands(1);
    });

    // Add them to the ui
    ui->Managment->addWidget(service);
    ui->Managment->addWidget(password);
    ui->Managment->addWidget(autoPassGen);
    ui->Managment->addWidget(createBtn);
    ui->Managment->addWidget(goBack);
}

void window::_delete() {
    window::enableDisableCommands(0);

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
        enableDisableCommands(1);
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
    window::enableDisableCommands(0);

    // Service textline
    QLineEdit* service = new QLineEdit();
    service->setPlaceholderText("Enter the service of the password you want to grab");
    service->setObjectName("serviceLine");

    // Search button
    QPushButton* searchBtn = new QPushButton();
    searchBtn->setObjectName("searchBtn");
    searchBtn->setText("Read");

    // Return button
    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("go back");
    goBack->setText("Nevermind, go back");

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(1);
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
    window::enableDisableCommands(0);

    QLineEdit* service = new QLineEdit();
    service->setPlaceholderText("Enter the service of the password you want to change");
    service->setObjectName("serviceLine");

    QLineEdit* pass = new QLineEdit();
    pass->setPlaceholderText("Enter the updated password");
    pass->setObjectName("passLine");

    QPushButton* updateBtn = new QPushButton();
    updateBtn->setText("Update");
    updateBtn->setObjectName("updateBtn");

    // Password create automatically checkbox
    QCheckBox* autoPassGen = new QCheckBox();
    autoPassGen->setObjectName("passAutoGen");
    autoPassGen->setText("Create a password for me");
    autoPassGen->setChecked(alwaysGeneratePass);

    // Return button
    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("go back");
    goBack->setText("Nevermind, go back");

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() {
        clearScreen();
        enableDisableCommands(1);
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

void window::showPassPolicy() {
    window::clearScreen();

    QLabel* title = new QLabel();
    title->setFont(QFont("Arial", 16));
    title->setText("Password policy customizer\nPick the variable you wish to edit:");
    ui->Managment->addWidget(title);

    std::unique_ptr<QButtonGroup> buttons = std::make_unique<QButtonGroup>();

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
            newValue->setPlaceholderText("Enter the new value here (set it to -1 to disable this variable)");
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
            enableDisableCommands(1);
            handleActionComplete(false, "UPDATEPOLICY", "Please select a variable first");
        }
    });

    ui->Managment->addWidget(pick);
    ui->Managment->addWidget(goBack, 5, Qt::AlignRight | Qt::AlignBottom);
}

void window::settings() {
    window::enableDisableCommands(0);

    QPushButton* passPolicyEditBtn = new QPushButton();
    passPolicyEditBtn->setObjectName("passPolicyEdit");
    passPolicyEditBtn->setText("Edit password policy");

    QPushButton* clearDataBtn = new QPushButton();
    clearDataBtn->setObjectName("clearDataBtn");
    clearDataBtn->setText("DELETE EVERYTHING");

    QCheckBox* hidePasswordsBtn = new QCheckBox(); // and only allow copying them to the clipboard.
    hidePasswordsBtn->setObjectName("hidePasswords");
    hidePasswordsBtn->setText("Show passwords");
    hidePasswordsBtn->setChecked(showPasswords);

    QCheckBox* alwaysGenPasswordsBtn = new QCheckBox();
    alwaysGenPasswordsBtn->setObjectName("alwaysGenpasswords");
    alwaysGenPasswordsBtn->setText("Always generate passwords automatically");
    alwaysGenPasswordsBtn->setChecked(alwaysGeneratePass);

    QPushButton* goBack = new QPushButton();
    goBack->setObjectName("goBACK");
    goBack->setText("Nevermind, go back");

    passPolicyEditBtn->connect(passPolicyEditBtn, &QPushButton::clicked, this, [this]() {showPassPolicy();} );
    clearDataBtn->connect(clearDataBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton rep;
        rep = QMessageBox::question(this, "WARNING", "Are you sure you want to delete everything, all your data will be gone.", QMessageBox::Yes | QMessageBox::No);
        if (rep == QMessageBox::Yes) {
            // Procceed deletion
            db->resetSettings();
            setts.setValue("showPasswords", false);
            setts.setValue("alwaysGeneratePasswords", true);

            QSqlDatabase::database("main").close();
            QFile dbToDelete("simplePassMan.db");
            if (!dbToDelete.remove()) {
                window::handleActionComplete(false, "KILLSWITCH", dbToDelete.errorString().toStdString());
            }
            window::handleActionComplete(true, "KILLSWITCH");
        }
    });

    hidePasswordsBtn->connect(hidePasswordsBtn, &QCheckBox::clicked, this, [this, hidePasswordsBtn]() {
        showPasswords = !showPasswords;
        hidePasswordsBtn->setChecked(showPasswords);
        setts.setValue("showPasswords", showPasswords);
    } );

    alwaysGenPasswordsBtn->connect(alwaysGenPasswordsBtn, &QCheckBox::clicked, this, [this, alwaysGenPasswordsBtn]() {
        alwaysGeneratePass = !alwaysGeneratePass;
        alwaysGenPasswordsBtn->setChecked(alwaysGeneratePass);
        setts.setValue("alwaysGeneratePasswords", alwaysGeneratePass);
    });

    goBack->connect(goBack, &QPushButton::clicked, this, [this]() { clearScreen();enableDisableCommands(1); });

    ui->Managment->addWidget(passPolicyEditBtn);
    ui->Managment->addWidget(clearDataBtn);
    ui->Managment->addWidget(hidePasswordsBtn);
    ui->Managment->addWidget(alwaysGenPasswordsBtn);
    ui->Managment->addWidget(goBack);
}
