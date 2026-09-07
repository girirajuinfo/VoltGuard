#include "dashboardtypes.h"

QJsonObject dashboard::CommandValues::toJson() const
{
    return {
        {"transaction_id", transactionId},
        {"protocol_id", protocolId},
        {"unit_id", unitId},
        {"function_code", functionCode},
        {"address", address},
        {"quantity", quantity},
        {"value", value}
    };
}
