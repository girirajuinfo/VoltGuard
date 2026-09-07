#ifndef VOLTGUARD_DASHBOARD_TYPES_H
#define VOLTGUARD_DASHBOARD_TYPES_H

#include <QJsonObject>
#include <QString>

namespace dashboard {

struct CommandValues {
    int transactionId = 1;
    int protocolId = 0;
    int unitId = 1;
    int functionCode = 3;
    int address = 0;
    int quantity = 2;
    int value = 0;

    QJsonObject toJson() const;
};

struct HistoryEntry {
    QString time;
    QString functionCode;
    QString address;
    QString value;
    QString physics;
    QString decision;
    QString severity;
};

} // namespace dashboard

#endif
