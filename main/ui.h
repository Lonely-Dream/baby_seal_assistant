#ifndef UI_H_
#define UI_H_


#include "esp_err.h"

void SelfCheckUi();
/// @brief 统计fps
/// @note 1000ms调度
void StatUi();
/// @brief ui更新任务
/// @note 10ms调度
void StepUi();
esp_err_t InitUi();

#endif // UI_H_
