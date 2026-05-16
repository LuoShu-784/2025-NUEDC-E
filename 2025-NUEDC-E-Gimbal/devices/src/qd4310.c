//
// Created by jason on 25-10-16.
//
#include "main.h"
#include "QD4310.h"

// 限制函数，用于替代C++的std::clamp
static float qd4310_clamp(float value, float min, float max) 
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// 发送命令到电机
void qd4310_send_command(qd4310_t *motor, qd4310_cmd_t cmd, int16_t value) 
{
    static uint8_t tx_buff[3];
    tx_buff[0] = (uint8_t)cmd;
    // 将int16_t值拆分为两个字节
    tx_buff[1] = (uint8_t)(value & 0xFF);
    tx_buff[2] = (uint8_t)((value >> 8) & 0xFF);

    uint32_t txailbox = CAN_TX_MAILBOX0;
    CAN_TxHeaderTypeDef TxHeader;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.StdId = 0x400 + motor->id;
    TxHeader.ExtId = 0x400 + motor->id;
    TxHeader.TransmitGlobalTime = DISABLE;
    TxHeader.DLC = 3;
    HAL_CAN_AddTxMessage(motor->hcan, &TxHeader, tx_buff, &txailbox);
    while (HAL_CAN_GetTxMailboxesFreeLevel(motor->hcan) == 0);
}


// 更新电机状态
void qd4310_update(qd4310_t *motor, const uint8_t feedback[8]) 
{
    motor->enabled = feedback[0] & 0x01;

    // 重构int16_t值从字节数组
    int16_t current_raw = (int16_t)((feedback[3] << 8) | feedback[2]);
    motor->current = (float)current_raw * 10.0f / INT16_MAX;

    int16_t speed_raw = (int16_t)((feedback[5] << 8) | feedback[4]);
    motor->speed = (float)speed_raw * 1000.0f / 32767.0f;

    uint16_t angle_raw = (uint16_t)((feedback[7] << 8) | feedback[6]);
    motor->angle = (float)angle_raw * QD4310_TWO_PI / UINT16_MAX;
}


// 使能电机
inline void qd4310_enable(qd4310_t *motor) 
{
    qd4310_send_command(motor, QD4310_CMD_ENABLE, 0x0000);
}

// 失能电机
inline void qd4310_disable(qd4310_t *motor) 
{
    qd4310_send_command(motor, QD4310_CMD_DISABLE, 0x0000);
}

// 设置电机角度
void qd4310_set_angle(qd4310_t *motor, float angle) 
{
    angle = qd4310_clamp(angle, 0.0f, QD4310_TWO_PI);
    int16_t angle_value = (int16_t)(angle / QD4310_TWO_PI * UINT16_MAX);
    qd4310_send_command(motor, QD4310_CMD_ANGLE, angle_value);
}

// 设置电机步进角度
void qd4310_set_step_angle(qd4310_t *motor, float step_angle) 
{
    step_angle = qd4310_clamp(step_angle, QD4310_MIN_STEPANGLE, QD4310_MAX_STEPANGLE);
    int16_t step_angle_value = (int16_t)(step_angle / QD4310_MAX_STEPANGLE * INT16_MAX);
    qd4310_send_command(motor, QD4310_CMD_STEP_ANGLE, step_angle_value);
}

// 设置电机转速
void qd4310_set_speed(qd4310_t *motor, float speed) 
{
    speed = qd4310_clamp(speed, QD4310_MIN_SPEED, QD4310_MAX_SPEED);
    int16_t speed_value = (int16_t)(speed / QD4310_MAX_SPEED * INT16_MAX);
    qd4310_send_command(motor, QD4310_CMD_SPEED, speed_value);
}

// 设置电机低速
void qd4310_set_low_speed(qd4310_t *motor, float speed) 
{
    speed = qd4310_clamp(speed, QD4310_MIN_SPEED, QD4310_MAX_SPEED);
    int16_t speed_value = (int16_t)(speed / QD4310_MAX_SPEED * INT16_MAX);
    qd4310_send_command(motor, QD4310_CMD_LOW_SPEED, speed_value);
}

// 设置电机电流
void qd4310_set_current(qd4310_t *motor, float current) 
{
    current = qd4310_clamp(current, QD4310_MIN_CURRENT, QD4310_MAX_CURRENT);
    int16_t current_value = (int16_t)(current / QD4310_MAX_CURRENT * INT16_MAX);
    qd4310_send_command(motor, QD4310_CMD_CURRENT, current_value);
}

extern CAN_HandleTypeDef hcan1;

// void CAN_InterfaceInit(void) 
// {
//     CAN_FilterTypeDef sFilterConfig;
//     sFilterConfig.FilterBank = 0;
//     sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
//     sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
//     sFilterConfig.FilterIdLow = 0x0000;
//     sFilterConfig.FilterIdHigh = 0x0000;
//     sFilterConfig.FilterMaskIdLow = 0x0000;
//     sFilterConfig.FilterMaskIdHigh = 0x0000;
//     sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
//     sFilterConfig.FilterActivation = ENABLE;
//     sFilterConfig.SlaveStartFilterBank = 14;
//     if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) Error_Handler();
//     if (HAL_CAN_Start(&hcan1) != HAL_OK) Error_Handler();
//     if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) Error_Handler();
// }

// void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) 
// {
//     if (hcan == &hcan1) 
//     {
//         CAN_RxHeaderTypeDef rx_header;
//         uint8_t rx_data[8];
//         HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data);

//         if (rx_header.StdId >= 0x500 && rx_header.StdId <= 0x508)
//         {
//             if ((rx_header.StdId & 0xFF) == Motor_0.id) {
//                 qd4310_update(&Motor_0, rx_data);
//             }
//         }
//     }
// }