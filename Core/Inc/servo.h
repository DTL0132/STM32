#ifndef __SERVO_H
#define __SERVO_H

#include "com.h"

/* ============================================================================
 * 舵机与升降
 * 两路舵机共用 TIM4，周期必须配成 20ms（50Hz）：
 *   TIM4_CH1 接 PB6   夹爪舵机，SERVO_OPEN 张开，SERVO_CLOSE 闭合
 *   TIM4_CH2 接 PB7   云台舵机，两个固定位分别在 10 度和 170 度
 * 脉宽换算：500 + 角度 * 2000 / 180，
 *           也就是 0 度对应 500us、180 度对应 2500us
 * 升降皮带电机经红板 TB6612 驱动：
 *   PWMA 接 PA11(TIM1_CH4)，AIN1 接 PA4，AIN2 接 PA5
 * ========================================================================== */

#define LIFT_IN1_PORT GPIOA
#define LIFT_IN1_PIN  GPIO_PIN_4

#define LIFT_IN2_PORT GPIOA
#define LIFT_IN2_PIN  GPIO_PIN_5

#define LIFT_SPEED    5000   /* 升降电机 PWM 比较值，约 50% 占空 */

#define SERVO_OPEN    0
#define SERVO_CLOSE   125

void Lift_Run(int dir);
void Gripper_SetAngle(uint32_t deg);
void Gripper_SetAngle1(uint32_t deg);

#endif
