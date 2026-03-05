/**
 * @file    shopping_cart.h
 * @brief   智能购物车业务逻辑头文件
 */

#ifndef __SHOPPING_CART_H
#define __SHOPPING_CART_H

#include "stm32f10x.h"
#include <stdint.h>

// 最大商品种类数
#define MAX_PRODUCT_TYPES     20
// 条形码最大长度
#define BARCODE_MAX_LEN       32
// 商品名称最大长度
#define PRODUCT_NAME_MAX_LEN  64

// 购物车状态机
typedef enum
{
    CART_STATE_IDLE = 0,      // 空闲待机
    CART_STATE_SCANNING,      // 等待扫码
    CART_STATE_SELECTED,      // 已选中商品
    CART_STATE_CHECKOUT,      // 结账中
    CART_STATE_SAVED          // 已保存
} CartState_t;

// 商品数据结构
typedef struct
{
    char barcode[BARCODE_MAX_LEN];        // 条形码
    char name[PRODUCT_NAME_MAX_LEN];      // 商品名称
    float price;                          // 单价（元）
    uint8_t quantity;                     // 数量
    float total;                          // 小计
    uint8_t isSelected;                   // 是否被选中
} Product_t;

// 购物车数据结构
typedef struct
{
    Product_t products[MAX_PRODUCT_TYPES]; // 商品数组
    uint8_t productCount;                  // 当前商品种类数
    uint8_t currentPage;                   // 当前页码（屏幕显示用）
    uint8_t selectedIndex;                 // 当前选中的商品索引
    float grandTotal;                      // 总金额
    CartState_t state;                     // 当前状态
    uint8_t isModified;                    // 是否有修改未保存
} ShoppingCart_t;

// 全局购物车实例
extern ShoppingCart_t g_shoppingCart;

// 函数声明
void ShoppingCart_Init(void);
uint8_t ShoppingCart_AddProduct(const char* barcode);
uint8_t ShoppingCart_RemoveProduct(uint8_t index);
void ShoppingCart_UpdateQuantity(uint8_t index, int8_t delta);
float ShoppingCart_CalculateTotal(void);
void ShoppingCart_NextPage(void);
void ShoppingCart_PrevPage(void);
void ShoppingCart_SelectProduct(uint8_t index);
void ShoppingCart_DeselectAll(void);
uint8_t ShoppingCart_Checkout(void);
void ShoppingCart_Clear(void);
Product_t* ShoppingCart_GetCurrentProduct(void);

// 扫码枪相关
void BarcodeScanner_Init(void);
uint8_t BarcodeScanner_GetBarcode(char* buffer, uint8_t maxLen);
void BarcodeScanner_ProcessData(void);

#endif /* __SHOPPING_CART_H */