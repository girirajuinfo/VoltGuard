#include <QApplication>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

static QLabel* createValueLabel(const QString& value)
{
    auto* label = new QLabel(value);
    QFont font;
    font.setPointSize(14);
    font.setBold(true);
    label->setFont(font);
    return label;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("VoltGuard - ICS Security Dashboard");
    window.resize(1100, 700);

    auto* central = new QWidget;
    auto* mainLayout = new QVBoxLayout(central);

    // --------------------------------------------------
    // Header
    // --------------------------------------------------

    auto* title = new QLabel("VoltGuard");
    QFont titleFont;
    titleFont.setPointSize(26);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* subtitle = new QLabel(
        "Physics-Aware ICS / SCADA Security Monitoring"
    );

    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    // --------------------------------------------------
    // Status panels
    // --------------------------------------------------

    auto* statusLayout = new QGridLayout;

    auto* parserBox = new QGroupBox("Modbus Parser");
    auto* parserLayout = new QVBoxLayout(parserBox);
    parserLayout->addWidget(createValueLabel("READY"));
    parserLayout->addWidget(
        new QLabel("C++ packet parser initialized")
    );

    auto* physicsBox = new QGroupBox("Physics Engine");
    auto* physicsLayout = new QVBoxLayout(physicsBox);
    physicsLayout->addWidget(createValueLabel("READY"));
    physicsLayout->addWidget(
        new QLabel("Physics constraints loaded")
    );

    auto* decisionBox = new QGroupBox("Security Decision");
    auto* decisionLayout = new QVBoxLayout(decisionBox);
    decisionLayout->addWidget(createValueLabel("WAITING"));
    decisionLayout->addWidget(
        new QLabel("Awaiting Modbus command")
    );

    statusLayout->addWidget(parserBox, 0, 0);
    statusLayout->addWidget(physicsBox, 0, 1);
    statusLayout->addWidget(decisionBox, 0, 2);

    mainLayout->addLayout(statusLayout);

    // --------------------------------------------------
    // Command details
    // --------------------------------------------------

    auto* commandBox = new QGroupBox("Latest Modbus Command");
    auto* commandLayout = new QGridLayout(commandBox);

    commandLayout->addWidget(new QLabel("Transaction ID:"), 0, 0);
    commandLayout->addWidget(createValueLabel("-"), 0, 1);

    commandLayout->addWidget(new QLabel("Unit ID:"), 0, 2);
    commandLayout->addWidget(createValueLabel("-"), 0, 3);

    commandLayout->addWidget(new QLabel("Function Code:"), 1, 0);
    commandLayout->addWidget(createValueLabel("-"), 1, 1);

    commandLayout->addWidget(new QLabel("Address:"), 1, 2);
    commandLayout->addWidget(createValueLabel("-"), 1, 3);

    commandLayout->addWidget(new QLabel("Quantity:"), 2, 0);
    commandLayout->addWidget(createValueLabel("-"), 2, 1);

    commandLayout->addWidget(new QLabel("Value:"), 2, 2);
    commandLayout->addWidget(createValueLabel("-"), 2, 3);

    mainLayout->addWidget(commandBox);

    // --------------------------------------------------
    // Event log
    // --------------------------------------------------

    auto* logBox = new QGroupBox("Security Event Log");
    auto* logLayout = new QVBoxLayout(logBox);

    auto* log = new QPlainTextEdit;
    log->setReadOnly(true);

    log->appendPlainText(
        "[SYSTEM] VoltGuard dashboard initialized"
    );
    log->appendPlainText(
        "[PARSER] Modbus/TCP parser ready"
    );
    log->appendPlainText(
        "[PHYSICS] Physics engine ready"
    );
    log->appendPlainText(
        "[DECISION] Waiting for command"
    );

    logLayout->addWidget(log);
    mainLayout->addWidget(logBox);

    window.setCentralWidget(central);
    window.show();

    return app.exec();
}