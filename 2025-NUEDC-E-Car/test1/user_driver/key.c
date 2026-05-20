#include "key.h"

uint8_t KEY_Scan(void) {
    // 内部已配置上拉，按下时读取为 0
    if (DL_GPIO_readPins(key_key1_PORT, key_key1_PIN) == 0) return 1;
    if (DL_GPIO_readPins(key_key2_PORT, key_key2_PIN) == 0) return 2;
    if (DL_GPIO_readPins(key_key3_PORT, key_key3_PIN) == 0) return 3;
    if (DL_GPIO_readPins(key_key4_PORT, key_key4_PIN) == 0) return 4;
    if (DL_GPIO_readPins(key_key5_PORT, key_key5_PIN) == 0) return 5;
    if (DL_GPIO_readPins(key_key6_PORT, key_key6_PIN) == 0) return 6;
    
    return 0; // 没有按键按下
}