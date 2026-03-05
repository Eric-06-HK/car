/**
 * @file    cart_display.c
 * @brief   购物车屏幕显示模块实现
 *
 * 屏幕布局（128x64 OLED，两行显示）：
 * 第1行：[商品名] x[数量]  (选中时前面加">")
 * 第2行：总价：xx.xx元 [页码/总页数]
 */

#include "cart_display.h"
#include "shopping_cart.h"
#include "oled_i2c.h"
#include "stdio.h"
#include "string.h"

// 显示缓冲区
static char s_lineBuffer[DISPLAY_LINES][DISPLAY_COLS + 1] = {0};

/**
 * @brief  初始化显示屏
 */
void CartDisplay_Init(void)
{
    // OLED已在main.c中初始化，此处只需清屏
    OLED_CLS();
    CartDisplay_ShowMessage("智能购物车");
}

/**
 * @brief  更新屏幕显示
 */
void CartDisplay_Update(void)
{
    ShoppingCart_t* cart = &g_shoppingCart;

    if(cart->productCount == 0)
    {
        // 空购物车
        snprintf(s_lineBuffer[0], DISPLAY_COLS, "  请扫描商品");
        snprintf(s_lineBuffer[1], DISPLAY_COLS, "  总价：0.00元");
    }
    else
    {
        uint8_t startIndex = cart->currentPage * 2;  // 每页2个商品
        uint8_t maxPage = (cart->productCount + 1) / 2;

        // 构建第1行显示内容
        if(startIndex < cart->productCount)
        {
            Product_t* prod = &cart->products[startIndex];
            if(prod->isSelected)
            {
                // 选中的商品前面加">"
                snprintf(s_lineBuffer[0], DISPLAY_COLS, ">%s x%d", prod->name, prod->quantity);
            }
            else
            {
                snprintf(s_lineBuffer[0], DISPLAY_COLS, " %s x%d", prod->name, prod->quantity);
            }

            // 截断过长的商品名（最多显示10个字符）
            if(strlen(s_lineBuffer[0]) > DISPLAY_COLS)
            {
                s_lineBuffer[0][DISPLAY_COLS - 1] = '\0';
            }
        }
        else
        {
            snprintf(s_lineBuffer[0], DISPLAY_COLS, " ");
        }

        // 构建第2行显示内容（总价和页码）
        snprintf(s_lineBuffer[1], DISPLAY_COLS, "%.2f元 %d/%d",
                 cart->grandTotal, cart->currentPage + 1, maxPage);
    }

    // 刷新到OLED屏幕
    OLED_CLS();
    OLED_ShowStr(0, 0, (unsigned char*)s_lineBuffer[0], 16);
    OLED_ShowStr(0, 2, (unsigned char*)s_lineBuffer[1], 16);
}

/**
 * @brief  显示单个商品信息
 * @param  index: 商品索引
 */
void CartDisplay_ShowProduct(uint8_t index)
{
    ShoppingCart_t* cart = &g_shoppingCart;

    if(index >= cart->productCount)
    {
        return;
    }

    Product_t* prod = &cart->products[index];

    snprintf(s_lineBuffer[0], DISPLAY_COLS, "%s", prod->name);
    snprintf(s_lineBuffer[1], DISPLAY_COLS, "%.2f x %d = %.2f",
             prod->price, prod->quantity, prod->total);

    OLED_CLS();
    OLED_ShowStr(0, 0, (unsigned char*)s_lineBuffer[0], 16);
    OLED_ShowStr(0, 2, (unsigned char*)s_lineBuffer[1], 16);
}

/**
 * @brief  显示购物车汇总信息
 */
void CartDisplay_ShowCartInfo(void)
{
    ShoppingCart_t* cart = &g_shoppingCart;

    snprintf(s_lineBuffer[0], DISPLAY_COLS, "商品数：%d", cart->productCount);
    snprintf(s_lineBuffer[1], DISPLAY_COLS, "总计：%.2f元", cart->grandTotal);

    OLED_CLS();
    OLED_ShowStr(0, 0, (unsigned char*)s_lineBuffer[0], 16);
    OLED_ShowStr(0, 2, (unsigned char*)s_lineBuffer[1], 16);
}

/**
 * @brief  清屏
 */
void CartDisplay_Clear(void)
{
    OLED_CLS();
}

/**
 * @brief  显示消息
 * @param  msg: 消息字符串
 */
void CartDisplay_ShowMessage(const char* msg)
{
    OLED_CLS();
    // 居中显示（简单实现，实际可根据字符串长度计算偏移）
    OLED_ShowStr(16, 1, (unsigned char*)msg, 16);
}

/**
 * @brief  显示结账信息
 * @param  total: 总金额
 * @param  itemCount: 商品数量
 */
void CartDisplay_ShowCheckoutInfo(float total, uint8_t itemCount)
{
    snprintf(s_lineBuffer[0], DISPLAY_COLS, "结账：共%d件", itemCount);
    snprintf(s_lineBuffer[1], DISPLAY_COLS, "应付：%.2f元", total);

    OLED_CLS();
    OLED_ShowStr(0, 0, (unsigned char*)s_lineBuffer[0], 16);
    OLED_ShowStr(0, 2, (unsigned char*)s_lineBuffer[1], 16);
}