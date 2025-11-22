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
#include "app_hid_touchscreen.h"
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
uint8_t key_enable = 0;//�������ʹ�ܣ���Ϊ0�ż�ⰴ���Ƿ���
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

    if(key1_irq_actived != 0)
    {
        LedBlink(LED1_PORT, LED1_PIN);
        #if (CFG_APP_HID)
        // Check if HID is ready before sending
        if (is_app_hid_ready()) {
						uint32_t report = 0; 
						// ����Play/Pause����1λ��
						report = (1UL << 28) ;
						app_hid_send_consumer_report((uint8_t*)&report);
						report = 0; 
						app_hid_send_consumer_report((uint8_t*)&report);
					
						report = (1UL << 1) ;
						app_hid_send_consumer_report((uint8_t*)&report);
						report = 0; 
						app_hid_send_consumer_report((uint8_t*)&report);
        } else {
            //NS_LOG_WARNING("HID not ready, skipping keyboard send\r\n");
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

            uint8_t all_key_values[] = {
//								// ��ĸ����A-Z��
//								HID_KEY_A, HID_KEY_B, HID_KEY_C, HID_KEY_D, HID_KEY_E,
//								HID_KEY_F, HID_KEY_G, HID_KEY_H, HID_KEY_I, HID_KEY_J,
//								HID_KEY_K, HID_KEY_L, HID_KEY_M, HID_KEY_N, HID_KEY_O,
//								HID_KEY_P, HID_KEY_Q, HID_KEY_R, HID_KEY_S, HID_KEY_T,
//								HID_KEY_U, HID_KEY_V, HID_KEY_W, HID_KEY_X, HID_KEY_Y,
//								HID_KEY_Z,

//								// ���ּ����������� 0-9��
//								HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5,
//								HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9, HID_KEY_0,
//								// ���ż�������������
//								HID_KEY_ENTER, HID_KEY_ESC, HID_KEY_BACKSPACE, HID_KEY_TAB,
//								HID_KEY_SPACE, HID_KEY_MINUS, HID_KEY_EQUAL, HID_KEY_LEFTBRACE,
//								HID_KEY_RIGHTBRACE, HID_KEY_BACKSLASH, HID_KEY_SEMICOLON,
//								HID_KEY_APOSTROPHE, HID_KEY_GRAVE, HID_KEY_COMMA, HID_KEY_DOT,
//								HID_KEY_SLASH,

								// ���ܼ���F1-F24��
//								HID_KEY_F1, HID_KEY_F2, HID_KEY_F3, HID_KEY_F4, HID_KEY_F5,
//								HID_KEY_F6, HID_KEY_F7, HID_KEY_F8, HID_KEY_F9, HID_KEY_F10,
//								HID_KEY_F11, HID_KEY_F12, HID_KEY_F13, HID_KEY_F14, HID_KEY_F15,
//								HID_KEY_F16, HID_KEY_F17, HID_KEY_F18, HID_KEY_F19, HID_KEY_F20,
//								HID_KEY_F21, HID_KEY_F22, HID_KEY_F23, HID_KEY_F24,

								// �༭����
//								HID_KEY_CAPS_LOCK, HID_KEY_PRINT_SCREEN, HID_KEY_SCROLL_LOCK,
//								HID_KEY_PAUSE, HID_KEY_INSERT, HID_KEY_HOME, HID_KEY_PAGEUP,
//								HID_KEY_DELETE, HID_KEY_END, HID_KEY_PAGEDOWN, HID_KEY_RIGHT,
//								HID_KEY_LEFT, HID_KEY_DOWN, HID_KEY_UP, HID_KEY_NUM_LOCK,

//								// С������
//								HID_KEY_KP_SLASH, HID_KEY_KP_ASTERISK, HID_KEY_KP_MINUS,
//								HID_KEY_KP_PLUS, HID_KEY_KP_ENTER, HID_KEY_KP_1, HID_KEY_KP_2,
//								HID_KEY_KP_3, HID_KEY_KP_4, HID_KEY_KP_5, HID_KEY_KP_6,
//								HID_KEY_KP_7, HID_KEY_KP_8, HID_KEY_KP_9, HID_KEY_KP_0,
//								HID_KEY_KP_DOT, HID_KEY_KP_EQUAL,

//								// ��ý����Ƽ������ּ���֧�֣�
//								HID_KEY_MUTE, HID_KEY_VOLUME_UP, HID_KEY_VOLUME_DOWN,
//								HID_KEY_MEDIA_NEXT, HID_KEY_MEDIA_PREV, HID_KEY_MEDIA_PLAY_PAUSE
									
									HID_KEY_LEFT_CONTROL, HID_KEY_LEFT_SHIFT, HID_KEY_LEFT_ALT, HID_KEY_LEFT_GUI,
									HID_KEY_RIGHT_CONTROL, HID_KEY_RIGHT_SHIFT, HID_KEY_RIGHT_ALT, HID_KEY_RIGHT_GUI
						};
						uint8_t keys[6] = {0};
//						for (int i = 0; i < sizeof(all_key_values) / sizeof(all_key_values[0]); i++) {
//								// ���µ�ǰ�����������μ���
//								keys[0] = all_key_values[i];
//								build_keyboard_report(&kb_report, 0, keys, 1); 
//								app_hid_send_keyboard_report((uint8_t*)&kb_report);
//								// �ɿ���ǰ���������Ϳձ��棩
//								build_keyboard_report(&kb_report, 0, NULL, 0);
//								app_hid_send_keyboard_report((uint8_t*)&kb_report);
//						}
						keys[0] = HID_KEY_LEFT_CONTROL;
						keys[1] = HID_KEY_A;
						build_keyboard_report(&kb_report, 0, keys, 2); 
						app_hid_send_keyboard_report((uint8_t*)&kb_report);
						// �ɿ���ǰ���������Ϳձ��棩
						build_keyboard_report(&kb_report, 0, NULL, 0);
						app_hid_send_keyboard_report((uint8_t*)&kb_report);
            NS_LOG_INFO("Button 2, sending 'Hello' %d \r\n",key_enable);
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
            // Demo: Multi-touch screen operations
            static uint8_t touch_demo = 0;
						uint16_t x_start[3] = {8000, 16384, 24000};
            uint16_t y_start[3] = {5000, 5000, 5000};
						uint16_t x_end[3] = {8000, 16384, 24000};
            uint16_t y_end[3] = {15000, 15000, 15000};
						//app_multi_touchscreen_swipe(x_start,y_start,x_end,y_end,300);
						app_touchscreen_swipe(16384, 32767/10*3, 16384, 32767/10*6, 300);
//						switch(touch_demo % 2) {
//                case 0:
//                    // Single tap
//                    app_touchscreen_threefinger_swipe(16384, 32767/10*3, 16384, 32767/10*6, 300);
//                    break;

//                case 1:
//                    // Swipe
//                    app_touchscreen_swipe(16384, 32767/10*3, 16384, 32767/10*6, 300);
//                    break;
//						}
//            switch(touch_demo % 6) {
//                case 0:
//                    // Single tap
//                    NS_LOG_INFO("Touch screen: Single tap\r\n");
//                    app_touchscreen_tap(16384, 16384);  // Center of screen
//                    break;

//                case 1:
//                    // Swipe
//                    NS_LOG_INFO("Touch screen: Swipe\r\n");
//                    app_touchscreen_swipe(16384, 32767/10*3, 16384, 32767/10*6, 300);
//                    break;

//                case 2:
//                    // Two-finger tap (real multi-touch)
//                    NS_LOG_INFO("Touch screen: Two-finger tap\r\n");
//                    {
//                        uint16_t x_coords[2] = {10000, 22000};
//                        uint16_t y_coords[2] = {16384, 16384};
//                        app_touchscreen_multi_tap(2, x_coords, y_coords);
//                    }
//                    break;

//                case 3:
//                    // Three-finger tap (real multi-touch)
//                    NS_LOG_INFO("Touch screen: Three-finger tap\r\n");
//                    {
//                        uint16_t x_coords[3] = {8000, 16384, 24000};
//                        uint16_t y_coords[3] = {16384, 16384, 16384};
//                        app_touchscreen_multi_tap(3, x_coords, y_coords);
//                    }
//                    break;

//                case 4:
//                    // Pinch gesture (zoom)
//                    NS_LOG_INFO("Touch screen: Pinch zoom\r\n");
//                    app_touchscreen_pinch(16384, 16384, 8000, 16000, 500);
//                    break;

//                case 5:
//                    // Rotate gesture
//                    NS_LOG_INFO("Touch screen: Two-finger rotate\r\n");
//                    app_touchscreen_rotate(16384, 16384, 6000, 90, 800);
//                    break;
//            }

            touch_demo++;
        } else {
            NS_LOG_WARNING("HID not ready, skipping touch screen send\r\n");
        }
        #endif
        //key3_irq_actived = 0;
				ns_timer_create(KEY_PRESS_DELAY,app_key_press_timeout_handler);
        key3_irq_actived = 1;
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
	if(key_enable != 0){
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
  else{
		if ( EXTI_GetITStatus(KEY1_INPUT_EXTI_LINE)!= RESET)
			{
					delay_n_10us(1000);
					
					EXTI_ClrITPendBit(KEY1_INPUT_EXTI_LINE);
			}
			if ( EXTI_GetITStatus(KEY2_INPUT_EXTI_LINE)!= RESET)
			{
					delay_n_10us(1000);
					
					EXTI_ClrITPendBit(KEY2_INPUT_EXTI_LINE);
			}
			if ( EXTI_GetITStatus(KEY3_INPUT_EXTI_LINE)!= RESET)
			{
					delay_n_10us(1000);
					
					EXTI_ClrITPendBit(KEY3_INPUT_EXTI_LINE);
			}
	}		
}


/**
 * @}
 */
