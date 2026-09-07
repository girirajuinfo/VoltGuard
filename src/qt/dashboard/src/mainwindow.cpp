#include "mainwindow.h"
#include "backendprocess.h"

#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

namespace {
QString jsonValue(const QJsonObject &object, const char *key, const QString &fallback = "--")
{
    return object.contains(key) ? object.value(key).toVariant().toString() : fallback;
}

QString numberValue(const QJsonObject &object, const char *key, const QString &unit)
{
    if (!object.contains(key) || !object.value(key).isDouble()) {
        return "--";
    }
    return QString::number(object.value(key).toDouble(), 'f', 2) + unit;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), backend_(new BackendProcess(this))
{
    setWindowTitle("VoltGuard | Physics-Aware ICS/SCADA Security Console");
    resize(1400, 850);
    setMinimumSize(1100, 700);
    setupUi();
    connectSignals();
    initializeSystem();
}

QLabel *MainWindow::createValueLabel(const QString &text)
{
    auto *label = new QLabel(text, this);
    label->setObjectName("metricValue");
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QSpinBox *MainWindow::createSpinBox(int minimum, int maximum)
{
    auto *box = new QSpinBox(this);
    box->setRange(minimum, maximum);
    box->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    return box;
}

QWidget *MainWindow::createStatusItem(const QString &name, QLabel *valueLabel)
{
    auto *widget = new QWidget(this);
    auto *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 14, 0);
    auto *dot = new QLabel("●", widget);
    dot->setObjectName("statusDot");
    auto *nameLabel = new QLabel(name, widget);
    nameLabel->setObjectName("statusName");
    layout->addWidget(dot);
    layout->addWidget(nameLabel);
    layout->addWidget(valueLabel);
    return widget;
}

QWidget *MainWindow::createCard(const QString &title, QLayout *contentLayout)
{
    auto *card = new QFrame(this);
    card->setObjectName("card");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 15, 18, 18);
    layout->setSpacing(10);
    auto *heading = new QLabel(title, card);
    heading->setObjectName("cardTitle");
    layout->addWidget(heading);
    layout->addLayout(contentLayout);
    return card;
}

void MainWindow::setupUi()
{
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget(scrollArea);
    auto *mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(26, 22, 26, 26);
    mainLayout->setSpacing(16);

    auto *header = new QFrame(content);
    header->setObjectName("header");
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(22, 18, 22, 18);
    auto *titleLayout = new QVBoxLayout;
    auto *title = new QLabel("VOLTGUARD", header);
    title->setObjectName("appTitle");
    auto *subtitle = new QLabel("Physics-Aware ICS/SCADA Security System", header);
    subtitle->setObjectName("appSubtitle");
    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    systemStatus_ = new QLabel("INITIALIZING", header);
    systemStatus_->setObjectName("systemBadge");
    headerLayout->addWidget(systemStatus_);
    mainLayout->addWidget(header);

    auto *statusBar = new QFrame(content);
    statusBar->setObjectName("statusBar");
    auto *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(16, 9, 16, 9);
    parserStatus_ = new QLabel("STARTING", statusBar);
    physicsStatus_ = new QLabel("STARTING", statusBar);
    decisionEngineStatus_ = new QLabel("STARTING", statusBar);
    statusLayout->addWidget(createStatusItem("MODBUS PARSER", parserStatus_));
    statusLayout->addWidget(createStatusItem("PHYSICS ENGINE", physicsStatus_));
    statusLayout->addWidget(createStatusItem("DECISION ENGINE", decisionEngineStatus_));
    statusLayout->addStretch();
    mainLayout->addWidget(statusBar);

    auto *topGrid = new QGridLayout;
    topGrid->setSpacing(16);

    auto *commandForm = new QFormLayout;
    commandForm->setLabelAlignment(Qt::AlignLeft);
    transactionIdBox_ = createSpinBox(0, 65535);
    protocolIdBox_ = createSpinBox(0, 65535);
    unitIdBox_ = createSpinBox(0, 255);
    addressBox_ = createSpinBox(0, 65535);
    quantityBox_ = createSpinBox(0, 65535);
    valueBox_ = createSpinBox(0, 65535);
    transactionIdBox_->setValue(1);
    unitIdBox_->setValue(1);
    quantityBox_->setValue(2);
    functionCodeBox_ = new QComboBox(this);
    functionCodeBox_->addItem("03  |  Read Holding Registers", 3);
    functionCodeBox_->addItem("04  |  Read Input Registers", 4);
    functionCodeBox_->addItem("06  |  Write Single Register", 6);
    functionCodeBox_->addItem("10  |  Write Multiple Registers", 16);
    commandForm->addRow("Transaction ID", transactionIdBox_);
    commandForm->addRow("Protocol ID", protocolIdBox_);
    commandForm->addRow("Unit ID", unitIdBox_);
    commandForm->addRow("Function Code", functionCodeBox_);
    commandForm->addRow("Address", addressBox_);
    commandForm->addRow("Quantity", quantityBox_);
    commandForm->addRow("Value", valueBox_);
    auto *commandButtons = new QHBoxLayout;
    analyzeButton_ = new QPushButton("ANALYZE COMMAND", this);
    analyzeButton_->setObjectName("primaryButton");
    auto *safeButton = new QPushButton("LOAD SAFE TEST", this);
    safeButton->setObjectName("secondaryButton");
    safeButton->setProperty("test", "safe");
    auto *unsafeButton = new QPushButton("LOAD UNSAFE TEST", this);
    unsafeButton->setObjectName("warningButton");
    unsafeButton->setProperty("test", "unsafe");
    auto *clearButton = new QPushButton("CLEAR", this);
    clearButton->setObjectName("secondaryButton");
    clearButton->setProperty("test", "clear");
    commandButtons->addWidget(safeButton);
    commandButtons->addWidget(unsafeButton);
    commandButtons->addWidget(clearButton);
    commandButtons->addWidget(analyzeButton_);
    commandButtons->addStretch();
    auto *commandLayout = new QVBoxLayout;
    commandLayout->addLayout(commandForm);
    commandLayout->addSpacing(4);
    commandLayout->addLayout(commandButtons);
    auto *commandCard = createCard("MODBUS COMMAND", commandLayout);
    topGrid->addWidget(commandCard, 0, 0, 2, 1);

    auto *decisionLayout = new QVBoxLayout;
    decisionLabel_ = new QLabel("WAITING", this);
    decisionLabel_->setObjectName("decisionValue");
    severityLabel_ = createValueLabel("--");
    reasonLabel_ = new QLabel("No command analyzed", this);
    reasonLabel_->setObjectName("reasonValue");
    reasonLabel_->setWordWrap(true);
    decisionLayout->addWidget(decisionLabel_);
    decisionLayout->addWidget(severityLabel_);
    decisionLayout->addWidget(reasonLabel_);
    auto *decisionCard = createCard("SECURITY DECISION", decisionLayout);
    decisionCard->setObjectName("decisionCard");
    topGrid->addWidget(decisionCard, 0, 1);

    auto *physicsGrid = new QGridLayout;
    physicsGrid->setHorizontalSpacing(22);
    physicsGrid->setVerticalSpacing(8);
    physicsStatusValue_ = createValueLabel("--");
    flowLabel_ = createValueLabel("--");
    pressureLabel_ = createValueLabel("--");
    pumpSpeedLabel_ = createValueLabel("Not reported");
    valvePositionLabel_ = createValueLabel("Not reported");
    const QStringList labels = {"STATUS", "PREDICTED FLOW", "PREDICTED PRESSURE", "PUMP SPEED", "VALVE POSITION"};
    const QList<QLabel *> values = {physicsStatusValue_, flowLabel_, pressureLabel_, pumpSpeedLabel_, valvePositionLabel_};
    for (int index = 0; index < labels.size(); ++index) {
        auto *name = new QLabel(labels.at(index), this);
        name->setObjectName("metricName");
        physicsGrid->addWidget(name, 0, index);
        physicsGrid->addWidget(values.at(index), 1, index);
    }
    topGrid->addWidget(createCard("PHYSICS ANALYSIS", physicsGrid), 1, 1);
    topGrid->setColumnStretch(0, 1);
    topGrid->setColumnStretch(1, 2);
    mainLayout->addLayout(topGrid);

    auto *resultGrid = new QGridLayout;
    resultGrid->setSpacing(16);
    violationsView_ = new QPlainTextEdit(this);
    violationsView_->setReadOnly(true);
    violationsView_->setObjectName("violationsView");
    violationCountLabel_ = createValueLabel("0 active");
    auto *violationsLayout = new QVBoxLayout;
    violationsLayout->addWidget(violationCountLabel_);
    violationsLayout->addWidget(violationsView_);
    resultGrid->addWidget(createCard("PHYSICS VIOLATIONS", violationsLayout), 0, 0);

    parsedCommandView_ = new QLabel("No parsed command returned", this);
    parsedCommandView_->setObjectName("parsedCommand");
    parsedCommandView_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto *parsedLayout = new QVBoxLayout;
    parsedLayout->addWidget(parsedCommandView_);
    resultGrid->addWidget(createCard("PARSED MODBUS COMMAND", parsedLayout), 0, 1);
    resultGrid->setColumnStretch(0, 1);
    resultGrid->setColumnStretch(1, 1);
    mainLayout->addLayout(resultGrid);

    auto *statsLayout = new QHBoxLayout;
    totalAnalyzedLabel_ = createValueLabel("0");
    allowedLabel_ = createValueLabel("0");
    blockedLabel_ = createValueLabel("0");
    highSeverityLabel_ = createValueLabel("0");
    const QList<QPair<QString, QLabel *>> stats = {
        {"TOTAL ANALYZED", totalAnalyzedLabel_}, {"ALLOWED", allowedLabel_},
        {"BLOCKED", blockedLabel_}, {"HIGH SEVERITY", highSeverityLabel_}
    };
    for (const auto &stat : stats) {
        auto *statBox = new QVBoxLayout;
        auto *name = new QLabel(stat.first, this);
        name->setObjectName("metricName");
        statBox->addWidget(name);
        statBox->addWidget(stat.second);
        statsLayout->addLayout(statBox);
        statsLayout->addStretch();
    }
    mainLayout->addWidget(createCard("SESSION STATISTICS", statsLayout));

    historyTable_ = new QTableWidget(0, 7, this);
    historyTable_->setHorizontalHeaderLabels({"TIME", "FUNCTION", "ADDRESS", "VALUE", "PHYSICS", "DECISION", "SEVERITY"});
    historyTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable_->verticalHeader()->setVisible(false);
    historyTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable_->setMinimumHeight(150);
    auto *historyLayout = new QVBoxLayout;
    historyLayout->addWidget(historyTable_);
    mainLayout->addWidget(createCard("COMMAND HISTORY  /  LAST 100 ANALYSES", historyLayout));

    eventLog_ = new QPlainTextEdit(this);
    eventLog_->setReadOnly(true);
    eventLog_->setMaximumBlockCount(300);
    eventLog_->setMinimumHeight(140);
    auto *logLayout = new QVBoxLayout;
    logLayout->addWidget(eventLog_);
    mainLayout->addWidget(createCard("EVENT LOG", logLayout));

    content->setLayout(mainLayout);
    scrollArea->setWidget(content);
    setCentralWidget(scrollArea);
}

void MainWindow::connectSignals()
{
    connect(analyzeButton_, &QPushButton::clicked, this, &MainWindow::analyzeCommand);
}

void MainWindow::initializeSystem()
{
    auto *commandCard = findChild<QFrame *>("card");
    Q_UNUSED(commandCard);
    auto buttons = findChildren<QPushButton *>();
    for (auto *button : buttons) {
        if (button->property("test").toString() == "safe") {
            connect(button, &QPushButton::clicked, this, &MainWindow::loadSafeTest);
        } else if (button->property("test").toString() == "unsafe") {
            connect(button, &QPushButton::clicked, this, &MainWindow::loadUnsafeTest);
        } else if (button->property("test").toString() == "clear") {
            connect(button, &QPushButton::clicked, this, &MainWindow::clearCurrentResult);
        }
    }
    connect(backend_, &BackendProcess::backendStarted, this, &MainWindow::handleBackendStarted);
    connect(backend_, &BackendProcess::backendCompleted, this, &MainWindow::handleBackendCompleted);
    connect(backend_, &BackendProcess::backendError, this, &MainWindow::handleBackendError);
    connect(backend_, &BackendProcess::backendLog, this, &MainWindow::appendBackendLog);

    logEvent("SYSTEM", "VoltGuard initialized");
    logEvent("PARSER", "Modbus/TCP parser ready");
    logEvent("PHYSICS", "Physics engine ready");
    logEvent("DECISION", "Decision engine ready");
    logEvent("SYSTEM", "Dashboard waiting for command...");
}

void MainWindow::analyzeCommand()
{
    const dashboard::CommandValues command = readCommand();
    logEvent("BACKEND", "Sending command to Python pipeline...");
    backend_->evaluateCommand(command.toJson());
}

void MainWindow::loadSafeTest()
{
    transactionIdBox_->setValue(1);
    protocolIdBox_->setValue(0);
    unitIdBox_->setValue(1);
    functionCodeBox_->setCurrentIndex(0);
    addressBox_->setValue(0);
    quantityBox_->setValue(2);
    valueBox_->setValue(0);
    logEvent("SYSTEM", "Loaded safe test command; press ANALYZE COMMAND to execute.");
}

void MainWindow::loadUnsafeTest()
{
    transactionIdBox_->setValue(3);
    protocolIdBox_->setValue(0);
    unitIdBox_->setValue(1);
    functionCodeBox_->setCurrentIndex(2);
    addressBox_->setValue(32);
    quantityBox_->setValue(0);
    valueBox_->setValue(3500);
    logEvent("SYSTEM", "Loaded unsafe test command; press ANALYZE COMMAND to execute.");
}

void MainWindow::clearCurrentResult()
{
    transactionIdBox_->setValue(1);
    protocolIdBox_->setValue(0);
    unitIdBox_->setValue(1);
    functionCodeBox_->setCurrentIndex(0);
    addressBox_->setValue(0);
    quantityBox_->setValue(2);
    valueBox_->setValue(0);
    resetResultPanels();
    logEvent("SYSTEM", "Current command and result cleared; history retained.");
}

void MainWindow::handleBackendStarted()
{
    analyzeButton_->setEnabled(false);
    analyzeButton_->setText("ANALYZING...");
    setStatus(systemStatus_, "PROCESSING", "warning");
}

void MainWindow::handleBackendCompleted(const QJsonObject &result)
{
    analyzeButton_->setEnabled(true);
    analyzeButton_->setText("ANALYZE COMMAND");
    setStatus(systemStatus_, "SYSTEM ONLINE", "safe");
    renderResult(result);
    addHistoryEntry(result);
    const QJsonObject decision = result.value("decision").toObject();
    updateStatistics(decision.value("action").toString(), decision.value("severity").toString());
    const QJsonObject physics = result.value("physics").toObject();
    logEvent("PHYSICS", "Status: " + physics.value("status").toString("REJECTED"));
    if (!physics.value("violations").toArray().isEmpty()) {
        logEvent("ALERT", "Physics constraint violation");
    }
    logEvent("DECISION", decision.value("action").toString());
}

void MainWindow::handleBackendError(const QString &error)
{
    analyzeButton_->setEnabled(true);
    analyzeButton_->setText("ANALYZE COMMAND");
    setStatus(systemStatus_, "SYSTEM ERROR", "danger");
    logEvent("ERROR", error);
    QMessageBox::warning(this, "Backend Error", error);
}

void MainWindow::appendBackendLog(const QString &message)
{
    logEvent("BACKEND", message);
}

void MainWindow::setStatus(QLabel *label, const QString &text, const QString &state)
{
    label->setText(text);
    label->setProperty("state", state);
    label->style()->unpolish(label);
    label->style()->polish(label);
}

void MainWindow::setDecisionState(const QString &action, const QString &severity)
{
    decisionLabel_->setText(action.isEmpty() ? "ERROR / REJECTED" : action);
    decisionLabel_->setProperty("state", action == "ALLOW" ? "safe" : "danger");
    severityLabel_->setText("SEVERITY  /  " + (severity.isEmpty() ? "UNKNOWN" : severity));
    severityLabel_->setProperty("state", severity == "NORMAL" ? "safe" : "danger");
    decisionLabel_->style()->unpolish(decisionLabel_);
    decisionLabel_->style()->polish(decisionLabel_);
    severityLabel_->style()->unpolish(severityLabel_);
    severityLabel_->style()->polish(severityLabel_);
}

void MainWindow::renderResult(const QJsonObject &result)
{
    const QJsonObject decision = result.value("decision").toObject();
    const QJsonObject physics = result.value("physics").toObject();
    setDecisionState(decision.value("action").toString("DROP"), decision.value("severity").toString("HIGH"));
    reasonLabel_->setText(decision.value("reason").toString("Backend rejected the command."));
    renderPhysics(physics);
    renderViolations(physics.value("violations").toArray());
    if (result.contains("parsed_command") && result.value("parsed_command").isObject()) {
        renderParsedCommand(result.value("parsed_command").toObject());
    } else {
        parsedCommandView_->setText("No parsed command returned\n\nBackend status: " + result.value("status").toString("REJECTED"));
    }
}

void MainWindow::renderParsedCommand(const QJsonObject &command)
{
    parsedCommandView_->setText(QString("Transaction ID   %1\nProtocol ID      %2\nUnit ID           %3\nFunction Code    %4\nAddress          %5\nQuantity         %6\nValue            %7")
        .arg(jsonValue(command, "transaction_id"), jsonValue(command, "protocol_id"),
             jsonValue(command, "unit_id"), jsonValue(command, "function_code"),
             jsonValue(command, "address"), jsonValue(command, "quantity"), jsonValue(command, "value")));
}

void MainWindow::renderPhysics(const QJsonObject &physics)
{
    const QString status = physics.value("status").toString("NOT RETURNED");
    physicsStatusValue_->setText(status);
    physicsStatusValue_->setProperty("state", status == "SAFE" ? "safe" : "danger");
    flowLabel_->setText(numberValue(physics, "predicted_flow", " L/min"));
    pressureLabel_->setText(numberValue(physics, "predicted_pressure", " bar"));
    physicsStatusValue_->style()->unpolish(physicsStatusValue_);
    physicsStatusValue_->style()->polish(physicsStatusValue_);
}

void MainWindow::renderViolations(const QJsonArray &violations)
{
    violationCountLabel_->setText(QString::number(violations.size()) + " active");
    violationsView_->clear();
    if (violations.isEmpty()) {
        violationsView_->setPlainText("No physics violations detected.");
        violationCountLabel_->setProperty("state", "safe");
    } else {
        QStringList lines;
        for (const auto &violation : violations) {
            if (violation.isString()) {
                lines << "!  " + violation.toString();
            }
        }
        violationsView_->setPlainText(lines.join("\n\n"));
        violationCountLabel_->setProperty("state", "danger");
    }
    violationCountLabel_->style()->unpolish(violationCountLabel_);
    violationCountLabel_->style()->polish(violationCountLabel_);
}

void MainWindow::updateStatistics(const QString &action, const QString &severity)
{
    ++totalAnalyzed_;
    if (action == "ALLOW") {
        ++allowed_;
    } else {
        ++blocked_;
    }
    if (severity == "HIGH") {
        ++highSeverity_;
    }
    totalAnalyzedLabel_->setText(QString::number(totalAnalyzed_));
    allowedLabel_->setText(QString::number(allowed_));
    blockedLabel_->setText(QString::number(blocked_));
    highSeverityLabel_->setText(QString::number(highSeverity_));
}

void MainWindow::addHistoryEntry(const QJsonObject &result)
{
    const QJsonObject command = result.value("parsed_command").toObject();
    const QJsonObject physics = result.value("physics").toObject();
    const QJsonObject decision = result.value("decision").toObject();
    const int row = historyTable_->rowCount();
    historyTable_->insertRow(row);
    const QStringList values = {
        QDateTime::currentDateTime().toString("HH:mm:ss"),
        QString("%1").arg(command.value("function_code").toInt(), 2, 16, QLatin1Char('0')).toUpper(),
        jsonValue(command, "address"), jsonValue(command, "value"),
        physics.value("status").toString("REJECTED"), decision.value("action").toString("DROP"),
        decision.value("severity").toString("HIGH")
    };
    for (int column = 0; column < values.size(); ++column) {
        auto *item = new QTableWidgetItem(values.at(column));
        item->setTextAlignment(Qt::AlignCenter);
        historyTable_->setItem(row, column, item);
    }
    while (historyTable_->rowCount() > 100) {
        historyTable_->removeRow(0);
    }
    historyTable_->scrollToBottom();
}

void MainWindow::logEvent(const QString &category, const QString &message)
{
    eventLog_->appendPlainText(QString("[%1] [%2] %3")
        .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), category, message));
}

void MainWindow::resetResultPanels()
{
    setDecisionState("WAITING", "UNKNOWN");
    reasonLabel_->setText("No command analyzed");
    physicsStatusValue_->setText("--");
    flowLabel_->setText("--");
    pressureLabel_->setText("--");
    pumpSpeedLabel_->setText("Not reported");
    valvePositionLabel_->setText("Not reported");
    violationCountLabel_->setText("0 active");
    violationsView_->setPlainText("No physics violations detected.");
    parsedCommandView_->setText("No parsed command returned");
}

dashboard::CommandValues MainWindow::readCommand() const
{
    dashboard::CommandValues command;
    command.transactionId = transactionIdBox_->value();
    command.protocolId = protocolIdBox_->value();
    command.unitId = unitIdBox_->value();
    command.functionCode = functionCodeBox_->currentData().toInt();
    command.address = addressBox_->value();
    command.quantity = quantityBox_->value();
    command.value = valueBox_->value();
    return command;
}
