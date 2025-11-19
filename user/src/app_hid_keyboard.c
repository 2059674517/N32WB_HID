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
 * @file app_hid_keyboard.c
 * @author Nations Firmware Team
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */

#include "app_hid_keyboard.h"
#include <string.h>
#include "app_hid.h"
#include "ns_delay.h"
#include "ns_log.h"

/**
 * @brief Build HID keyboard report
 * @param report Pointer to keyboard report structure
 * @param modifier Modifier keys (Ctrl, Shift, Alt, GUI)
 * @param keys Array of key codes to send
 * @param key_count Number of keys in the array (max 6)
 */
void build_keyboard_report(hid_keyboard_report_t* report, uint8_t modifier, uint8_t* keys, uint8_t key_count)
{
    // Clear the report
    memset(report, 0, sizeof(hid_keyboard_report_t));

    // Set modifier keys
    report->modifier = modifier;

    // Copy key codes (max 6 keys)
    if (keys && key_count > 0) {
        uint8_t count = (key_count > 6) ? 6 : key_count;
        memcpy(report->keys, keys, count);
    }
}

/**
 * @brief Build HID touch screen report
 * @param report Pointer to touch screen report structure
 * @param contact_id Contact identifier (which finger)
 * @param is_touching Whether the finger is touching (1) or not (0)
 * @param x X coordinate (0-2047)
 * @param y Y coordinate (0-1151)
 * @param pressure Pressure value (0-255)
 */
void build_touchscreen_report(hid_touchscreen_report_t* report,
                              uint8_t contact_id,
                              uint8_t is_touching,
                              uint16_t x, uint16_t y,
                              uint8_t pressure)
{
    // Clear the report
    memset(report, 0, sizeof(hid_touchscreen_report_t));

    // Set contact information
    report->contact.contact_id = contact_id & 0x0F;  // Limit to 4 bits
    report->contact.tip_switch = is_touching ? 1 : 0;
    report->contact.in_range = is_touching ? 1 : 0;
    report->contact.touch_valid = 1;  // Always valid when we're sending
    report->contact.padding = 0;

    // Set coordinates (limit to valid ranges)
    report->contact.x = (x > 2047) ? 2047 : x;
    report->contact.y = (y > 1151) ? 1151 : y;

    // Set pressure
    report->contact.pressure = pressure;

    // Set default width and height (can be customized)
    report->contact.width = 20;   // Default contact width
    report->contact.height = 20;  // Default contact height

    // Set contact count
    report->contact_count = is_touching ? 1 : 0;
    report->contact_count_max = 10;  // Support up to 10 contacts
}

/**
 * @brief Simulate a touch screen tap
 * @param x X coordinate to tap
 * @param y Y coordinate to tap
 */
void touchscreen_tap(uint16_t x, uint16_t y)
{
    hid_touchscreen_report_t report;

    // Send touch down event
    build_touchscreen_report(&report, 0, 1, x, y, 100);
    app_hid_send_touchscreen_report(&report);

    // Small delay for tap duration
    delay_n_ms(50);

    // Send touch up event
    build_touchscreen_report(&report, 0, 0, x, y, 0);
    app_hid_send_touchscreen_report(&report);
}

/**
 * @brief Simulate a touch screen swipe
 * @param x_start Starting X coordinate
 * @param y_start Starting Y coordinate
 * @param x_end Ending X coordinate
 * @param y_end Ending Y coordinate
 * @param duration_ms Duration of the swipe in milliseconds
 */
void touchscreen_swipe(uint16_t x_start, uint16_t y_start,
                       uint16_t x_end, uint16_t y_end,
                       uint8_t duration_ms)
{
    hid_touchscreen_report_t report;
    uint8_t steps = 10;  // Number of steps in the swipe
    uint8_t delay_per_step = duration_ms / steps;

    // Calculate step increments
    int16_t x_step = (x_end - x_start) / steps;
    int16_t y_step = (y_end - y_start) / steps;

    uint16_t current_x = x_start;
    uint16_t current_y = y_start;

    // Send initial touch down
    build_touchscreen_report(&report, 0, 1, current_x, current_y, 100);
    app_hid_send_touchscreen_report(&report);

    // Send intermediate positions
    for (uint8_t i = 1; i < steps; i++) {
        current_x += x_step;
        current_y += y_step;

        build_touchscreen_report(&report, 0, 1, current_x, current_y, 100);
        app_hid_send_touchscreen_report(&report);

        delay_n_ms(delay_per_step);
    }

    // Send final position
    build_touchscreen_report(&report, 0, 1, x_end, y_end, 100);
    app_hid_send_touchscreen_report(&report);

    // Send touch up event
    delay_n_ms(10);
    build_touchscreen_report(&report, 0, 0, x_end, y_end, 0);
    app_hid_send_touchscreen_report(&report);
}

/**
 * @brief Send touch screen report via HID
 * @param report Pointer to touch screen report
 */
void app_hid_send_touchscreen_report(hid_touchscreen_report_t* report)
{
    NS_LOG_DEBUG("Sending touchscreen report: contact_id=%d, tip=%d, x=%d, y=%d, pressure=%d\r\n",
                 report->contact.contact_id,
                 report->contact.tip_switch,
                 report->contact.x,
                 report->contact.y,
                 report->contact.pressure);

    // This function will be implemented in app_hid.c
    // It needs to send the report with Report ID 4
    void app_hid_send_report_id(uint8_t report_id, uint8_t* data, uint16_t len);
    app_hid_send_report_id(4, (uint8_t*)report, sizeof(hid_touchscreen_report_t));
}
