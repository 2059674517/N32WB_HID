/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file app_gpio.c
 * @author Nations Firmware Team
 * @version v1.0.1
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#include "app_gpio.h"
#include <stdio.h>
#include "global_func.h"
#include "app_hid.h"
#include "app_hid_keyboard.h"
#include "ns_log.h"
#include "ns_timer.h"
#include "ns_delay.h"
#include "app_ble.h"
/** @addtogroup 
 * @{
 */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define KEY_PRESS_DELAY 100
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t key1_irq_actived = 0;
uint8_t key2_irq_actived = 0;
uint8_t key3_irq_actived = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


/**
 * @brief  Configures LED GPIO.
 * @param Led Specifies the Led to be configured.
 *   This parameter can be GPIO_PIN_0~GPIO_PIN_13.
 */
void LedInit(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_InitType GPIO_InitStructure;

    /* Check the parameters */
    assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

    /* Enable the GPIO Clock */
    if (GPIOx == GPIOA)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    }
    else if (GPIOx == GPIOB)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    }
    else
    {
        return;
    }

    /* Configure the GPIO pin as output push-pull */
    if (Pin <= GPIO_PIN_ALL)
    {
        GPIO_InitStruct(&GPIO_InitStructure);
        GPIO_InitStructure.Pin = Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure);
    }
}
/**
 * @brief  Turns selected Led on.
 * @param GPIOx x can be A to B to select the GPIO port.
 * @param Pin This parameter can be GPIO_PIN_0~GPIO_PIN_13.
 */
void LedOn(GPIO_Module* GPIOx, uint16_t Pin)
{    
    GPIO_SetBits(GPIOx, Pin);
}

/**
 * @brief  Turns selected Led Off.
 * @param GPIOx x can be A to B to select the GPIO port.
 * @param Pin This parameter can be GPIO_PIN_0~GPIO_PIN_13.
 */
void LedOff(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_ResetBits(GPIOx, Pin);
}

/**
 * @brief  Toggles the selected Led.
 * @param GPIOx x can be A to B to select the GPIO port.
 * @param Pin This parameter can be GPIO_PIN_0~GPIO_PIN_13.
 */
void LedBlink(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_TogglePin(GPIOx, Pin);
}

/**
 * @brief  Configures key port.
 */
void app_key_configuration(void)
{
    GPIO_InitType GPIO_InitStructure;
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    /* Enable the GPIO Clock */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);

    /*Configure the GPIO pin as input floating */
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin          = KEY1_INPUT_PIN;
    GPIO_InitStructure.GPIO_Pull    = GPIO_PULL_UP;
    GPIO_InitPeripheral(KEY1_INPUT_PORT, &GPIO_InitStructure);

    /* Enable the GPIO Clock */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_AFIO, ENABLE);

    /*Configure the GPIO pin as input floating*/
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin          = KEY2_INPUT_PIN;
    GPIO_InitStructure.GPIO_Pull    = GPIO_PULL_UP;
    GPIO_InitPeripheral(KEY2_INPUT_PORT, &GPIO_InitStructure);
		
		/*Configure the GPIO pin as input floating*/
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin          = KEY3_INPUT_PIN;
    GPIO_InitStructure.GPIO_Pull    = GPIO_PULL_UP;
    GPIO_InitPeripheral(KEY3_INPUT_PORT, &GPIO_InitStructure);


    /*Configure key EXTI Line to key input Pin*/
    GPIO_ConfigEXTILine(KEY1_INPUT_PORT_SOURCE, KEY1_INPUT_PIN_SOURCE);
    GPIO_ConfigEXTILine(KEY2_INPUT_PORT_SOURCE, KEY2_INPUT_PIN_SOURCE);
		GPIO_ConfigEXTILine(KEY3_INPUT_PORT_SOURCE, KEY3_INPUT_PIN_SOURCE);

    /*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line    = KEY1_INPUT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = KEY1_INPUT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
        
    /*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line    = KEY2_INPUT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = KEY2_INPUT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);        
    
		
		/*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line    = KEY3_INPUT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel                   = KEY3_INPUT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);        
}

/**
 * @brief  re-configures key port aafter sleep.
 */
void app_key_reinit_after_sleep(void)
{
    /* Enable the GPIO Clock */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);

}

/**
 * @brief key press timer handler.
 */
void app_key_press_timeout_handler(void)
{

    if(key1_irq_actived == 1)
    {
        LedBlink(LED1_PORT, LED1_PIN);
        #if (CFG_APP_HID)
        // Check if HID is ready before sending
        if (is_app_hid_ready()) {
            // Demo: Send keyboard 'ABC' key press
            hid_keyboard_report_t kb_report;
            uint8_t keys[] = {HID_KEY_A,HID_KEY_CAPS_LOCK,HID_KEY_B}; // Press 'aB' keys
            build_keyboard_report(&kb_report, 0, keys, 3);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);

            // Send key release
            build_keyboard_report(&kb_report, 0, NULL, 0);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            NS_LOG_INFO("Button 1, sending keyboard 'ABC' with Shift\r\n");
        } else {
            NS_LOG_WARNING("HID not ready, skipping keyboard send\r\n");
        }
        #endif
        key1_irq_actived = 0;
    }
    else if(key1_irq_actived == 2 )
    {
        //re-start timer if got irq after first created timer
        ns_timer_create(KEY_PRESS_DELAY,app_key_press_timeout_handler);
        key1_irq_actived = 1;
    }

    if(key2_irq_actived == 1)
    {
        LedBlink(LED1_PORT, LED1_PIN);
        #if (CFG_APP_HID)
        // Check if HID is ready before sending
        if (is_app_hid_ready()) {
            // Demo: Send keyboard 'Hello' with multi-key support
            hid_keyboard_report_t kb_report;

            // Send 'H' (Shift + h)
            uint8_t keys[] = {HID_KEY_H};
            build_keyboard_report(&kb_report, HID_KEYBOARD_LEFT_SHIFT, keys, 1);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            build_keyboard_report(&kb_report, 0, NULL, 0);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);

            // Send 'e'
            keys[0] = HID_KEY_E;
            build_keyboard_report(&kb_report, 0, keys, 1);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            build_keyboard_report(&kb_report, 0, NULL, 0);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);

            // Send 'l'
            keys[0] = HID_KEY_L;
            build_keyboard_report(&kb_report, 0, keys, 1);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            build_keyboard_report(&kb_report, 0, NULL, 0);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);

            // Send 'l'
            keys[0] = HID_KEY_L;
            build_keyboard_report(&kb_report, 0, keys, 1);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            build_keyboard_report(&kb_report, 0, NULL, 0);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);

            // Send 'o'
            keys[0] = HID_KEY_O;
            build_keyboard_report(&kb_report, 0, keys, 1);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            build_keyboard_report(&kb_report, 0, NULL, 0);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            NS_LOG_INFO("Button 2, sending 'Hello'\r\n");
        } else {
            NS_LOG_WARNING("HID not ready, skipping keyboard send\r\n");
        }
        #endif
        key2_irq_actived = 0;
    }
    else if(key2_irq_actived == 2)
    {
        //re-start timer if got irq after first created timer
        ns_timer_create(KEY_PRESS_DELAY,app_key_press_timeout_handler);
        key2_irq_actived = 1;
    }

    if(key3_irq_actived == 1)
    {
        LedBlink(LED1_PORT, LED1_PIN);
        #if (CFG_APP_HID)
        // Check if HID is ready before sending
        if (is_app_hid_ready()) {
            // Demo: Send multiple keys pressed at same time (Ctrl+A)
            hid_keyboard_report_t kb_report;
            uint8_t keys[] = {HID_KEY_A};
            build_keyboard_report(&kb_report, HID_KEYBOARD_LEFT_CTRL, keys, 1);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);

            // Release keys
            build_keyboard_report(&kb_report, 0, NULL, 0);
            app_hid_send_keyboard_report((uint8_t*)&kb_report);
            NS_LOG_INFO("Button 3, sending Ctrl+A\r\n");
        } else {
            NS_LOG_WARNING("HID not ready, skipping keyboard send\r\n");
        }
        #endif
        key3_irq_actived = 0;
    }
    else if(key3_irq_actived == 2)
    {
        //re-start timer if got irq after first created timer
        ns_timer_create(KEY_PRESS_DELAY,app_key_press_timeout_handler);
        key3_irq_actived = 1;
    }

}

/**
 * @brief  External lines 1 interrupt.
 */
void EXTI4_12_IRQHandler(void)
{
    
    if ( EXTI_GetITStatus(KEY1_INPUT_EXTI_LINE)!= RESET)
    {
        delay_n_10us(1000);
				if (GPIO_ReadInputDataBit(KEY1_INPUT_PORT,KEY1_INPUT_PIN) == 0)
				{
					if(key1_irq_actived == 0)
					{
							ke_msg_send_basic(APP_KEY_DETECTED, TASK_APP, TASK_APP); 
							key1_irq_actived = 1;
					}
					else if(key1_irq_actived == 1)
					{
							//if timer started, update flag
							key1_irq_actived = 2;

					}
				}
        
        EXTI_ClrITPendBit(KEY1_INPUT_EXTI_LINE);
    }
		if ( EXTI_GetITStatus(KEY2_INPUT_EXTI_LINE)!= RESET)
    {
        delay_n_10us(1000);
				if (GPIO_ReadInputDataBit(KEY2_INPUT_PORT,KEY2_INPUT_PIN) == 0)
				{
					if(key2_irq_actived == 0)
					{
							ke_msg_send_basic(APP_KEY_DETECTED, TASK_APP, TASK_APP); 
							key2_irq_actived = 1;
					}
					else if(key2_irq_actived == 1)
					{
							//if timer started, update flag
							key2_irq_actived = 2;
					}
				}
        
        EXTI_ClrITPendBit(KEY2_INPUT_EXTI_LINE);
    }
		if ( EXTI_GetITStatus(KEY3_INPUT_EXTI_LINE)!= RESET)
    {
        delay_n_10us(1000);
				if (GPIO_ReadInputDataBit(KEY3_INPUT_PORT,KEY3_INPUT_PIN) == 0)
				{
					if(key3_irq_actived == 0)
					{
							ke_msg_send_basic(APP_KEY_DETECTED, TASK_APP, TASK_APP); 
							key3_irq_actived = 1;
					}
					else if(key3_irq_actived == 1)
					{
							//if timer started, update flag
							key3_irq_actived = 2;
					}
				}
        
        EXTI_ClrITPendBit(KEY3_INPUT_EXTI_LINE);
    }
}


/**
 * @}
 */
