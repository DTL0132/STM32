#ifndef __LANYA_H
#define __LANYA_H

#include "com.h"

/* ============================================================================
 * 蓝牙命令表
 * 串口收到一整行文本（以换行符结尾）后解析成下面的命令码，
 * 主循环再根据 bt_cmd 执行对应动作。
 * ========================================================================== */

#define CMD_STOP 0
#define CMD_FORWARD 1
#define CMD_BACKWARD       2
#define CMD_STRAFE_LEFT    3
#define CMD_STRAFE_RIGHT   4
#define CMD_TURN_LEFT      5
#define CMD_TURN_RIGHT     6
#define CMD_AUTO           7
#define CMD_TURN_LEFT_SLOW    8
#define CMD_TURN_RIGHT_SLOW   9
#define CMD_STRAFE_LEFT_SLOW  10
#define CMD_STRAFE_RIGHT_SLOW 11
#define CMD_FORWARD_SLOW      12
#define CMD_BACKWARD_SLOW     13

extern uint8_t  bt_rx_byte;
extern char     bt_line[16];
extern uint8_t  bt_len;
extern volatile uint8_t bt_cmd;
extern volatile uint8_t step_req;
#endif
