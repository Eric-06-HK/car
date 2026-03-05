/**
 * @file    cart_display.h
 * @brief   购物车屏幕显示模块头文件
 */

#ifndef __CART_DISPLAY_H
#define __CART_DISPLAY_H

#include "stm32f10x.h"
#include <stdint.h>

// 屏幕显示参数
#define DISPLAY_LINES           2       // OLED显示行数
#define DISPLAY_COLS            16      // 每行最多字符数（8x16字体）

// 函数声明
void CartDisplay_Init(void);
void CartDisplay_Update(void);
void CartDisplay_ShowProduct(uint8_t index);
void CartDisplay_ShowCartInfo(void);
void CartDisplay_Clear(void);
void CartDisplay_ShowMessage(const char* msg);
void CartDisplay_ShowCheckoutInfo(float total, uint8_t itemCount);

#endif /* __CART_DISPLAY_H */