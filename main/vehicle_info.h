#ifndef VEHICLE_INFO_H_
#define VEHICLE_INFO_H_

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VEHICLE_INFO_VALID_VEHICLE_SPEED (1U << 0)
#define VEHICLE_INFO_VALID_IC_VEHICLE_SPEED (1U << 1)
#define VEHICLE_INFO_VALID_ENGINE_SPEED (1U << 2)
#define VEHICLE_INFO_VALID_POWER (1U << 3)

    struct VehicleInfo {
        uint32_t veh_spd;
        uint32_t ic_veh_spd;
        uint32_t eng_spd;
        int32_t power;
        uint32_t valid_mask;
        uint32_t updated_at_ms;
    };
    typedef struct VehicleInfo VehicleInfo;

#pragma pack(1)
    union Msg238 {
        uint8_t buffer[8];
        struct {
            /// @note 0.25,0 rpm
            uint16_t EngSpd;
            /// @note 0.0625,0 km/h
            uint16_t VehSpd : 12;
            /// @note 1 "P" 2 "R" 3 "N" 4 "D"
            uint16_t Gear : 4;
            uint8_t : 8;
            uint8_t : 8;
            uint8_t : 8;
            uint8_t : 8;
        };
    };
    typedef union Msg238 Msg238;
    union Msg3D9 {
        uint8_t buffer[7 * 7];
        struct {
            uint8_t msg1[7];
            uint8_t msg2[7];
            uint8_t msg3[7];
            /// @brief msg4
            struct {
                /// @note 1,0 km
                uint8_t RangeEV;
                uint8_t : 8;
                uint8_t : 8;
                uint8_t : 8;
                uint8_t : 8;
                uint16_t : 4;
                /// @note 0.5,-1000.5 kw
                uint16_t Power : 12;
            };
            /// @brief msg5
            struct {
                /// @note 0.1,0 km
                uint32_t ODO : 24;
                uint32_t : 8;
                uint8_t : 8;
                uint8_t : 8;
                uint8_t : 4;
                uint8_t RC : 4;
            };
            /// @brief msg6
            struct {
                uint8_t : 8;
                uint8_t : 8;
                uint8_t : 8;
                /// @note 1,0 km/h
                uint8_t IcVehSpd;
                uint8_t : 8;
                uint8_t : 8;
                uint8_t : 8;
            };
            /// @brief msg7
            struct {
                uint8_t EngCoolTemp;
                uint16_t TripHEV;
                uint8_t : 8;
                uint8_t : 8;
                uint8_t : 8;
                uint8_t : 8;
            };
        };
    };
    typedef union Msg3D9 Msg3D9;

#pragma pack()

    bool IsRequiredMessage(uint32_t id);
    void VehicleInfoReceiveCan(const CanMessage* msg);
    bool VehicleInfoGetSnapshot(VehicleInfo* snapshot);
    esp_err_t InitVehicleInfo();

#ifdef __cplusplus
}
#endif

#endif // VEHICLE_INFO_H_