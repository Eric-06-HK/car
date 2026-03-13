/**********************************************************************************
 * @file    main.c
 * @author  智能购物车系统
 * @version V1.1.0
 * @date    2024.01.01
 * @brief   智能购物车主程序 - 扫码枪 +5 按键+OLED 显示 +UWB 跟随+TOF 避障
**********************************************************************************/

#include "stm32f10x.h"
#include "delay.h"
#include "Periph_init.h"
#include "timer.h"
#include "hal_usart.h"
#include "control.h"
#include "uwb.h"
#include "hal_iic.h"
#include "oled_i2c.h"
#include "motor.h"
#include "hal_key.h"
#include "shopping_cart.h"
#include "cart_display.h"

// CAR_STOP 宏定义（如果 motor.h 中未定义）
#ifndef CAR_STOP
#define CAR_STOP()  do { \
    motor_speed_A(0); \
    motor_speed_B(0); \
    motor_speed_C(0); \
    motor_speed_D(0); \
} while(0)
#endif

// 全局模式标志
typedef enum
{
    MODE_SHOPPING = 0,      // 购物模式：扫码、选商品、结账
    MODE_FOLLOWING = 1,     // 跟随模式：UWB 跟随 +TOF 避障
    MODE_CHECKOUT_DONE = 2  // 结账完成等待重置
} SystemMode_t;

static SystemMode_t g_systemMode = MODE_SHOPPING;
static uint64_t g_modeSwitchTime = 0;
static uint8_t g_key5PressCount = 0;

/*******************************************************************************
 * 函数名  : init
 * 描述    : 初始化函数
 *******************************************************************************/
void init(void)
{
    // 初始化设置
    SystemInit();
    RCC_Configuration_part();
    SysTick_Init();              // 嘀嗒定时
    Periph_init();               // 指示灯 + 蜂鸣器初始化
    Timer_Init();                // 定时器初始化设置
    Uart_Queue_Init();           // PDOA 数据队列初始化
    HalUARTInit();               // 串口 1+ 串口 2+ 串口 3(扫码枪) 设置
    HalKey_Init();               // 按键初始化（5 键购物车专用）
    ShoppingCart_Init();         // 购物车系统初始化
    BarcodeScanner_Init();       // 扫码枪初始化
    CartDisplay_Init();          // 屏幕显示初始化（替换原有 UWB 内容）
    EXTI_ALL_Init();             // 编码器 IO 初始化
    Motor_Gpio_init();           // 马达电机 IO 口初始化
}

/*******************************************************************************
 * 函数名  : main
 * 描述    : 主函数
 *******************************************************************************/
int main(void)
{
    init();

    while(1)
    {
        // 模式切换逻辑
        switch(g_systemMode)
        {
            case MODE_SHOPPING:
            {
                // 购物模式：处理扫码、按键、显示
                uint8_t keyValue = HalKey_Scan();

                if(keyValue != 0)
                {
                    // 处理按键事件
                    switch(keyValue)
                    {
                        case KEY1_MASK:  // KEY1: 选中/取消选中商品
                            if(g_shoppingCart.productCount > 0)
                            {
                                uint8_t idx = g_shoppingCart.currentPage * 2;
                                if(idx < g_shoppingCart.productCount)
                                {
                                    if(g_shoppingCart.products[idx].isSelected)
                                    {
                                        ShoppingCart_DeselectAll();
                                    }
                                    else
                                    {
                                        ShoppingCart_SelectProduct(idx);
                                    }
                                }
                            }
                            break;

                        case KEY2_MASK:  // KEY2: 取消选中/返回
                            ShoppingCart_DeselectAll();
                            break;

                        case KEY3_MASK:  // KEY3: 上翻/增加数量
                            if(g_shoppingCart.state == CART_STATE_SELECTED)
                            {
                                ShoppingCart_UpdateQuantity(g_shoppingCart.selectedIndex, 1);
                            }
                            else
                            {
                                ShoppingCart_PrevPage();
                            }
                            break;

                        case KEY4_MASK:  // KEY4: 下翻/减少数量
                            if(g_shoppingCart.state == CART_STATE_SELECTED)
                            {
                                ShoppingCart_UpdateQuantity(g_shoppingCart.selectedIndex, -1);
                            }
                            else
                            {
                                ShoppingCart_NextPage();
                            }
                            break;

                        case KEY5_MASK:  // KEY5: 结账（长按 2 秒进入跟随模式）
                            if(g_shoppingCart.productCount > 0)
                            {
                                if(ShoppingCart_Checkout())
                                {
                                    CartDisplay_ShowCheckoutInfo(g_shoppingCart.grandTotal,
                                                                g_shoppingCart.productCount);
                                    g_key5PressCount++;
                                    if(g_key5PressCount >= 40) // 约 2 秒（50ms*40）
                                    {
                                        g_systemMode = MODE_FOLLOWING;
                                        g_modeSwitchTime = portGetTickCnt();
                                        g_key5PressCount = 0;
                                        ShoppingCart_Clear(); // 清空购物车准备下次使用
                                    }
                                }
                            }
                            else
                            {
                                // 无商品时直接切换到跟随模式
                                g_key5PressCount++;
                                if(g_key5PressCount >= 40)
                                {
                                    g_systemMode = MODE_FOLLOWING;
                                    g_modeSwitchTime = portGetTickCnt();
                                    g_key5PressCount = 0;
                                }
                            }
                            break;

                        default:
                            break;
                    }

                    HalKey_ClearFlag(0);
                }
                else
                {
                    g_key5PressCount = 0; // 重置计数
                }

                // 更新屏幕显示（商品信息）
                CartDisplay_Update();

                // 停止电机（购物模式下）
                CAR_STOP();
                break;
            }

            case MODE_FOLLOWING:
            {
                // 跟随模式：运行 UWB 跟随和 TOF 避障
                // 检测是否需要切换回购物模式（短按 KEY5 或超时）
                uint8_t keyValue = HalKey_Scan();
                if(keyValue == KEY5_MASK)
                {
                    // 短按 KEY5 切换回购物模式
                    g_systemMode = MODE_SHOPPING;
                    g_modeSwitchTime = portGetTickCnt();
                    Delay_Ms(300); // 去抖
                }

                // 执行跟随和避障控制
                Read_AoA_Control();
                break;
            }

            case MODE_CHECKOUT_DONE:
            {
                // 结账完成状态（可选扩展）
                // 等待用户确认后返回购物模式
                uint8_t keyValue = HalKey_Scan();
                if(keyValue == KEY5_MASK)
                {
                    g_systemMode = MODE_SHOPPING;
                    ShoppingCart_Clear();
                }
                break;
            }

            default:
                g_systemMode = MODE_SHOPPING;
                break;
        }
    }
}

