#ifndef __STEP_MOTOR_H
#define __STEP_MOTOR_H

#include "com.h"
#include "main.h"
#include "string.h"
#include "stdint.h"

/* ============================================================================
 * 步进电机（42BYGH34 搭配 A4988）
 * 该机构在调试阶段一直堵转，最终没有装车使用，
 * 代码保留只是为了不影响工程编译，正常比赛流程里不会走到这里。
 * 接线：STEP 接 PA0(TIM2_CH1)，DIR 接 PA1，ENABLE 接 PC10，
 *       MS1/MS2/MS3 接 PB8/PB9/PB4
 * ========================================================================== */

#define MOTOR_STEP_PORT     GPIOA
#define MOTOR_STEP_PIN      GPIO_PIN_0

#define MOTOR_DIR_PORT      GPIOA
#define MOTOR_DIR_PIN       GPIO_PIN_1

#define MOTOR_ENABLE_PORT   GPIOC
#define MOTOR_ENABLE_PIN    GPIO_PIN_10

#define MOTOR_MS1_PORT      GPIOB
#define MOTOR_MS1_PIN       GPIO_PIN_8

#define MOTOR_MS2_PORT      GPIOB
#define MOTOR_MS2_PIN       GPIO_PIN_9

#define MOTOR_MS3_PORT      GPIOB
#define MOTOR_MS3_PIN       GPIO_PIN_4

#define STEPS_PER_ROUND     200U

void StepMotor_Init(void);
void StepMotor_RotateAngle(float angle);
void StepMotor_Rotate120(uint8_t dir);

#endif
