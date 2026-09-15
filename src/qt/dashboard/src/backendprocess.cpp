#include "backendprocess.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace {
constexpr int BackendTimeoutMs = 15000;
}

BackendProcess::BackendProcess(QObject *parent)
    : QObject(parent)
{
    timeoutTimer_.setSingleShot(true);
    timeoutTimer_.setInterval(BackendTimeoutMs);

    connect(&process_, &QProcess::finished, this, &BackendProcess::handleFinished);
    connect(&process_, &QProcess::errorOccurred, this, &BackendProcess::handleError);
    connect(&process_, &QProcess::readyReadStandardError, this, &BackendProcess::handleStandardError);
    connect(&timeoutTimer_, &QTimer::timeout, this, &BackendProcess::handleTimeout);
}

QString BackendProcess::findProjectRoot(const QString &startingDirectory)
{
    QDir candidate(startingDirectory);
    while (!candidate.isRoot()) {
        const bool hasSource = QFileInfo(candidate.filePath("src")).isDir();
        const bool hasConfig = QFileInfo(candidate.filePath("configs")).isDir();
        const bool hasTraffic = QFileInfo(candidate.filePath("traffic")).isDir();
        if (hasSource && hasConfig && hasTraffic) {
            return candidate.absolutePath();
        }
        if (!candidate.cdUp()) {
            break;
        }
    }
    return {};
}

void BackendProcess::evaluateCommand(const QJsonObject &command)
{
    if (running_) {
        emit backendError("A backend analysis is already in progress.");
        return;
    }

    const QString root = findProjectRoot(QCoreApplication::applicationDirPath());
    if (root.isEmpty()) {
        emit backendError("VoltGuard project root was not found from the dashboard executable.");
        return;
    }

    resetProcess();
    process_.setWorkingDirectory(root);
    process_.setProgram("python3");
    process_.setArguments({"-m", "src.python.integration.backend_pipeline"});
    running_ = true;
    emit backendStarted();
    emit backendLog("Project root: " + root);
    emit backendLog("Starting Python backend pipeline...");
    process_.start();
    if (!process_.waitForStarted(2000)) {
        running_ = false;
        timeoutTimer_.stop();
        emit backendError("Unable to start python3: " + process_.errorString());
        return;
    }

    const QByteArray payload = QJsonDocument(command).toJson(QJsonDocument::Compact);
    process_.write(payload);
    process_.closeWriteChannel();
    timeoutTimer_.start();
}

void BackendProcess::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    timeoutTimer_.stop();
    const QByteArray output = process_.readAllStandardOutput();
    const QByteArray errorOutput = process_.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        standardError_.append(errorOutput);
    }
    running_ = false;

    if (exitStatus != QProcess::NormalExit) {
        emit backendError("Backend process crashed before returning a result.");
        return;
    }
    if (output.trimmed().isEmpty()) {
        emit backendError("Backend returned no JSON result (exit code " + QString::number(exitCode) + ").");
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(output, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        emit backendError("Malformed backend JSON: " + parseError.errorString());
        return;
    }

    const QJsonObject result = document.object();
    const QString validationError = validateResult(result);
    if (!validationError.isEmpty()) {
        emit backendError(validationError);
        return;
    }
    emit backendLog("Python pipeline completed successfully.");
    emit backendCompleted(result);
}

void BackendProcess::handleError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart || error == QProcess::Crashed) {
        running_ = false;
        timeoutTimer_.stop();
        emit backendError(process_.errorString());
    }
}

void BackendProcess::handleStandardError()
{
    const QByteArray message = process_.readAllStandardError();
    if (!message.isEmpty()) {
        standardError_.append(message);
        emit backendLog("Backend stderr: " + QString::fromLocal8Bit(message).trimmed());
    }
}

void BackendProcess::handleTimeout()
{
    if (!running_) {
        return;
    }
    process_.terminate();
    if (!process_.waitForFinished(500)) {
        process_.kill();
    }
    running_ = false;
    emit backendError("Backend timeout after 15 seconds.");
}

void BackendProcess::resetProcess()
{
    if (process_.state() != QProcess::NotRunning) {
        process_.kill();
        process_.waitForFinished(500);
    }
    standardError_.clear();
}

QString BackendProcess::validateResult(const QJsonObject &result) const
{
    if (!result.contains("decision") || !result.value("decision").isObject()) {
        return "Backend response is missing the decision object.";
    }
    const QJsonObject decision = result.value("decision").toObject();
    if (!decision.value("action").isString() || !decision.value("reason").isString()
        || !decision.value("severity").isString()) {
        return "Backend decision is missing action, reason, or severity.";
    }
    return {};
}
