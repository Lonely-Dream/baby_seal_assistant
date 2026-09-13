#ifndef VEHICLE_INFO_H_
#define VEHICLE_INFO_H_

#include <stdint.h>
#include "can.h"

struct VehicleInfo
{
    uint32_t speed;
    uint32_t rpm;
    int32_t power;
};
typedef struct VehicleInfo VehicleInfo;

extern VehicleInfo g_vehicle_info;

void VehicleInfoReceiveCan(const CanMessage *msg);

#endif // VEHICLE_INFO_H_