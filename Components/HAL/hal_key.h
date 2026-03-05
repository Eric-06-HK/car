/**
 * @file    hal_key.h
 * @brief   按键驱动头文件 - 5键购物车专用
 */

#ifndef __HAL_KEY_H
#define __HAL_KEY_H

#include "stm32f10x.h"
#include <stdint.h>

// 按键数量
#define KEY_NUM             5

// 消抖参数（假设10ms调用一次扫描）
#define KEY_DEBOUNCE_TIMES  3       // 30ms消抖
#define KEY_LONG_PRESS_TIMES 100     // 1s长按

// 按键掩码定义
#define KEY1_MASK           0x01    // 选中/确认
#define KEY2_MASK           0x02    // 取消/返回
#define KEY3_MASK           0x04    // 上翻/增加
#define KEY4_MASK           0x08    // 下翻/减少
#define KEY5_MASK           0x10    // 结账

// 按键ID定义
#define KEY_ID_1            0
#define KEY_ID_2            1
#define KEY_ID_3            2
#define KEY_ID_4            3
#define KEY_ID_5            4

// 按键状态枚举
typedef enum
{
    KEY_IDLE = 0,       // 空闲
    KEY_PRESSED,        // 短按按下
    KEY_RELEASED,       // 释放
    KEY_LONG_PRESSED    // 长按
} KeyState_t;

// 函数声明
void HalKey_Init(void);
uint8_t HalKey_Scan(void);
KeyState_t HalKey_GetState(uint8_t keyId);
void HalKey_ClearFlag(uint8_t keyId);
uint8_t HalKey_IsPressed(uint8_t keyId);

#endif /* __HAL_KEY_H */