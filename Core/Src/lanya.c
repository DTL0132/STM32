#include "lanya.h"
#include "motor.h"
#include "servo.h"
extern UART_HandleTypeDef huart3;

/* ============================================================================
 * 蓝牙串口接收
 * 每收到一个字节进一次中断：先把字节原样回显，方便调试时确认链路；
 * 遇到换行符就把这一行拼成字符串，再去和命令表比对；
 * 比对成功就改写 bt_cmd，或者直接操作舵机与升降。
 * 注意：蓝牙助手必须勾选“发送新行”，否则命令不会触发。
 * ========================================================================== */

uint8_t  bt_rx_byte;
char     bt_line[16];
uint8_t  bt_len = 0;
volatile uint8_t bt_cmd = CMD_STOP;
volatile uint8_t step_req = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        /* 回显：把收到的字节原样发回去，用来确认单片机确实收到了数据 */
        HAL_UART_Transmit(&huart3, &bt_rx_byte, 1, 10);

        /* 收到换行符说明一条命令结束，开始解析 */
        if (bt_rx_byte == '\n' || bt_rx_byte == '\r')
        {
            bt_line[bt_len] = '\0';
            if (strcmp(bt_line, "fwd") == 0)
                bt_cmd = CMD_FORWARD;
            else if (strcmp(bt_line, "stop") == 0)
                bt_cmd = CMD_STOP;
						else if (strcmp(bt_line, "auto") == 0 ||
                     strcmp(bt_line, "avoid") == 0)
                bt_cmd = CMD_AUTO;
            else if (strcmp(bt_line, "up") == 0)
                Lift_Run(1);          /* 升降上升 */
            else if (strcmp(bt_line, "down") == 0)
                Lift_Run(-1);         /* 升降下降 */
            else if (strcmp(bt_line, "pause") == 0) 
                Lift_Run(0);          /* 升降停止 */
            else if (strcmp(bt_line, "grip") == 0)
                Gripper_SetAngle(SERVO_CLOSE);
            else if (strcmp(bt_line, "open") == 0)
                Gripper_SetAngle(SERVO_OPEN);
						else if (strcmp(bt_line, "turn") == 0)
								Gripper_SetAngle1(10);
						else if (strcmp(bt_line, "tur") == 0)
								Gripper_SetAngle1(170);
						else if (strcmp(bt_line, "back") == 0)
                bt_cmd = CMD_BACKWARD;
						else if (strcmp(bt_line, "stp1") == 0)
							  step_req = 1;
						else if (strcmp(bt_line, "stp2") == 0)
							  step_req = 2;
            else if (strcmp(bt_line, "left") == 0 ||
                     strcmp(bt_line, "strafeleft") == 0)
                bt_cmd = CMD_STRAFE_LEFT;

            else if (strcmp(bt_line, "right") == 0 ||
                     strcmp(bt_line, "straferight") == 0)
                bt_cmd = CMD_STRAFE_RIGHT;

            else if (strcmp(bt_line, "turnleft") == 0)
                bt_cmd = CMD_TURN_LEFT;

            else if (strcmp(bt_line, "turnright") == 0)
						{
                bt_cmd = CMD_TURN_RIGHT;
				  	}
						else if (strcmp(bt_line, "tsl") == 0) bt_cmd = CMD_TURN_LEFT_SLOW;
            else if (strcmp(bt_line, "tsr") == 0) bt_cmd = CMD_TURN_RIGHT_SLOW;
            else if (strcmp(bt_line, "ssl") == 0) bt_cmd = CMD_STRAFE_LEFT_SLOW;
            else if (strcmp(bt_line, "ssr") == 0) bt_cmd = CMD_STRAFE_RIGHT_SLOW;

            else if (strcmp(bt_line, "sfwd") == 0)  bt_cmd = CMD_FORWARD_SLOW;
            else if (strcmp(bt_line, "sback") == 0) bt_cmd = CMD_BACKWARD_SLOW;
						
        bt_len = 0;
				}
        else if (bt_len < sizeof(bt_line) - 1)
        {
            bt_line[bt_len++] = bt_rx_byte;
        }
        else
        {
            bt_len = 0;
        }
        HAL_UART_Receive_IT(&huart3, &bt_rx_byte, 1);
    }
}
