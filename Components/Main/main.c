/**********************************************************************************
 * @file    main.c
 * @author  智能购物车系统
 * @version V1.0.0
 * @date    2024.01.01
 * @brief   智能购物车主程序 - 扫码枪 +5 按键+OLED 显示
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
        // 按键扫描
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

                case KEY5_MASK:  // KEY5: 结账
                    if(g_shoppingCart.productCount > 0)
                    {
                        if(ShoppingCart_Checkout())
                        {
                            CartDisplay_ShowCheckoutInfo(g_shoppingCart.grandTotal,
                                                        g_shoppingCart.productCount);
                            Delay_Ms(2000);
                        }
                    }
                    break;

                default:
                    break;
            }

            HalKey_ClearFlag(0);
        }

        // 更新屏幕显示
        CartDisplay_Update();

        // 原 UWB 跟随功能已注释，如需保留可取消下面注释
        // Read_AoA_Control();
    }
}

