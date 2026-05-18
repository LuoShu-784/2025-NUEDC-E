#include "app_motor.h"
#include "qd4310.h"
#include "cmsis_os2.h"

#include <string.h>

extern CAN_HandleTypeDef hcan1;

#define MOTOR_RAD_TO_DEG  (180.0f / QD4310_PI)
#define MOTOR_DEG_TO_RAD  (QD4310_PI / 180.0f)

static CAN_HandleTypeDef *hcan;
static qd4310_t motor0;
static qd4310_t motor1;

static inline float motor_rad_to_deg(float rad)
{
    return rad * MOTOR_RAD_TO_DEG;
}

static inline float motor_deg_to_rad(float deg)
{
    return deg * MOTOR_DEG_TO_RAD;
}

static inline qd4310_t *motor_get_handle(uint8_t id)
{
    if (id == 0U) {
        return &motor0;
    }

    if (id == 1U) {
        return &motor1;
    }

    return NULL;
}

static void can_interface_init(CAN_HandleTypeDef *handle)
{
    if (handle == NULL) {
        return;
    }

    CAN_FilterTypeDef sFilterConfig = {0};
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

    if (HAL_CAN_ConfigFilter(handle, &sFilterConfig) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_CAN_Start(handle) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_CAN_ActivateNotification(handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        Error_Handler();
    }
}

void motor_irq_handler(CAN_HandleTypeDef *hcanx)
{
    if ((hcanx == NULL) || (hcanx != hcan)) {
        return;
    }

    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8] = {0};

    if (HAL_CAN_GetRxMessage(hcanx, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        return;
    }

    if ((rx_header.StdId < 0x500U) || (rx_header.StdId > 0x508U)) {
        return;
    }

    qd4310_t *motor = motor_get_handle((uint8_t)(rx_header.StdId & 0xFFU));
    if (motor != NULL) {
        qd4310_update(motor, rx_data);
    }
}

void app_motor_init(void)
{
    hcan = &hcan1;

    motor0 = (qd4310_t){0};
    motor1 = (qd4310_t){0};

    motor0.id = 0U;
    motor0.hcan = hcan;

    motor1.id = 1U;
    motor1.hcan = hcan;

    can_interface_init(hcan);
    app_motor_disable(0);
    app_motor_disable(1);
}

void app_motor_get_data(uint8_t id, motor_data_t *data)
{
    if (data == NULL) {
        return;
    }

    qd4310_t *motor = motor_get_handle(id);
    if (motor == NULL) {
        memset(data, 0, sizeof(*data));
        return;
    }

    data->enabled = motor->enabled;
    data->speed = motor->speed;
    data->angle = motor_rad_to_deg(motor->angle);
    data->current = motor->current;
}

void app_motor_enable(uint8_t id)
{
    qd4310_t *motor = motor_get_handle(id);
    if (motor == NULL) {
        return;
    }

    motor->enabled = true;
    qd4310_enable(motor);
}

void app_motor_disable(uint8_t id)
{
    qd4310_t *motor = motor_get_handle(id);
    if (motor == NULL) {
        return;
    }

    motor->enabled = false;
    qd4310_disable(motor);
}

void app_motor_set_speed(uint8_t id, float speed)
{
    qd4310_t *motor = motor_get_handle(id);
    if (motor == NULL) {
        return;
    }

    qd4310_set_speed(motor, speed);
}

void app_motor_set_angle(uint8_t id, float angle_deg)
{
    qd4310_t *motor = motor_get_handle(id);
    if (motor == NULL) {
        return;
    }

    qd4310_set_angle(motor, motor_deg_to_rad(angle_deg));
}
