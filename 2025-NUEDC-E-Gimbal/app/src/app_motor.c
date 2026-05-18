#include "app_motor.h"
#include "qd4310.h"
#include "cmsis_os2.h"

extern CAN_HandleTypeDef hcan1;

static CAN_HandleTypeDef* hcan;

static qd4310_t motor0, motor1;

void CAN_InterfaceInit(CAN_HandleTypeDef* hcan) 
{
    if (hcan == NULL ) return;
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;
    if (HAL_CAN_ConfigFilter(hcan, &sFilterConfig) != HAL_OK) Error_Handler();
    if (HAL_CAN_Start(hcan) != HAL_OK) Error_Handler();
    if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) Error_Handler();
}

void motor_init(void)
{
    hcan = &hcan1;
    CAN_InterfaceInit(hcan);

    motor0.id = 0;
    motor0.hcan = hcan;
    
    motor1.id = 1;
    motor1.hcan = hcan;
}

void motor_irq_handler(CAN_HandleTypeDef *hcanx)
{
    if (hcanx == NULL) {
        return;
    }

    if (hcanx == hcan)
    {
        CAN_RxHeaderTypeDef rx_header;
        uint8_t rx_data[8];

        HAL_CAN_GetRxMessage(hcanx, CAN_RX_FIFO0, &rx_header, rx_data);

        if (rx_header.StdId >= 0x500 && rx_header.StdId <= 0x508)
        {
            if ((rx_header.StdId & 0xFF) == motor0.id) {
                qd4310_update(&motor0, rx_data);
            }

            if ((rx_header.StdId & 0xFF) == motor1.id) {
                qd4310_update(&motor1, rx_data);
            } 
        }
    }
}