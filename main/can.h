#ifndef CAN_H_
#define CAN_H_


#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

    struct CanMessage {
        uint32_t id;
        uint8_t dlc;
        uint8_t is_ext;
        uint8_t flag;
        uint8_t r;
        uint8_t data[8];
    };
    typedef struct CanMessage CanMessage;

    /// @brief 处理CAN tx任务
    /// @note 1000ms调度
    void ProcessCanTx();
    esp_err_t InitCan();
    esp_err_t DeinitCan();

    /// @brief 处理CAN任务
    /// @note 10ms调度
    void StepCan();

#ifdef __cplusplus
}
#endif

#endif // CAN_H_