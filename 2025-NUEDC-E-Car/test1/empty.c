// #include "ti_msp_dl_config.h"
// #include "delay.h"
// #include "oled.h"
// #include "motor.h"
// #include <stdio.h>
// #include "key.h"
// #include "uart.h"

// int base_speed = 110;
// char display_str[20];
// extern PID_TypeDef pid_left;
// extern PID_TypeDef pid_right;
// extern volatile int32_t left_encoder_count;
// extern volatile int32_t right_encoder_count;
// extern volatile int current_speed_L;
// extern volatile int current_speed_R;
// extern volatile int target_speed_L;
// extern volatile int target_speed_R;


// uint8_t key_raw = 0;
// uint8_t key_down = 0;
// uint8_t key_old = 0;
// volatile uint32_t sys_tick = 0; 


// uint8_t begin_key_raw = 0;
// uint8_t begin_key_down = 0;
// uint8_t begin_key_old = 0;


// uint8_t current_mode = 0;     
// uint8_t selected_laps = 0;  
// uint8_t selected_point = 0;   
// uint8_t is_running = 0;        


// volatile uint32_t left_turn_count = 0;   
// volatile uint32_t target_left_turns = 0; 

// typedef struct {
//     int L3, L2, L1, C, R1, R2, R3;
// } SensorState;

// SensorState Read_Sensors(void) {
//     SensorState s;
//     s.L3 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_4) == 0) ? 1 : 0;
//     s.L2 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_5) == 0) ? 1 : 0;
//     s.L1 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_6) == 0) ? 1 : 0;
//     s.C  = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_7) == 0) ? 1 : 0;
//     s.R1 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_8) == 0) ? 1 : 0;
//     s.R2 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_9) == 0) ? 1 : 0;
//     s.R3 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_23)== 0) ? 1 : 0;
//     return s;
// }

// typedef enum {
//     STATE_TRACKING,           
//     STATE_TURN_LEFT_BLIND,    
//     STATE_TURN_LEFT_ROTATE   
// } CarState;

// volatile CarState current_state = STATE_TRACKING;
// volatile uint32_t turn_start_time = 0; 
// volatile int last_error = 0;

// void UART_SendCmd(uint8_t cmd, uint8_t data) {
//     uart1_send_char(0xCC);  // 帧头
//     uart1_send_char(cmd);   // 命令字
//     uart1_send_char(data);  // 数据位
//     uart1_send_char(0xDD);  // 帧尾
// }

// void Track_Task(void) {
//     SensorState s = Read_Sensors();

//     switch (current_state) {
//         case STATE_TRACKING:
//             if (s.L3 == 1 && s.R3 == 0) { 
//                 current_state = STATE_TURN_LEFT_BLIND;
//                 turn_start_time = sys_tick; 
//                 Car_SetSpeed(base_speed, base_speed); 
//             } 
//             else {
//                 int active_count = s.L2 + s.L1 + s.C + s.R1 + s.R2 + s.R3; 
//                 int error = 0;
//                 if (active_count > 0) {

//                     int sum = (s.L2 * -2) + (s.L1 * -1) + (s.C * 0) + (s.R1 * 1) + (s.R2 * 2) + (s.R3 * 3);
//                     error = (sum * 4) / active_count; 
//                 } else {
//                     error = last_error; 
//                 }
                
//                 int turn_adjust = (error * 1); 
//                 last_error = error;
                
//                 int left_speed  = base_speed + turn_adjust;
//                 int right_speed = base_speed - turn_adjust;
//                 Car_SetSpeed(left_speed, right_speed);
//             }
//             break;

//         case STATE_TURN_LEFT_BLIND:
//             if ((sys_tick - turn_start_time) >= 16) {
//                 current_state = STATE_TURN_LEFT_ROTATE;
//                 turn_start_time = sys_tick; 
//                 Car_SetSpeed(0, 100); 
//             }
//             break;

//         case STATE_TURN_LEFT_ROTATE:
//             if ((sys_tick - turn_start_time) > 10) {

//                 if (s.C == 1 || s.L1 == 1 || s.R1 == 1) { 
//                     current_state = STATE_TRACKING; 
//                     last_error = 0;

//                     left_turn_count++;

//                     if (target_left_turns > 0 && left_turn_count >= target_left_turns) {
//                         is_running = 0;       // 自动停止
//                         current_mode = 0;     // 跑完自动回到主菜单
//                         selected_laps = 0;    // 清空上次设置
//                         selected_point = 0;
//                         Car_SetSpeed(0, 0);   // 强制停车
//                     }
//                 }
//             }
//             break;
//     }
// }


// void TIMA0_IRQHandler(void) {
//     static int speed_loop_cnt = 0; 
//     switch (DL_TimerA_getPendingInterrupt(TIMER_0_INST)) {
//         case DL_TIMER_IIDX_ZERO: 
//             sys_tick++; 
            
//             if (is_running) {
//                 Track_Task(); 
//             } else {
//                 Car_SetSpeed(0, 0); 
//             }
            
//             speed_loop_cnt++;
//             if (speed_loop_cnt >= 5) {
//                 speed_loop_cnt = 0;
//                 Motor_Control_Loop(); 
//             }
//             break;
//         default: break;
//     }
// }

// int main(void)
// {
//     SYSCFG_DL_init();
//     NVIC_EnableIRQ(EECODER_INT_IRQN);
//     NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
//     NVIC_ClearPendingIRQ(UART1_INST_INT_IRQN);
//     NVIC_EnableIRQ(UART1_INST_INT_IRQN);

//     OLED_Init();
//     OLED_Clear();
//     OLED_ShowString(0, 0, (u8*)"System Ready", 16);
//     OLED_Refresh();
//     delay_ms(1000); 

//     int display_count = 0;
  
//     while (1) {

//         key_raw = KEY_Scan();
//         key_down = (key_raw != key_old && key_raw != 0) ? key_raw : 0;
//         key_old = key_raw;
//         begin_key_raw = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_21) == 0) ? 1 : 0;
//         begin_key_down = begin_key_raw & (begin_key_raw ^ begin_key_old);
//         begin_key_old = begin_key_raw;
//         if (!is_running) {

//             if (current_mode == 0) {
//                 if (key_down == 1) {
//                     current_mode = 1; selected_laps = 0; selected_point = 0;
//                     UART_SendCmd(0xC1, 0x00);
//                 } else if (key_down == 2) {
//                     current_mode = 2; selected_laps = 0; selected_point = 0;
//                 } else if (key_down == 3) {
//                     current_mode = 3; selected_laps = 1; // 模式3固定1圈
//                     UART_SendCmd(0xC3, 0x00);
//                 } else if (key_down == 4) {
//                     current_mode = 4; selected_laps = 2; // 模式4固定2圈
//                     UART_SendCmd(0xC4, 0x00);
//                 } else if (key_down == 5) {
//                     current_mode = 5; selected_laps = 1; // 模式5固定1圈
//                     UART_SendCmd(0xC5, 0x00);
//                 }
//             } 

//             else if (current_mode == 1) {
//                 if (key_down >= 2 && key_down <= 6) {
//                     selected_laps = key_down - 1; // 按键2->1圈，按键6->5圈
//                 }
//             }
//             else if (current_mode == 2) {
//                 if (key_down >= 3 && key_down <= 6) {
//                     selected_point = key_down - 2; // 按键3->点1，按键6->点4
//                     UART_SendCmd(0xC2, selected_point);
//                 }
//             }
            
//             if (begin_key_down && current_mode != 0) {
                

//                 uart1_send_char(0x55); 
//                 uart1_send_char(0x55);

//                 if (current_mode == 1)      target_left_turns = selected_laps * 4;
//                 else if (current_mode == 3) target_left_turns = 1 * 4;
//                 else if (current_mode == 4) target_left_turns = 2 * 4;
//                 else if (current_mode == 5) target_left_turns = 1 * 4;
//                 else if (current_mode == 2) target_left_turns = 0;
 
//                 if (target_left_turns > 0) {
//                     left_turn_count = 0;            
//                     current_state = STATE_TRACKING;  
//                     is_running = 1;                  
//                 } else {
//                     is_running = 0;              
//                     current_mode = 0;               
//                     selected_laps = 0;           
//                     selected_point = 0;
//                     Car_SetSpeed(0, 0);             
//                 }
//             }
//         }

//         display_count++;
//         if (display_count >= 2) { 
//             display_count = 0;
//             OLED_Clear();
//             if (current_mode == 0) {
//                 sprintf(display_str, "-Select Mode-");
//             } else {
//                 sprintf(display_str, "Work Mode: %d", current_mode);
//             }
//             OLED_ShowString(0, 0, (u8*)display_str, 16);
        
//             if (current_mode == 1 || current_mode >= 3) {
//                 sprintf(display_str, "Set Laps: %d  ", selected_laps);
//             } else if (current_mode == 2) {
//                 sprintf(display_str, "Set Point: %d ", selected_point);
//             } else {
//                 sprintf(display_str, "             ");
//             }
//             OLED_ShowString(0, 16, (u8*)display_str, 16);
   
//             sprintf(display_str, "Turns: %d / %d", left_turn_count, target_left_turns);
//             OLED_ShowString(0, 32, (u8*)display_str, 16);
            
//             if (is_running) {
//                 sprintf(display_str, "Status: RUN  ");
//             } else {
//                 sprintf(display_str, "Status: WAIT ");
//             }
//             OLED_ShowString(0, 48, (u8*)display_str, 16);
            
//             OLED_Refresh();
//         }
//         delay_ms(10); 
//     }
// }
#include "ti_msp_dl_config.h"
#include "delay.h"
#include "oled.h"
#include "motor.h"
#include <stdio.h>
#include "key.h"
#include "uart.h"

int base_speed = 30;
char display_str[20];

extern PID_TypeDef pid_left;
extern PID_TypeDef pid_right;
extern volatile int32_t left_encoder_count;
extern volatile int32_t right_encoder_count;
extern volatile int current_speed_L;
extern volatile int current_speed_R;
extern volatile int target_speed_L;
extern volatile int target_speed_R;

uint8_t key_raw = 0;
uint8_t key_down = 0;
uint8_t key_old = 0;
volatile uint32_t sys_tick = 0; 

uint8_t begin_key_raw = 0;
uint8_t begin_key_down = 0;
uint8_t begin_key_old = 0;


uint8_t current_mode = 0;       
uint8_t selected_laps = 0;      
uint8_t selected_point = 0;     
uint8_t is_running = 0;         

volatile uint32_t left_turn_count = 0;   
volatile uint32_t target_left_turns = 0; 

typedef struct {
    int L3, L2, L1, C, R1, R2, R3;
} SensorState;

SensorState Read_Sensors(void) {
    SensorState s;
    s.L3 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_4) == 0) ? 1 : 0;
    s.L2 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_5) == 0) ? 1 : 0;
    s.L1 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_6) == 0) ? 1 : 0;
    s.C  = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_7) == 0) ? 1 : 0;
    s.R1 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_8) == 0) ? 1 : 0;
    s.R2 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_9) == 0) ? 1 : 0;
    s.R3 = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_23)== 0) ? 1 : 0;
    return s;
}

typedef enum {
    STATE_TRACKING,           
    STATE_TURN_LEFT_BLIND,    
    STATE_TURN_LEFT_ROTATE   
} CarState;

volatile CarState current_state = STATE_TRACKING;
volatile uint32_t turn_start_time = 0; 
volatile int last_error = 0;
void UART_SendCmd(uint8_t cmd, uint8_t data) {
    uart1_send_char(0xCC); 
    uart1_send_char(cmd);  
    uart1_send_char(data); 
    uart1_send_char(0xDD); 
}

void Track_Task(void) {
    SensorState s = Read_Sensors();

    switch (current_state) {
        case STATE_TRACKING:
            if (s.L3 == 1 && s.R3 == 0) { 
                current_state = STATE_TURN_LEFT_BLIND;
                turn_start_time = sys_tick; 
                Car_SetSpeed(base_speed, base_speed); 
            } 
            else {
                int active_count = s.L2 + s.L1 + s.C + s.R1 + s.R2 + s.R3;
                int error = 0;
                if (active_count > 0) {
                    int sum = (s.L2 * -2) + (s.L1 * -1) + (s.C * 0) + (s.R1 * 1) + (s.R2 * 2) + (s.R3 * 4);
                    error = (sum * 2) / active_count; 
                } else {
                    error = last_error; 
                }
                
                int turn_adjust = (error * 1); 
                last_error = error;
                
                int left_speed  = base_speed + turn_adjust;
                int right_speed = base_speed - turn_adjust;
                Car_SetSpeed(left_speed, right_speed);
            }
            break;

        case STATE_TURN_LEFT_BLIND:
            if ((sys_tick - turn_start_time) >= 24) {
                current_state = STATE_TURN_LEFT_ROTATE;
                turn_start_time = sys_tick; 
                Car_SetSpeed(-1, 30); 
            }
            break;

        case STATE_TURN_LEFT_ROTATE:
            // if ((sys_tick - turn_start_time) > 1) {
                if (s.C == 1 || s.L1 == 1 || s.R1 == 1||s.R2 == 1||s.R3 == 1||s.L2 == 1 ) { 
                    current_state = STATE_TRACKING; 
                    last_error = 0;
                    
                    left_turn_count++;
                    
                    if (target_left_turns > 0 && left_turn_count >= target_left_turns) {
                        is_running = 0;       
                        current_mode = 0;     
                        selected_laps = 0;    
                        selected_point = 0;
                        Car_SetSpeed(0, 0);   
                    }
                }
            // }
            break;
    }
}

void TIMA0_IRQHandler(void) {
    static int speed_loop_cnt = 0; 
    switch (DL_TimerA_getPendingInterrupt(TIMER_0_INST)) {
        case DL_TIMER_IIDX_ZERO: 
            sys_tick++; 
            
            if (is_running) {
                Track_Task(); 
            } else {
                Car_SetSpeed(0, 0); 
            }
            
            speed_loop_cnt++;
            if (speed_loop_cnt >= 3) {
                speed_loop_cnt = 0;
                Motor_Control_Loop(); 
            }
            break;
        default: break;
    }
}

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(EECODER_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(UART1_INST_INT_IRQN);
    NVIC_EnableIRQ(UART1_INST_INT_IRQN);

    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, (u8*)"System Ready", 16);
    OLED_Refresh();
    delay_ms(1000); 

    int display_count = 0;
  
    while (1) {
        key_raw = KEY_Scan();
        key_down = (key_raw != key_old && key_raw != 0) ? key_raw : 0;
        key_old = key_raw;

        begin_key_raw = (DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_21) == 0) ? 1 : 0;
        begin_key_down = begin_key_raw & (begin_key_raw ^ begin_key_old);
        begin_key_old = begin_key_raw;

        if (!is_running) {
            if (current_mode == 0) {
                if (key_down == 1) { current_mode = 1; selected_laps = 0; selected_point = 0; UART_SendCmd(0xC1, 0x00); } 
                else if (key_down == 2) { current_mode = 2; selected_laps = 0; selected_point = 0; } 
                else if (key_down == 3) { current_mode = 3; selected_laps = 1; UART_SendCmd(0xC3, 0x00); } 
                else if (key_down == 4) { current_mode = 4; selected_laps = 2; UART_SendCmd(0xC4, 0x00); } 
                else if (key_down == 5) { current_mode = 5; selected_laps = 1; UART_SendCmd(0xC5, 0x00); }
            } 
            else if (current_mode == 1) {
                if (key_down >= 2 && key_down <= 6) { selected_laps = key_down - 1; }
            }
            else if (current_mode == 2) {
                if (key_down >= 3 && key_down <= 6) { selected_point = key_down - 2; UART_SendCmd(0xC2, selected_point); }
            }
            
            if (begin_key_down && current_mode != 0) {
                uart1_send_char(0x55); 
                uart1_send_char(0x55);
                
                if (current_mode == 1)      target_left_turns = selected_laps * 4;
                else if (current_mode == 3) target_left_turns = 1 * 4;
                else if (current_mode == 4) target_left_turns = 2 * 4;
                else if (current_mode == 5) target_left_turns = 1 * 4;
                else if (current_mode == 2) target_left_turns = 0; 
                
                if (target_left_turns > 0) {
                    left_turn_count = 0;             
                    current_state = STATE_TRACKING;  
                    is_running = 1;                  
                } else {
                    is_running = 0;                  
                    current_mode = 0;                
                    selected_laps = 0;               
                    selected_point = 0;
                    Car_SetSpeed(0, 0);              
                }
            }
        }

        display_count++;
        if (display_count >= 2) { 
            display_count = 0;
            OLED_Clear();
            if(is_running) sprintf(display_str, "Mode:%d  [RUN]", current_mode);
            else sprintf(display_str, "Mode:%d  [STOP]", current_mode);
            OLED_ShowString(0, 0, (u8*)display_str, 16);

            sprintf(display_str, "Tar: L%-4d R%-4d", target_speed_L, target_speed_R);
            OLED_ShowString(0, 16, (u8*)display_str, 16);

            sprintf(display_str, "Cur: L%-4d R%-4d", current_speed_L, current_speed_R);
            OLED_ShowString(0, 32, (u8*)display_str, 16);

            if (target_left_turns > 0) {
                sprintf(display_str, "Turns: %d / %d", left_turn_count, target_left_turns);
            } else {
                sprintf(display_str, "Set: Lap%d Pt%d", selected_laps, selected_point);
            }
            OLED_ShowString(0, 48, (u8*)display_str, 16);
            
            OLED_Refresh();
        }
        // delay_ms(10); 
    }
}