#ifndef __MOTOR_H
#define __MOTOR_H

#include "com.h"

/* ============================================================================
 * 底盘电机配置
 * 四路驱动板与车轮的对应关系（和接线文档一致）：
 *   0 号 前左轮  接 B 区   PWM 用 PA7(TIM3_CH2)   方向用 PB14 / PB15
 *   1 号 前右轮  接 A 区   PWM 用 PA6(TIM3_CH1)   方向用 PB12 / PB13
 *   2 号 后左轮  接 D 区   PWM 用 PB1(TIM3_CH4)   方向用 PC2  / PC3
 *   3 号 后右轮  接 C 区   PWM 用 PB0(TIM3_CH3)   方向用 PC0  / PC1
 * 速度说明：PWM_MAX 是 7199，所以写 7200 会被限幅，效果等于满速。
 * ========================================================================== */

#define FL_IN1_PORT   GPIOB
#define FL_IN1_PIN    GPIO_PIN_14
#define FL_IN2_PORT   GPIOB
#define FL_IN2_PIN    GPIO_PIN_15

#define FR_IN1_PORT   GPIOB
#define FR_IN1_PIN    GPIO_PIN_12
#define FR_IN2_PORT   GPIOB
#define FR_IN2_PIN    GPIO_PIN_13

#define BL_IN1_PORT   GPIOC
#define BL_IN1_PIN    GPIO_PIN_2
#define BL_IN2_PORT   GPIOC
#define BL_IN2_PIN    GPIO_PIN_3

#define BR_IN1_PORT   GPIOC
#define BR_IN1_PIN    GPIO_PIN_0
#define BR_IN2_PORT   GPIOC
#define BR_IN2_PIN    GPIO_PIN_1

#define STBY_PORT     GPIOA
#define STBY_PIN      GPIO_PIN_12
#define PWM_MAX       7199

#define SPD_FL        7200
#define SPD_FR        7200
#define SPD_BL        7200
#define SPD_BR        7200

#define SPD_TURN      7200      /* 原地转弯速度 */
#define SPD_STRAFE    7200      /* 手动平移速度 */

#define SPD_AVOID     7200      /* 避障时前进速度 */
#define SPD_SLOW      3600      /* 慢速档，慢速平移与慢速转弯都用它 */

/* 底层接口：m 是电机编号 0 到 3，pwm 正负表示正反转，范围正负 PWM_MAX */
void motor_run(uint8_t m, int16_t pwm);
/* 底盘整车动作封装：每个函数内部把四路电机一起设置好 */
void Car_Stop(void);
void Car_Forward(void);
void Car_TurnLeft(void);
void Car_Backward(void);
void Car_StrafeLeft(void);
void Car_StrafeRight(void);
void Car_TurnRight(void);
void Car_Forward_Avoid(void);
void Car_Forward_Slow(void);
void Car_Backward_Slow(void);
void Car_TurnLeft_Slow(void);
void Car_TurnRight_Slow(void);
void Car_StrafeLeft_Slow(void);
void Car_StrafeRight_Slow(void);
#endif
