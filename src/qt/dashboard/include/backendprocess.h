#ifndef VOLTGUARD_BACKEND_PROCESS_H
#define VOLTGUARD_BACKEND_PROCESS_H

#include <QObject>
#include <QJsonObject>
#include <QProcess>
#include <QTimer>

class BackendProcess : public QObject
{
    Q_OBJECT

public:
    explicit BackendProcess(QObject *parent = nullptr);

    void evaluateCommand(const QJsonObject &command);
    static QString findProjectRoot(const QString &startingDirectory);

signals:
    void backendStarted();
    void backendCompleted(const QJsonObject &result);
    void backendError(const QString &error);
    void backendLog(const QString &message);

private slots:
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleError(QProcess::ProcessError error);
    void handleStandardError();
    void handleTimeout();

private:
    void resetProcess();
    QString validateResult(const QJsonObject &result) const;

    QProcess process_;
    QTimer timeoutTimer_;
    QByteArray standardError_;
    bool running_ = false;
};

#endif
