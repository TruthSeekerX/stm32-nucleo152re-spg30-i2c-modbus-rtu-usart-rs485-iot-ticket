/*
 * utils.h
 *
 *  Created on: Feb 24, 2023
 *      Author: Siyuan Xu
 */

#ifndef UTILS_H_
#define UTILS_H_

#define DEBUG_CONSOLE_EN 1
#define DBUG_MSG_LEN     100

void delay_us(const unsigned long delay);
void delay_ms(const unsigned long delay);
void debug_console(const char *message);

#endif /* UTILS_H_ */
