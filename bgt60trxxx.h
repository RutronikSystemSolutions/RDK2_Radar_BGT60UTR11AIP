/*
 * bgt60trxxx.h
 *
 *  Created on: 31 May 2024
 *      Author: jorda
 *
 * Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
 * including the software is for testing purposes only and,
 * because it has limited functions and limited resilience, is not suitable
 * for permanent use under real conditions. If the evaluation board is
 * nevertheless used under real conditions, this is done at one’s responsibility;
 * any liability of Rutronik is insofar excluded
 */

#ifndef BGT60TRXXX_H_
#define BGT60TRXXX_H_

#include <stdint.h>

int bgt60trxxx_init();

uint16_t bgt60trxxx_is_data_available();

int bgt60trxxx_get_data(uint16_t* data);

int bgt60trxxx_get_fifo_status(uint32_t* status);

uint16_t bgt60trxxx_get_samples_per_frame();

uint16_t bgt60trxxx_get_antenna_count();

uint16_t bgt60trxxx_get_chirps_per_frame();

uint16_t bgt60trxxx_get_samples_per_chirp();

uint64_t bgt60trxxx_get_end_freq();

uint64_t bgt60trxxx_get_start_freq();

uint32_t bgt60trxxx_get_sampling_rate();

#endif /* BGT60TRXXX_H_ */
