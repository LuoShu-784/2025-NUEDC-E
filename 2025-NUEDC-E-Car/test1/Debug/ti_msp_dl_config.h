/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_0 */
#define PWM_0_INST                                                         TIMA1
#define PWM_0_INST_IRQHandler                                   TIMA1_IRQHandler
#define PWM_0_INST_INT_IRQN                                     (TIMA1_INT_IRQn)
#define PWM_0_INST_CLK_FREQ                                             16000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_0_C0_PORT                                                 GPIOB
#define GPIO_PWM_0_C0_PIN                                         DL_GPIO_PIN_26
#define GPIO_PWM_0_C0_IOMUX                                      (IOMUX_PINCM57)
#define GPIO_PWM_0_C0_IOMUX_FUNC                     IOMUX_PINCM57_PF_TIMA1_CCP0
#define GPIO_PWM_0_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_0_C1_PORT                                                 GPIOB
#define GPIO_PWM_0_C1_PIN                                         DL_GPIO_PIN_27
#define GPIO_PWM_0_C1_IOMUX                                      (IOMUX_PINCM58)
#define GPIO_PWM_0_C1_IOMUX_FUNC                     IOMUX_PINCM58_PF_TIMA1_CCP1
#define GPIO_PWM_0_C1_IDX                                    DL_TIMER_CC_1_INDEX



/* Defines for TIMER_0 */
#define TIMER_0_INST                                                     (TIMA0)
#define TIMER_0_INST_IRQHandler                                 TIMA0_IRQHandler
#define TIMER_0_INST_INT_IRQN                                   (TIMA0_INT_IRQn)
#define TIMER_0_INST_LOAD_VALUE                                           (399U)




/* Defines for OLED */
#define OLED_INST                                                           I2C0
#define OLED_INST_IRQHandler                                     I2C0_IRQHandler
#define OLED_INST_INT_IRQN                                         I2C0_INT_IRQn
#define OLED_BUS_SPEED_HZ                                                 100000
#define GPIO_OLED_SDA_PORT                                                 GPIOA
#define GPIO_OLED_SDA_PIN                                          DL_GPIO_PIN_0
#define GPIO_OLED_IOMUX_SDA                                       (IOMUX_PINCM1)
#define GPIO_OLED_IOMUX_SDA_FUNC                        IOMUX_PINCM1_PF_I2C0_SDA
#define GPIO_OLED_SCL_PORT                                                 GPIOA
#define GPIO_OLED_SCL_PIN                                          DL_GPIO_PIN_1
#define GPIO_OLED_IOMUX_SCL                                       (IOMUX_PINCM2)
#define GPIO_OLED_IOMUX_SCL_FUNC                        IOMUX_PINCM2_PF_I2C0_SCL


/* Defines for UART1 */
#define UART1_INST                                                         UART1
#define UART1_INST_FREQUENCY                                             4000000
#define UART1_INST_IRQHandler                                   UART1_IRQHandler
#define UART1_INST_INT_IRQN                                       UART1_INT_IRQn
#define GPIO_UART1_RX_PORT                                                 GPIOA
#define GPIO_UART1_TX_PORT                                                 GPIOA
#define GPIO_UART1_RX_PIN                                          DL_GPIO_PIN_9
#define GPIO_UART1_TX_PIN                                          DL_GPIO_PIN_8
#define GPIO_UART1_IOMUX_RX                                      (IOMUX_PINCM20)
#define GPIO_UART1_IOMUX_TX                                      (IOMUX_PINCM19)
#define GPIO_UART1_IOMUX_RX_FUNC                       IOMUX_PINCM20_PF_UART1_RX
#define GPIO_UART1_IOMUX_TX_FUNC                       IOMUX_PINCM19_PF_UART1_TX
#define UART1_BAUD_RATE                                                   (9600)
#define UART1_IBRD_4_MHZ_9600_BAUD                                          (26)
#define UART1_FBRD_4_MHZ_9600_BAUD                                           (3)





/* Port definition for Pin Group LED */
#define LED_PORT                                                         (GPIOB)

/* Defines for LED_B22: GPIOB.22 with pinCMx 50 on package pin 21 */
#define LED_LED_B22_PIN                                         (DL_GPIO_PIN_22)
#define LED_LED_B22_IOMUX                                        (IOMUX_PINCM50)
/* Port definition for Pin Group EECODER */
#define EECODER_PORT                                                     (GPIOA)

/* Defines for left_A: GPIOA.26 with pinCMx 59 on package pin 30 */
// pins affected by this interrupt request:["left_A","right_A"]
#define EECODER_INT_IRQN                                        (GPIOA_INT_IRQn)
#define EECODER_INT_IIDX                        (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define EECODER_left_A_IIDX                                 (DL_GPIO_IIDX_DIO26)
#define EECODER_left_A_PIN                                      (DL_GPIO_PIN_26)
#define EECODER_left_A_IOMUX                                     (IOMUX_PINCM59)
/* Defines for left_B: GPIOA.27 with pinCMx 60 on package pin 31 */
#define EECODER_left_B_PIN                                      (DL_GPIO_PIN_27)
#define EECODER_left_B_IOMUX                                     (IOMUX_PINCM60)
/* Defines for right_A: GPIOA.12 with pinCMx 34 on package pin 5 */
#define EECODER_right_A_IIDX                                (DL_GPIO_IIDX_DIO12)
#define EECODER_right_A_PIN                                     (DL_GPIO_PIN_12)
#define EECODER_right_A_IOMUX                                    (IOMUX_PINCM34)
/* Defines for right_B: GPIOA.13 with pinCMx 35 on package pin 6 */
#define EECODER_right_B_PIN                                     (DL_GPIO_PIN_13)
#define EECODER_right_B_IOMUX                                    (IOMUX_PINCM35)
/* Defines for AIN1: GPIOB.0 with pinCMx 12 on package pin 47 */
#define MOTOR_AIN1_PORT                                                  (GPIOB)
#define MOTOR_AIN1_PIN                                           (DL_GPIO_PIN_0)
#define MOTOR_AIN1_IOMUX                                         (IOMUX_PINCM12)
/* Defines for AIN2: GPIOB.1 with pinCMx 13 on package pin 48 */
#define MOTOR_AIN2_PORT                                                  (GPIOB)
#define MOTOR_AIN2_PIN                                           (DL_GPIO_PIN_1)
#define MOTOR_AIN2_IOMUX                                         (IOMUX_PINCM13)
/* Defines for AIN3: GPIOA.29 with pinCMx 4 on package pin 36 */
#define MOTOR_AIN3_PORT                                                  (GPIOA)
#define MOTOR_AIN3_PIN                                          (DL_GPIO_PIN_29)
#define MOTOR_AIN3_IOMUX                                          (IOMUX_PINCM4)
/* Defines for AIN4: GPIOA.30 with pinCMx 5 on package pin 37 */
#define MOTOR_AIN4_PORT                                                  (GPIOA)
#define MOTOR_AIN4_PIN                                          (DL_GPIO_PIN_30)
#define MOTOR_AIN4_IOMUX                                          (IOMUX_PINCM5)
/* Port definition for Pin Group road */
#define road_PORT                                                        (GPIOB)

/* Defines for left33: GPIOB.4 with pinCMx 17 on package pin 52 */
#define road_left33_PIN                                          (DL_GPIO_PIN_4)
#define road_left33_IOMUX                                        (IOMUX_PINCM17)
/* Defines for left22: GPIOB.5 with pinCMx 18 on package pin 53 */
#define road_left22_PIN                                          (DL_GPIO_PIN_5)
#define road_left22_IOMUX                                        (IOMUX_PINCM18)
/* Defines for left11: GPIOB.6 with pinCMx 23 on package pin 58 */
#define road_left11_PIN                                          (DL_GPIO_PIN_6)
#define road_left11_IOMUX                                        (IOMUX_PINCM23)
/* Defines for center: GPIOB.7 with pinCMx 24 on package pin 59 */
#define road_center_PIN                                          (DL_GPIO_PIN_7)
#define road_center_IOMUX                                        (IOMUX_PINCM24)
/* Defines for right11: GPIOB.8 with pinCMx 25 on package pin 60 */
#define road_right11_PIN                                         (DL_GPIO_PIN_8)
#define road_right11_IOMUX                                       (IOMUX_PINCM25)
/* Defines for right22: GPIOB.9 with pinCMx 26 on package pin 61 */
#define road_right22_PIN                                         (DL_GPIO_PIN_9)
#define road_right22_IOMUX                                       (IOMUX_PINCM26)
/* Defines for right33: GPIOB.23 with pinCMx 51 on package pin 22 */
#define road_right33_PIN                                        (DL_GPIO_PIN_23)
#define road_right33_IOMUX                                       (IOMUX_PINCM51)
/* Defines for key1: GPIOA.31 with pinCMx 6 on package pin 39 */
#define key_key1_PORT                                                    (GPIOA)
#define key_key1_PIN                                            (DL_GPIO_PIN_31)
#define key_key1_IOMUX                                            (IOMUX_PINCM6)
/* Defines for key2: GPIOA.28 with pinCMx 3 on package pin 35 */
#define key_key2_PORT                                                    (GPIOA)
#define key_key2_PIN                                            (DL_GPIO_PIN_28)
#define key_key2_IOMUX                                            (IOMUX_PINCM3)
/* Defines for key3: GPIOB.12 with pinCMx 29 on package pin 64 */
#define key_key3_PORT                                                    (GPIOB)
#define key_key3_PIN                                            (DL_GPIO_PIN_12)
#define key_key3_IOMUX                                           (IOMUX_PINCM29)
/* Defines for key4: GPIOB.13 with pinCMx 30 on package pin 1 */
#define key_key4_PORT                                                    (GPIOB)
#define key_key4_PIN                                            (DL_GPIO_PIN_13)
#define key_key4_IOMUX                                           (IOMUX_PINCM30)
/* Defines for key5: GPIOA.7 with pinCMx 14 on package pin 49 */
#define key_key5_PORT                                                    (GPIOA)
#define key_key5_PIN                                             (DL_GPIO_PIN_7)
#define key_key5_IOMUX                                           (IOMUX_PINCM14)
/* Defines for key6: GPIOB.14 with pinCMx 31 on package pin 2 */
#define key_key6_PORT                                                    (GPIOB)
#define key_key6_PIN                                            (DL_GPIO_PIN_14)
#define key_key6_IOMUX                                           (IOMUX_PINCM31)
/* Defines for begin_key: GPIOB.21 with pinCMx 49 on package pin 20 */
#define key_begin_key_PORT                                               (GPIOB)
#define key_begin_key_PIN                                       (DL_GPIO_PIN_21)
#define key_begin_key_IOMUX                                      (IOMUX_PINCM49)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_0_init(void);
void SYSCFG_DL_TIMER_0_init(void);
void SYSCFG_DL_OLED_init(void);
void SYSCFG_DL_UART1_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
