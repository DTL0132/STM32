#include "step_motor.h"

extern TIM_HandleTypeDef htim2;

#define STEPS_120DEG    67U

/* 已发出的脉冲数，在定时器更新中断里累加 */
static volatile uint32_t s_count = 0;

/* 初始化：使能驱动、设置细分、启动 TIM2_CH1 的 PWM 但不输出脉冲 */
void StepMotor_Init(void)
{
    HAL_GPIO_WritePin(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(MOTOR_DIR_PORT, MOTOR_DIR_PIN, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(MOTOR_MS1_PORT, MOTOR_MS1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_MS2_PORT, MOTOR_MS2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_MS3_PORT, MOTOR_MS3_PIN, GPIO_PIN_RESET);

    __HAL_TIM_SET_AUTORELOAD(&htim2, 4000 - 1);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 500);

    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
}

/* 转动 120 度：dir 非 0 正转，0 反转；内部阻塞等待脉冲发完 */
void StepMotor_Rotate120(uint8_t dir)
{
    HAL_GPIO_WritePin(MOTOR_DIR_PORT, MOTOR_DIR_PIN,
                      dir ? GPIO_PIN_SET : GPIO_PIN_RESET);

    s_count = 0;
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 500);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

    while (s_count < STEPS_120DEG);
}

/* 定时器更新中断回调：每来一个脉冲计一次，够 67 个就停 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2) return;

    s_count++;

    if (s_count >= STEPS_120DEG) {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);
    }
}
