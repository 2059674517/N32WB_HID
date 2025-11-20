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
 * @file app_hid_touchscreen.h
 * @author Nations Firmware Team
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#ifndef __APP_HID_TOUCHSCREEN_H__
#define __APP_HID_TOUCHSCREEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Touch Screen report structure
typedef struct {
    uint8_t contact_id : 4;     // Contact identifier (0-15)
    uint8_t tip_switch : 1;     // Finger touching (1) or not (0)
    uint8_t in_range : 1;       // Finger in range
    uint8_t touch_valid : 1;    // Touch data is valid
    uint8_t padding : 1;         // Padding bit
    uint16_t x;                  // X coordinate (0-2047)
    uint16_t y;                  // Y coordinate (0-1151)
    uint8_t pressure;            // Pressure (0-255)
    uint8_t width;               // Contact width (0-127)
    uint8_t height;              // Contact height (0-127)
} hid_touch_contact_t;

typedef struct {
    hid_touch_contact_t contact;  // Single touch contact
    uint8_t contact_count;        // Number of contacts (0-10)
    uint8_t contact_count_max;    // Maximum supported contacts (10)
} hid_touchscreen_report_t;

// Multi-touch support (up to 5 fingers)
typedef struct {
    hid_touch_contact_t contacts[5];  // Up to 5 simultaneous touches
    uint8_t contact_count;            // Number of active contacts
    uint8_t contact_count_max;        // Maximum supported contacts
} hid_multitouch_report_t;

/**
 * @brief Send touch screen event
 * @param contact_id Contact identifier (0-15)
 * @param is_touching 1 if touching, 0 if not
 * @param x X coordinate (0-2047)
 * @param y Y coordinate (0-1151)
 * @param pressure Pressure value (0-255)
 */
void app_hid_send_touchscreen(uint8_t contact_id, uint8_t is_touching,
                              uint16_t x, uint16_t y, uint8_t pressure);

/**
 * @brief Simulate a tap on the touchscreen
 * @param x X coordinate to tap
 * @param y Y coordinate to tap
 */
void app_touchscreen_tap(uint16_t x, uint16_t y);

/**
 * @brief Simulate a swipe on the touchscreen
 * @param x_start Starting X coordinate
 * @param y_start Starting Y coordinate
 * @param x_end Ending X coordinate
 * @param y_end Ending Y coordinate
 * @param duration_ms Duration of swipe in milliseconds
 */
void app_touchscreen_swipe(uint16_t x_start, uint16_t y_start,
                           uint16_t x_end, uint16_t y_end,
                           uint16_t duration_ms);

#ifdef __cplusplus
}
#endif

#endif /* __APP_HID_TOUCHSCREEN_H__ */
