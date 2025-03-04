/*
 * hal_timer.h
 *
 *  Created on: Feb 21, 2025
 *      Author: ROJ030
 */

#ifndef HAL_TIMER_H_
#define HAL_TIMER_H_


#include <stdint.h>

int hal_timer_init();

uint32_t hal_timer_get_uticks(void);


#endif /* HAL_TIMER_H_ */
