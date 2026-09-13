#ifndef CAN_H_
#define CAN_H_


#include <stdint.h>
#include "esp_err.h"


struct CanMessage
{
    uint32_t id;
    uint8_t dlc;
    uint8_t is_ext;
    uint8_t flag;
    uint8_t r;
    uint8_t data[8];
};
typedef struct CanMessage CanMessage;

esp_err_t InitCan();
esp_err_t DeinitCan();

void TaskCan(void * arg);

#endif // CAN_H_