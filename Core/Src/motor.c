#include "motor.h"

extern TIM_HandleTypeDef htim3;

/* ----------------------------------------------------------------------------
 * 单路电机底层驱动
 * m   : 电机编号 0 前左 / 1 前右 / 2 后左 / 3 后右
 * pwm : 正数正转、负数反转、0 滑行停止，范围正负 PWM_MAX
 * 方向由两个方向脚决定，速度由对应 PWM 通道的比较值决定。
 * -------------------------------------------------------------------------- */
void motor_run(uint8_t m, int16_t pwm)
{
    if (pwm > PWM_MAX) pwm = PWM_MAX;
    if (pwm < -PWM_MAX) pwm = -PWM_MAX;

    GPIO_TypeDef *in1_port, *in2_port;
    uint16_t in1_pin, in2_pin;
    uint32_t channel;

    switch (m) {
    case 0:
    in1_port = FL_IN1_PORT; in1_pin = FL_IN1_PIN;
    in2_port = FL_IN2_PORT; in2_pin = FL_IN2_PIN;
    channel = TIM_CHANNEL_2;
    break;

    case 1:
    in1_port = FR_IN1_PORT; in1_pin = FR_IN1_PIN;
    in2_port = FR_IN2_PORT; in2_pin = FR_IN2_PIN;
    channel = TIM_CHANNEL_1;
    break;

    case 2:
    in1_port = BL_IN1_PORT; in1_pin = BL_IN1_PIN;
    in2_port = BL_IN2_PORT; in2_pin = BL_IN2_PIN;
    channel = TIM_CHANNEL_4;
    break;

    case 3:
    in1_port = BR_IN1_PORT; in1_pin = BR_IN1_PIN;
    in2_port = BR_IN2_PORT; in2_pin = BR_IN2_PIN;
    channel = TIM_CHANNEL_3;
    break;
    default:
        return;
    }
		
    /* 两个前轮是反向装在车上的，这里统一取反，
       让上层调用不用再关心前后轮的区别 */
		if (m == 0 || m == 1)
{
    pwm = -pwm;
}

    if (pwm > 0) {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
    } else if (pwm < 0) {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(in1_port, in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(in2_port, in2_pin, GPIO_PIN_RESET);
    }

    uint32_t duty = (pwm < 0) ? (uint32_t)(-pwm) : (uint32_t)pwm;
    __HAL_TIM_SET_COMPARE(&htim3, channel, duty);
		
}

/* 四轮同时停止（方向脚全拉低，电机滑行） */
void Car_Stop(void)
{
    motor_run(0, 0); motor_run(1, 0);
    motor_run(2, 0); motor_run(3, 0);
}

/* 直行：四轮同向，速度取满速 */
void Car_Forward(void)
{
    motor_run(0, SPD_FL);
    motor_run(1, SPD_FR);
    motor_run(2, SPD_BL);
    motor_run(3, SPD_BR);
}

/* 原地左转：左右两侧电机反向 */
void Car_TurnLeft(void)
{
    motor_run(0,  SPD_TURN);
    motor_run(1, -SPD_TURN);
    motor_run(2, -SPD_TURN);
    motor_run(3,  SPD_TURN);
}

/* 后退：四轮反方向 */
void Car_Backward(void)
{
    motor_run(0, -SPD_FL);
    motor_run(1, -SPD_FR);
    motor_run(2, -SPD_BL);
    motor_run(3, -SPD_BR);
}

/* 向左平移：对角轮反向，麦轮的横移动作 */
void Car_StrafeLeft(void)
{
    motor_run(0, -SPD_STRAFE);
    motor_run(1,  SPD_STRAFE);
    motor_run(2, -SPD_STRAFE);
    motor_run(3,  SPD_STRAFE);
}

/* 向右平移：与左移相反 */
void Car_StrafeRight(void)
{
    motor_run(0,  SPD_STRAFE);
    motor_run(1, -SPD_STRAFE);
    motor_run(2,  SPD_STRAFE);
    motor_run(3, -SPD_STRAFE);
}

/* 原地右转：与左转相反 */
void Car_TurnRight(void)
{
    motor_run(0, -SPD_TURN);
    motor_run(1,  SPD_TURN);
    motor_run(2,  SPD_TURN);
    motor_run(3, -SPD_TURN);
}

/* 避障时用的前进，速度单独留出来，方便以后单独调 */
void Car_Forward_Avoid(void)
{
    motor_run(0, SPD_AVOID);
    motor_run(1, SPD_AVOID);
    motor_run(2, SPD_AVOID);
    motor_run(3, SPD_AVOID);
}

/* 慢速前进：用于平地微调对位 */
void Car_Forward_Slow(void)
{
    motor_run(0, SPD_SLOW);
    motor_run(1, SPD_SLOW);
    motor_run(2, SPD_SLOW);
    motor_run(3, SPD_SLOW);
}

/* 慢速后退：与慢速前进相反 */
void Car_Backward_Slow(void)
{
    motor_run(0, -SPD_SLOW);
    motor_run(1, -SPD_SLOW);
    motor_run(2, -SPD_SLOW);
    motor_run(3, -SPD_SLOW);
}

/* 慢速原地左转 */
void Car_TurnLeft_Slow(void)
{
    motor_run(0,  SPD_SLOW);
    motor_run(1, -SPD_SLOW);
    motor_run(2, -SPD_SLOW);
    motor_run(3,  SPD_SLOW);
}

/* 慢速原地右转 */
void Car_TurnRight_Slow(void)
{
    motor_run(0, -SPD_SLOW);
    motor_run(1,  SPD_SLOW);
    motor_run(2,  SPD_SLOW);
    motor_run(3, -SPD_SLOW);
}

/* 慢速向左平移，避障横移用的就是它 */
void Car_StrafeLeft_Slow(void)
{
    motor_run(0, -SPD_SLOW);
    motor_run(1,  SPD_SLOW);
    motor_run(2, -SPD_SLOW);
    motor_run(3,  SPD_SLOW);
}

/* 慢速向右平移 */
void Car_StrafeRight_Slow(void)
{
    motor_run(0,  SPD_SLOW);
    motor_run(1, -SPD_SLOW);
    motor_run(2,  SPD_SLOW);
    motor_run(3, -SPD_SLOW);
}
