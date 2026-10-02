#include "csb.h"

/* 打开 DWT 计数器，用来做微秒级延时 */
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

/* 微秒级延时，靠 DWT 周期计数实现 */
void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);

    while ((DWT->CYCCNT - start) < ticks);
}

/* ----------------------------------------------------------------------------
 * 单次测距
 * 时序：TRIG 拉低 2us，再拉高 10us 触发，然后等 ECHO 回波
 * 距离换算：声速按 340m/s，回波是往返路程所以要除以 2，
 *           距离(厘米) = 回波高电平时间(微秒) * 0.034 / 2
 * 返回 -1 表示这一轮没收到回波（超时），属于无效读数，
 * 上层判断时不能把 -1 当成“前方无阻挡”或“无限远”。
 * -------------------------------------------------------------------------- */
float HCSR04_Read(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                  GPIO_TypeDef *echo_port, uint16_t echo_pin)
{
    uint32_t t = 0;
    uint32_t timeout = 0;

    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
    delay_us(2);

    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
    delay_us(10);

    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);

    timeout = 0;
    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_RESET)
    {
        if (++timeout > 100000) return -1.0f;
    }

    timeout = 0;
    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_SET)
    {
        delay_us(1);
        t++;

        if (++timeout > 30000) return -1.0f;
    }

    if (t < ECHO_MIN_T) return -1.0f;

    return (float)t * 0.034f / 2.0f;
}

/* 左前 */
float HCSR04_Left(void)
{
    return HCSR04_Read(TRIG_L_PORT, TRIG_L_PIN,
                       ECHO_L_PORT, ECHO_L_PIN);
}

/* 正前 */
float HCSR04_Center(void)
{
    return HCSR04_Read(TRIG_C_PORT, TRIG_C_PIN,
                       ECHO_C_PORT, ECHO_C_PIN);
}

/* 右前 */
float HCSR04_Right(void)
{
    return HCSR04_Read(TRIG_R_PORT, TRIG_R_PIN,
                       ECHO_R_PORT, ECHO_R_PIN);
}
