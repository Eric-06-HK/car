/**
 * @file    shopping_cart.c
 * @brief   智能购物车业务逻辑实现
 */

#include "shopping_cart.h"
#include "string.h"
#include "stdio.h"
#include "hal_usart.h"

// 全局购物车实例
ShoppingCart_t g_shoppingCart = {0};

// 扫码枪缓冲区（环形缓冲）
#define BARCODE_BUFFER_SIZE   64
static char s_barcodeBuffer[BARCODE_BUFFER_SIZE] = {0};
static uint8_t s_barcodeIndex = 0;
static uint8_t s_barcodeReady = 0;

// 模拟商品数据库（实际应用中可从Flash或外部存储读取）
typedef struct
{
    const char* barcode;
    const char* name;
    float price;
} ProductDB_t;

// 示例商品数据库（可根据需要扩展）
static const ProductDB_t s_productDB[] =
{
    {"6901234567890", "可口可乐 330ml", 3.50f},
    {"6901234567891", "雪碧 330ml", 3.50f},
    {"6901234567892", "芬达 330ml", 3.50f},
    {"6909876543210", "康师傅红烧牛肉面", 4.50f},
    {"6909876543211", "统一老坛酸菜面", 4.00f},
    {"6912345678901", "乐事薯片原味", 7.50f},
    {"6912345678902", "乐事薯片黄瓜味", 7.50f},
    {"6923456789012", "农夫山泉 550ml", 2.00f},
    {"6923456789013", "怡宝纯净水 555ml", 2.00f},
    {"6934567890123", "蒙牛纯牛奶 250ml", 3.80f},
    // 可继续添加更多商品...
};

#define PRODUCT_DB_SIZE (sizeof(s_productDB) / sizeof(s_productDB[0]))

/**
 * @brief  初始化购物车
 */
void ShoppingCart_Init(void)
{
    memset(&g_shoppingCart, 0, sizeof(ShoppingCart_t));
    g_shoppingCart.state = CART_STATE_IDLE;
    g_shoppingCart.productCount = 0;
    g_shoppingCart.currentPage = 0;
    g_shoppingCart.selectedIndex = 0;
    g_shoppingCart.grandTotal = 0.0f;
    g_shoppingCart.isModified = 0;

    // 清空扫码缓冲区
    memset(s_barcodeBuffer, 0, BARCODE_BUFFER_SIZE);
    s_barcodeIndex = 0;
    s_barcodeReady = 0;
}

/**
 * @brief  根据条形码查找商品信息
 * @param  barcode: 条形码字符串
 * @param  product: 输出商品结构体指针
 * @return 1-找到，0-未找到
 */
static uint8_t FindProductByBarcode(const char* barcode, Product_t* product)
{
    uint8_t i;

    for(i = 0; i < PRODUCT_DB_SIZE; i++)
    {
        if(strcmp(barcode, s_productDB[i].barcode) == 0)
        {
            strncpy(product->barcode, barcode, BARCODE_MAX_LEN - 1);
            strncpy(product->name, s_productDB[i].name, PRODUCT_NAME_MAX_LEN - 1);
            product->price = s_productDB[i].price;
            product->quantity = 1;
            product->total = product->price;
            product->isSelected = 0;
            return 1;
        }
    }

    // 未找到商品，使用默认信息
    strncpy(product->barcode, barcode, BARCODE_MAX_LEN - 1);
    snprintf(product->name, PRODUCT_NAME_MAX_LEN, "未知商品:%s", barcode);
    product->price = 0.0f;
    product->quantity = 1;
    product->total = 0.0f;
    product->isSelected = 0;

    return 0;
}

/**
 * @brief  添加商品到购物车
 * @param  barcode: 商品条形码
 * @return 1-成功，0-失败（购物车满）
 */
uint8_t ShoppingCart_AddProduct(const char* barcode)
{
    uint8_t i;
    Product_t newProduct;

    // 检查购物车是否已满
    if(g_shoppingCart.productCount >= MAX_PRODUCT_TYPES)
    {
        return 0;
    }

    // 查找商品信息
    if(!FindProductByBarcode(barcode, &newProduct))
    {
        // 新商品，添加到购物车
        memcpy(&g_shoppingCart.products[g_shoppingCart.productCount], &newProduct, sizeof(Product_t));
        g_shoppingCart.productCount++;
        g_shoppingCart.isModified = 1;
    }
    else
    {
        // 已存在商品，数量+1
        for(i = 0; i < g_shoppingCart.productCount; i++)
        {
            if(strcmp(g_shoppingCart.products[i].barcode, barcode) == 0)
            {
                g_shoppingCart.products[i].quantity++;
                g_shoppingCart.products[i].total = g_shoppingCart.products[i].price * g_shoppingCart.products[i].quantity;
                g_shoppingCart.isModified = 1;
                break;
            }
        }

        // 如果是新商品则添加
        if(i == g_shoppingCart.productCount)
        {
            memcpy(&g_shoppingCart.products[g_shoppingCart.productCount], &newProduct, sizeof(Product_t));
            g_shoppingCart.productCount++;
            g_shoppingCart.isModified = 1;
        }
    }

    // 重新计算总额
    ShoppingCart_CalculateTotal();

    return 1;
}

/**
 * @brief  从购物车移除商品
 * @param  index: 商品索引
 * @return 1-成功，0-失败
 */
uint8_t ShoppingCart_RemoveProduct(uint8_t index)
{
    if(index >= g_shoppingCart.productCount)
    {
        return 0;
    }

    // 移动后续商品前移
    memmove(&g_shoppingCart.products[index],
            &g_shoppingCart.products[index + 1],
            (g_shoppingCart.productCount - index - 1) * sizeof(Product_t));

    g_shoppingCart.productCount--;
    g_shoppingCart.isModified = 1;

    // 调整选中索引
    if(g_shoppingCart.selectedIndex >= g_shoppingCart.productCount)
    {
        g_shoppingCart.selectedIndex = g_shoppingCart.productCount > 0 ? g_shoppingCart.productCount - 1 : 0;
    }

    ShoppingCart_CalculateTotal();

    return 1;
}

/**
 * @brief  更新商品数量
 * @param  index: 商品索引
 * @param  delta: 数量变化（正数增加，负数减少）
 */
void ShoppingCart_UpdateQuantity(uint8_t index, int8_t delta)
{
    if(index >= g_shoppingCart.productCount)
    {
        return;
    }

    int newQty = (int)g_shoppingCart.products[index].quantity + delta;

    if(newQty <= 0)
    {
        // 数量为0时移除商品
        ShoppingCart_RemoveProduct(index);
    }
    else if(newQty <= 99)
    {
        g_shoppingCart.products[index].quantity = (uint8_t)newQty;
        g_shoppingCart.products[index].total = g_shoppingCart.products[index].price * newQty;
        g_shoppingCart.isModified = 1;
        ShoppingCart_CalculateTotal();
    }
}

/**
 * @brief  计算购物车总金额
 * @return 总金额
 */
float ShoppingCart_CalculateTotal(void)
{
    uint8_t i;
    g_shoppingCart.grandTotal = 0.0f;

    for(i = 0; i < g_shoppingCart.productCount; i++)
    {
        g_shoppingCart.grandTotal += g_shoppingCart.products[i].total;
    }

    return g_shoppingCart.grandTotal;
}

/**
 * @brief  下一页（屏幕显示用，每页显示2行）
 */
void ShoppingCart_NextPage(void)
{
    if(g_shoppingCart.productCount > 0)
    {
        uint8_t maxPage = (g_shoppingCart.productCount + 1) / 2;  // 每页2个商品
        if(g_shoppingCart.currentPage < maxPage - 1)
        {
            g_shoppingCart.currentPage++;
        }
    }
}

/**
 * @brief  上一页
 */
void ShoppingCart_PrevPage(void)
{
    if(g_shoppingCart.currentPage > 0)
    {
        g_shoppingCart.currentPage--;
    }
}

/**
 * @brief  选中商品
 * @param  index: 商品索引
 */
void ShoppingCart_SelectProduct(uint8_t index)
{
    if(index >= g_shoppingCart.productCount)
    {
        return;
    }

    // 取消所有选中
    ShoppingCart_DeselectAll();

    // 选中新商品
    g_shoppingCart.products[index].isSelected = 1;
    g_shoppingCart.selectedIndex = index;
    g_shoppingCart.state = CART_STATE_SELECTED;
}

/**
 * @brief  取消所有选中
 */
void ShoppingCart_DeselectAll(void)
{
    uint8_t i;
    for(i = 0; i < g_shoppingCart.productCount; i++)
    {
        g_shoppingCart.products[i].isSelected = 0;
    }
    g_shoppingCart.state = CART_STATE_IDLE;
}

/**
 * @brief  结账（保存数据）
 * @return 1-成功，0-失败
 */
uint8_t ShoppingCart_Checkout(void)
{
    if(g_shoppingCart.productCount == 0)
    {
        return 0;
    }

    // 标记为结账状态
    g_shoppingCart.state = CART_STATE_CHECKOUT;

    // TODO: 此处添加保存到Flash的代码
    // Flash_SaveCartData(&g_shoppingCart);

    g_shoppingCart.state = CART_STATE_SAVED;
    g_shoppingCart.isModified = 0;

    return 1;
}

/**
 * @brief  清空购物车
 */
void ShoppingCart_Clear(void)
{
    ShoppingCart_Init();
}

/**
 * @brief  获取当前选中的商品
 * @return 商品指针，无选中返回NULL
 */
Product_t* ShoppingCart_GetCurrentProduct(void)
{
    if(g_shoppingCart.selectedIndex < g_shoppingCart.productCount &&
       g_shoppingCart.products[g_shoppingCart.selectedIndex].isSelected)
    {
        return &g_shoppingCart.products[g_shoppingCart.selectedIndex];
    }
    return NULL;
}

/**
 * @brief  初始化扫码枪（USART3）
 */
void BarcodeScanner_Init(void)
{
    // USART3初始化将在hal_usart.c中添加
    // 此处仅初始化软件状态
    memset(s_barcodeBuffer, 0, BARCODE_BUFFER_SIZE);
    s_barcodeIndex = 0;
    s_barcodeReady = 0;
}

/**
 * @brief  获取扫码数据
 * @param  buffer: 输出缓冲区
 * @param  maxLen: 最大长度
 * @return 实际读取长度
 */
uint8_t BarcodeScanner_GetBarcode(char* buffer, uint8_t maxLen)
{
    if(s_barcodeReady)
    {
        uint8_t len = strlen(s_barcodeBuffer);
        if(len >= maxLen)
        {
            len = maxLen - 1;
        }
        strncpy(buffer, s_barcodeBuffer, len);
        buffer[len] = '\0';

        // 清除标志
        s_barcodeReady = 0;
        s_barcodeIndex = 0;
        memset(s_barcodeBuffer, 0, BARCODE_BUFFER_SIZE);

        return len;
    }
    return 0;
}

/**
 * @brief  处理扫码数据（需在中断或主循环中调用）
 */
void BarcodeScanner_ProcessData(void)
{
    // 此函数应在USART3中断中被调用
    // 当接收到完整条形码后设置s_barcodeReady标志
}

/**
 * @brief  USART3接收回调（需在中断服务函数中调用）
 * @param  data: 接收到的字节
 */
void BarcodeScanner_OnDataReceived(uint8_t data)
{
    // 假设扫码枪以回车符结束
    if(data == '\r' || data == '\n')
    {
        if(s_barcodeIndex > 0)
        {
            s_barcodeBuffer[s_barcodeIndex] = '\0';
            s_barcodeReady = 1;

            // 自动添加到购物车
            ShoppingCart_AddProduct(s_barcodeBuffer);
        }
        s_barcodeIndex = 0;
    }
    else if(s_barcodeIndex < BARCODE_BUFFER_SIZE - 1)
    {
        s_barcodeBuffer[s_barcodeIndex++] = data;
    }
}