#ifndef __CSB_H
#define __CSB_H

#include "com.h"

/* ============================================================================
 * 超声波测距配置（共三路，装在车头一字排开）
 *   左前：TRIG PC8 / ECHO PC9
 *   正前：TRIG PC7 / ECHO PC5
 *   右前：TRIG PC6 / ECHO PC4
 * 注意：ECHO 是 5V 输出，直接进 3.3V 的 IO 会经保护二极管倒灌电流，
 *       长期会让主控发热，建议在 ECHO 线上串 1k 电阻。
 * ========================================================================== */

#define TRIG_L_PORT     GPIOC
#define TRIG_L_PIN      GPIO_PIN_8
#define ECHO_L_PORT     GPIOC
#define ECHO_L_PIN      GPIO_PIN_9

#define TRIG_C_PORT     GPIOC
#define TRIG_C_PIN      GPIO_PIN_7
#define ECHO_C_PORT     GPIOC
#define ECHO_C_PIN      GPIO_PIN_5

#define TRIG_R_PORT     GPIOC
#define TRIG_R_PIN      GPIO_PIN_6
#define ECHO_R_PORT     GPIOC
#define ECHO_R_PIN      GPIO_PIN_4

#define OBSTACLE_CM     25.0f   /* 小于这个距离就算障碍物 */
#define CLEAR_CM        35.0f   /* 大于这个距离就算已经躲开 */
#define ECHO_MIN_T      100     /* 回波太短视为无效，过滤杂波 */

void DWT_Init(void);
void delay_us(uint32_t us);

float HCSR04_Read(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                  GPIO_TypeDef *echo_port, uint16_t echo_pin);

float HCSR04_Left(void);
float HCSR04_Center(void);
float HCSR04_Right(void);

#endif
