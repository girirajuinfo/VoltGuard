#ifndef VOLTGUARD_MAINWINDOW_H
#define VOLTGUARD_MAINWINDOW_H

#include "dashboardtypes.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QTableWidget>

class QComboBox;
class QPushButton;
class BackendProcess;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void analyzeCommand();
    void loadSafeTest();
    void loadUnsafeTest();
    void clearCurrentResult();
    void handleBackendStarted();
    void handleBackendCompleted(const QJsonObject &result);
    void handleBackendError(const QString &error);
    void appendBackendLog(const QString &message);

private:
    void setupUi();
    void connectSignals();
    void initializeSystem();
    QWidget *createCard(const QString &title, QLayout *contentLayout);
    QWidget *createStatusItem(const QString &name, QLabel *valueLabel);
    QLabel *createValueLabel(const QString &text = "--");
    QSpinBox *createSpinBox(int minimum, int maximum);
    void setStatus(QLabel *label, const QString &text, const QString &state);
    void setDecisionState(const QString &action, const QString &severity);
    void renderResult(const QJsonObject &result);
    void renderParsedCommand(const QJsonObject &command);
    void renderPhysics(const QJsonObject &physics);
    void renderViolations(const QJsonArray &violations);
    void updateStatistics(const QString &action, const QString &severity);
    void addHistoryEntry(const QJsonObject &result);
    void logEvent(const QString &category, const QString &message);
    void resetResultPanels();
    dashboard::CommandValues readCommand() const;

    BackendProcess *backend_ = nullptr;
    QSpinBox *transactionIdBox_ = nullptr;
    QSpinBox *protocolIdBox_ = nullptr;
    QSpinBox *unitIdBox_ = nullptr;
    QComboBox *functionCodeBox_ = nullptr;
    QSpinBox *addressBox_ = nullptr;
    QSpinBox *quantityBox_ = nullptr;
    QSpinBox *valueBox_ = nullptr;
    QPushButton *analyzeButton_ = nullptr;
    QLabel *systemStatus_ = nullptr;
    QLabel *parserStatus_ = nullptr;
    QLabel *physicsStatus_ = nullptr;
    QLabel *decisionEngineStatus_ = nullptr;
    QLabel *decisionLabel_ = nullptr;
    QLabel *severityLabel_ = nullptr;
    QLabel *reasonLabel_ = nullptr;
    QLabel *physicsStatusValue_ = nullptr;
    QLabel *flowLabel_ = nullptr;
    QLabel *pressureLabel_ = nullptr;
    QLabel *pumpSpeedLabel_ = nullptr;
    QLabel *valvePositionLabel_ = nullptr;
    QLabel *violationCountLabel_ = nullptr;
    QPlainTextEdit *violationsView_ = nullptr;
    QLabel *parsedCommandView_ = nullptr;
    QTableWidget *historyTable_ = nullptr;
    QPlainTextEdit *eventLog_ = nullptr;
    QLabel *totalAnalyzedLabel_ = nullptr;
    QLabel *allowedLabel_ = nullptr;
    QLabel *blockedLabel_ = nullptr;
    QLabel *highSeverityLabel_ = nullptr;
    int totalAnalyzed_ = 0;
    int allowed_ = 0;
    int blocked_ = 0;
    int highSeverity_ = 0;
};

#endif
