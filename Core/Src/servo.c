#include "servo.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;

/* ----------------------------------------------------------------------------
 * 升降皮带电机控制
 * dir > 0 上升，dir < 0 下降，dir == 0 停止
 * 停止时把两个方向脚都拉高，让电机快速刹住
 * -------------------------------------------------------------------------- */
void Lift_Run(int dir)
{
    if (dir > 0)
    {
        HAL_GPIO_WritePin(LIFT_IN1_PORT, LIFT_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LIFT_IN2_PORT, LIFT_IN2_PIN, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, LIFT_SPEED);
    }
    else if (dir < 0)
    {
        HAL_GPIO_WritePin(LIFT_IN1_PORT, LIFT_IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LIFT_IN2_PORT, LIFT_IN2_PIN, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, LIFT_SPEED);
    }
    else
    {
        HAL_GPIO_WritePin(LIFT_IN1_PORT, LIFT_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LIFT_IN2_PORT, LIFT_IN2_PIN, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
    }
}

/* 夹爪舵机：TIM4_CH1，传入 0 到 180 度 */
void Gripper_SetAngle(uint32_t deg)
{
    if (deg > 180)
    {
        deg = 180;
    }

    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 500 + deg * 2000 / 180);
}

/* 云台舵机：TIM4_CH2，传入 0 到 180 度 */
void Gripper_SetAngle1(uint32_t deg)
{
    if (deg > 180)
    {
        deg = 180;
    }

    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 500 + deg * 2000 / 180);
}
