/**
 * @file    hal_key.c
 * @brief   按键驱动 - 5键购物车专用
 * @硬件连接:
 *   KEY1 - PB0 (选中/确认)
 *   KEY2 - PB1 (取消/返回)
 *   KEY3 - PB2 (上翻/增加数量)
 *   KEY4 - PB3 (下翻/减少数量)
 *   KEY5 - PB4 (结账)
 */

#include "hal_key.h"
#include "stm32f10x.h"

// 按键GPIO定义
#define KEY1_PORT       GPIOB
#define KEY1_PIN        GPIO_Pin_0
#define KEY1_RCC        RCC_APB2Periph_GPIOB

#define KEY2_PORT       GPIOB
#define KEY2_PIN        GPIO_Pin_1
#define KEY2_RCC        RCC_APB2Periph_GPIOB

#define KEY3_PORT       GPIOB
#define KEY3_PIN        GPIO_Pin_2
#define KEY3_RCC        RCC_APB2Periph_GPIOB

#define KEY4_PORT       GPIOB
#define KEY4_PIN        GPIO_Pin_3
#define KEY4_RCC        RCC_APB2Periph_GPIOB

#define KEY5_PORT       GPIOB
#define KEY5_PIN        GPIO_Pin_4
#define KEY5_RCC        RCC_APB2Periph_GPIOB

// 按键状态
static KeyState_t s_keyState[KEY_NUM] = {0};
static uint8_t s_keyPressFlag[KEY_NUM] = {0};
static uint16_t s_pressCounter[KEY_NUM] = {0};

// 按键值缓存
static uint8_t s_keyValue = 0;

/**
 * @brief  按键GPIO初始化
 */
void HalKey_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOB时钟
    RCC_APB2PeriphClockCmd(KEY1_RCC | KEY2_RCC | KEY3_RCC | KEY4_RCC | KEY5_RCC, ENABLE);

    // 配置所有按键为浮空输入
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN | KEY4_PIN | KEY5_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入（外部按键接地）
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(KEY1_PORT, &GPIO_InitStructure);
}

/**
 * @brief  按键扫描函数（需定时调用，建议10-20ms）
 * @return 按键值，无按键返回0
 */
uint8_t HalKey_Scan(void)
{
    static uint8_t keyLastState = 0;
    uint8_t keyCurrentState = 0;
    uint8_t i;

    // 读取当前按键状态（低电平表示按下）
    if(GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 0) keyCurrentState |= KEY1_MASK;
    if(GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 0) keyCurrentState |= KEY2_MASK;
    if(GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == 0) keyCurrentState |= KEY3_MASK;
    if(GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN) == 0) keyCurrentState |= KEY4_MASK;
    if(GPIO_ReadInputDataBit(KEY5_PORT, KEY5_PIN) == 0) keyCurrentState |= KEY5_MASK;

    // 检测按键按下（下降沿）
    for(i = 0; i < KEY_NUM; i++)
    {
        uint8_t mask = (1 << i);

        if((keyCurrentState & mask) && !(keyLastState & mask))
        {
            // 按键按下，开始计数消抖
            s_pressCounter[i]++;
            if(s_pressCounter[i] >= KEY_DEBOUNCE_TIMES)
            {
                s_keyPressFlag[i] = 1;
                s_keyValue = mask;
                s_keyState[i] = KEY_PRESSED;
            }
        }
        else if(keyCurrentState & mask)
        {
            // 保持按下状态
            if(s_pressCounter[i] < KEY_LONG_PRESS_TIMES)
            {
                s_pressCounter[i]++;
            }
            else if(s_pressCounter[i] == KEY_LONG_PRESS_TIMES)
            {
                s_keyState[i] = KEY_LONG_PRESSED;
            }
        }
        else
        {
            // 按键释放
            if(s_keyState[i] == KEY_PRESSED || s_keyState[i] == KEY_LONG_PRESSED)
            {
                s_keyState[i] = KEY_RELEASED;
            }
            s_pressCounter[i] = 0;
        }
    }

    keyLastState = keyCurrentState;

    // 返回按键值
    if(s_keyValue != 0)
    {
        uint8_t ret = s_keyValue;
        s_keyValue = 0;  // 清除标志
        return ret;
    }

    return 0;
}

/**
 * @brief  获取按键状态
 * @param  keyId: 按键ID (0-4)
 * @return 按键状态
 */
KeyState_t HalKey_GetState(uint8_t keyId)
{
    if(keyId < KEY_NUM)
    {
        return s_keyState[keyId];
    }
    return KEY_IDLE;
}

/**
 * @brief  清除按键标志
 * @param  keyId: 按键ID
 */
void HalKey_ClearFlag(uint8_t keyId)
{
    if(keyId < KEY_NUM)
    {
        s_keyPressFlag[keyId] = 0;
        s_keyState[keyId] = KEY_IDLE;
    }
}

/**
 * @brief  检测特定按键是否按下
 * @param  keyId: 按键ID
 * @return 1-按下，0-未按下
 */
uint8_t HalKey_IsPressed(uint8_t keyId)
{
    if(keyId < KEY_NUM)
    {
        return s_keyPressFlag[keyId];
    }
    return 0;
}