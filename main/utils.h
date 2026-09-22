#ifndef UTILS_H_
#define UTILS_H_


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define RGB(c) { .red = ((c) >> 16) & 0xFF, .green = ((c) >> 8) & 0xFF, .blue = (c) & 0xFF }

    /// @brief 线性插值函数
    /// @param out_min 输出范围最小值
    /// @param out_max 输出范围最大值
    /// @param in_min 输入范围最小值
    /// @param in_max 输入范围最大值
    /// @param x 输入值
    /// @return 对应的输出值
    /// @note 如果输入范围为零，返回输出范围的最小值
    /// @note 外插截断
    static inline int32_t Lerp(int32_t out_min, int32_t out_max, int32_t in_min, int32_t in_max, int32_t x)
    {
        if (in_max == in_min) {
            return out_min;
        }
        if (x <= in_min) {
            return out_min;
        }
        if (x >= in_max) {
            return out_max;
        }
        int64_t num = (int64_t)(x - in_min) * (out_max - out_min);
        int64_t den = (int64_t)in_max - in_min;
        if (den < 0) {
            num = -num;
            den = -den;
        }
        int64_t r = 0;
        if (num >= 0) {
            r = (num + den / 2) / den;
        } else {
            r = (num - den / 2) / den;
        }
        return out_min + (int32_t)r;
    }

#ifdef __cplusplus
}
#endif

#endif // UTILS_H_