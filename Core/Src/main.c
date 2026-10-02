/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor.h"
#include "servo.h"
#include "step_motor.h"
#include "csb.h"
#include "lanya.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t boot_ms = 0; 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ============================================================================
 * 程序结构说明
 *   上电完成外设初始化后进入主循环，主循环按命令分派：
 *     bt_cmd == CMD_AUTO  ->  AutoAvoid_Run()   自动避障
 *     其它命令            ->  Manual_Run()      手动遥控
 *   上电默认是停车状态（bt_cmd = CMD_STOP），不会自己跑，等蓝牙命令。
 * ========================================================================== */
static uint8_t  avoid_state = 0;   /* 0 = 直行, 1 = 横移中 */
static uint8_t  avoid_dir   = 0;   /* 1 = 左, 2 = 右 */
static uint8_t  avoid_side  = 0;   /* 左右都测不到时交替用 */
static uint32_t avoid_end   = 0;
static uint32_t avoid_start = 0;

/* ----------------------------------------------------------------------------
 * 自动避障：每次调用读一轮三路超声波
 *   直行阶段：三路任意一路发现障碍，先停车 120ms 缓冲，再选方向横移
 *   横移阶段：朝选定方向持续横移，每 200ms 用正前方复查一次
 *   躲开之后才恢复直行，并解锁方向，让下一个障碍可以重新选边
 *   单次横移最长 2.5 秒，超时就停车，避免长时间顶住障碍堵转过流
 * -------------------------------------------------------------------------- */
static void AutoAvoid_Run(void)
{
    /* 三路超声波依次触发，中间留 20ms 间隔，减少相互串扰 */
    float left   = HCSR04_Left();
    HAL_Delay(20);
    float center = HCSR04_Center();
    HAL_Delay(20);
    float right  = HCSR04_Right();
    HAL_Delay(20);

    uint8_t left_hit   = (left   > 0.0f && left   < OBSTACLE_CM);
    uint8_t center_hit = (center > 0.0f && center < OBSTACLE_CM);
    uint8_t right_hit  = (right  > 0.0f && right  < OBSTACLE_CM);

    if (avoid_state == 0)
    {
        if (left_hit || center_hit || right_hit)
        {
            float l = (left  > 0.0f) ? left  : 999.0f;
            float r = (right > 0.0f) ? right : 999.0f;

            if (left_hit && right_hit)
            {
                Car_Stop();
                return;
            }

            /* 选方向：哪边被挡就避开；两边都有有效读数时，选更空的一侧 */
            if (left_hit && !right_hit)       avoid_dir = 2;
            else if (right_hit && !left_hit)  avoid_dir = 1;
            else if (left > 0.0f && right > 0.0f && l > r)  avoid_dir = 1;
            else if (left > 0.0f && right > 0.0f && r > l)  avoid_dir = 2;
            else if (avoid_dir == 0)   /* 完全没信息时才定方向, 有方向就保持不动 */
            {
                avoid_dir   = (avoid_side == 0) ? 1 : 2;
                avoid_side ^= 1;
            }

            /* 换向前先停 120ms 让轮子滑行减速，减小反向冲击电流 */
            Car_Stop();
            HAL_Delay(120);

            avoid_start = HAL_GetTick();
            avoid_state = 1;
            avoid_end   = HAL_GetTick() + 600;
        }
        else
        {
            Car_Forward_Avoid();
        }
    }

    if (avoid_state == 1)
    {
        if (HAL_GetTick() - avoid_start > 2500)
        {
            Car_Stop();
            avoid_state = 0;
            return;
        }

        /* 实测校正：avoid_dir == 1 时实际要向车辆右侧平移 */
        if (avoid_dir == 1) Car_StrafeRight_Slow();
        else                Car_StrafeLeft_Slow();

        if (HAL_GetTick() >= avoid_end)
        {
            float c = HCSR04_Center();

            /* 正前方已经空了，说明躲开了，恢复直行 */
            if (c > CLEAR_CM || c < 0.0f)
            {
                Car_Stop();
                HAL_Delay(120);   /* 同样留出滑行时间，保护电机和电源 */
                avoid_dir   = 0;       /* 这次躲开了, 下次重新选边 */
                avoid_state = 0;
            }

            avoid_end = HAL_GetTick() + 200;
        }
    }
}

/* ----------------------------------------------------------------------------
 * 手动遥控：直接按蓝牙命令驱动底盘
 *   进入手动模式先清掉避障状态，避免上一次的避障状态残留
 * -------------------------------------------------------------------------- */
static void Manual_Run(void)
{
    avoid_state = 0;

    switch (bt_cmd)
    {
    case CMD_FORWARD:      Car_Forward();     break;
    case CMD_BACKWARD:     Car_Backward();    break;
    case CMD_STRAFE_LEFT:  Car_StrafeLeft();  break;
    case CMD_STRAFE_RIGHT: Car_StrafeRight(); break;
    case CMD_TURN_LEFT:    Car_TurnLeft();    break;
    case CMD_TURN_RIGHT:   Car_TurnRight();   break;
		case CMD_TURN_LEFT_SLOW:    Car_TurnLeft_Slow();    break;
    case CMD_TURN_RIGHT_SLOW:   Car_TurnRight_Slow();   break;
    case CMD_STRAFE_LEFT_SLOW:  Car_StrafeLeft_Slow();  break;
    case CMD_STRAFE_RIGHT_SLOW: Car_StrafeRight_Slow(); break;
    case CMD_FORWARD_SLOW:      Car_Forward_Slow();     break;
    case CMD_BACKWARD_SLOW:     Car_Backward_Slow();    break;
    case CMD_STOP:
    default:               Car_Stop();        break;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  /* 拉高 STBY，使能四路电机驱动板 */
  HAL_GPIO_WritePin(STBY_PORT, STBY_PIN, GPIO_PIN_SET);

 /* 启动四路底盘电机的 PWM（TIM3 的四个通道） */
 HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
 HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
 HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
 HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
 
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
 __HAL_TIM_MOE_ENABLE(&htim1);

 /* 启动两路舵机 PWM：TIM4_CH1 是夹爪，TIM4_CH2 是云台 */
 HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
 Gripper_SetAngle(SERVO_OPEN);   /* 上电夹爪张开 */
 Gripper_SetAngle1(170);         /* 上电云台转到 170 度位 */
 HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
 
 /* 记录上电时刻，并打开串口接收中断，开始收蓝牙数据 */
 boot_ms = HAL_GetTick();
 HAL_UART_Receive_IT(&huart3, &bt_rx_byte, 1);
 
 /* 初始化 DWT 计数器，给超声波测距提供微秒级延时 */
 DWT_Init();
 
 HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
 __HAL_TIM_MOE_ENABLE(&htim1);
 
 Lift_Run(0);          /* 升降电机上电先停住 */
 
 /* 上电默认停车，等蓝牙命令；步进的初始化保留，该机构最终未使用 */
 bt_cmd = CMD_STOP;
 StepMotor_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
/* 主循环：自动避障与手动遥控二选一执行 */
while (1)
{
    if (bt_cmd == CMD_AUTO)
    {
        AutoAvoid_Run();
    }
    else
    {
        Manual_Run();
    }

		/* 步进的一次性请求，执行完立刻清零，避免重复转动 */
		if (step_req == 1)
		{
				step_req = 0; 
				StepMotor_Rotate120(1);
		}
		else if (step_req == 2)
		{
				step_req = 0; 
				StepMotor_Rotate120(0);
		}
    HAL_Delay(20);
}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
