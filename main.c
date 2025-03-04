/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RutDevKit-USB_CDC_Test
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*  Created on: 2021-06-01
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Address: Jonavos g. 30, Kaunas 44262, Lithuania
*  Author: GDR
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cycfg.h"
#include "cy_retarget_io.h"

#include "bgt60trxxx.h"

#include <stdio.h>
#include <stdlib.h>

#include "custom_alloc.h"
#include "presence_detection/presence_detection.h"

#include "hal_timer.h"


void handle_error(void);

void presence_detection_listener(float magnitude, uint16_t bin, float angle)
{
	printf("Presence detected. Mag: %1.1f - Distance: %1.1f \r\n", magnitude, presence_detection_bin_to_meters(bin));
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  This is the main function for CM4 CPU. It initializes the USB Device block
*  and enumerates as a CDC device. It constantly checks for data received from
*  host and echos it back.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    uint16_t * samples = NULL;
    presence_detection_param_t params;
    radar_configuration_t radar_configuration;
    int retval = 0;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /*Enable debug output via KitProg UART*/
	result = cy_retarget_io_init( KITPROG_TX, KITPROG_RX, CY_RETARGET_IO_BAUDRATE);
	if (result != CY_RSLT_SUCCESS)
	{
		handle_error();
	}

	printf("***********************************\r\n");
	printf("\tRDK2 - BGT60UTR11AIP - Presence Detection \r\n");
	printf("***********************************\r\n");

    /* init buttons */
    result = cyhal_gpio_init(USER_BTN1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
	if (result != CY_RSLT_SUCCESS)
	{CY_ASSERT(0);}

    /*Initialize LEDs*/
    result = cyhal_gpio_init( LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}
    result = cyhal_gpio_init( LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}

    int hal_ret = hal_timer_init();
    if (hal_ret != 0)
    {
    	printf("Cannot init timer: %d \r\n", hal_ret);
    	for(;;){}
    }

    // Init samples
    samples = custom_malloc(bgt60trxxx_get_samples_per_frame() * sizeof(uint16_t));

    // Init presence detection algorithm
    params.threshold = 0.2;
    params.bin_start = 0;
    params.bin_end = 0; // bin_start = bin_end -> complete range

    radar_configuration.antenna_count = bgt60trxxx_get_antenna_count();
    radar_configuration.chirps_per_frame = bgt60trxxx_get_chirps_per_frame();
    radar_configuration.samples_per_chirp = bgt60trxxx_get_samples_per_chirp();
    radar_configuration.start_freq = bgt60trxxx_get_start_freq();
    radar_configuration.end_freq = bgt60trxxx_get_end_freq();
    radar_configuration.sampling_rate = bgt60trxxx_get_sampling_rate();

    presence_detection_set_malloc_free(custom_malloc, custom_free);
    presence_detection_set_listener(presence_detection_listener);
    retval = presence_detection_init(radar_configuration, params);
    if (retval != 0)
    {
    	printf("presence_detection_init error: %d \r\n", retval);
    	handle_error();
    }

    printf("Initialize radar sensor\r\n");

    // Start frame generation
    int bgtret = bgt60trxxx_init();
    if (bgtret != 0)
    {
    	printf("Cannot init: %d \r\n", bgtret);
    	cyhal_gpio_write(LED1, CYBSP_LED_STATE_ON);
    	cyhal_gpio_write(LED2, CYBSP_LED_STATE_ON);
    	for(;;){}
    }

    printf("Ok, radar initialized - Start measurement \r\n");

    cyhal_gpio_write(LED1, CYBSP_LED_STATE_OFF);
    cyhal_gpio_write(LED2, CYBSP_LED_STATE_OFF);

    for(;;)
    {
    	// Read and send over USB
    	if (bgt60trxxx_is_data_available())
    	{
    		retval = bgt60trxxx_get_data(samples);
    		if (retval == 0)
    		{
    			cyhal_gpio_write(LED2, CYBSP_LED_STATE_ON);
    			presence_detection_feed(samples);
    			cyhal_gpio_write(LED2, CYBSP_LED_STATE_OFF);
    		}
    		cyhal_gpio_toggle(LED1);
    	}
    }
}

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();
    cyhal_gpio_write(LED1, CYBSP_LED_STATE_OFF);
    cyhal_gpio_write(LED2, CYBSP_LED_STATE_ON);
    CY_ASSERT(0);
}

/* [] END OF FILE */
