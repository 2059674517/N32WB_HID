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
 * @file app_hid_touchscreen.c
 * @author Nations Firmware Team
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */

#include "app_hid_touchscreen.h"
#include <string.h>
#include "app_profile/app_hid.h"
#include "ns_delay.h"
#include "ns_log.h"
#include "hogp/hogpd/api/hogpd_task.h"
#include "prf.h"
#include "ke_msg.h"
#include "co_utils.h"

// Touch screen report size (must match HID descriptor)
#define APP_HID_TOUCHSCREEN_REPORT_LEN  10  // 1(status) + 2(x) + 2(y) + 1(pressure) + 1(width) + 1(height) + 1(count) + 1(max)

// External HID environment
extern struct app_hid_env_tag app_hid_env;

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
 * @brief Send touch screen report via HID with proper format
 * @param contact_id Contact identifier (which finger)
 * @param is_touching Whether the finger is touching
 * @param x X coordinate
 * @param y Y coordinate
 * @param pressure Pressure value
 */
void app_hid_send_touchscreen(uint8_t contact_id, uint8_t is_touching,
                              uint16_t x, uint16_t y, uint8_t pressure)
{
    NS_LOG_INFO("Touchscreen: id=%d, touch=%d, x=%d, y=%d, p=%d\r\n",
                contact_id, is_touching, x, y, pressure);

    if (app_hid_env.state != APP_HID_READY) {
        NS_LOG_WARNING("HID not ready for touchscreen\r\n");
        return;
    }

    if (app_hid_env.nb_report == 0) {
        NS_LOG_WARNING("No reports available for touchscreen\r\n");
        return;
    }

    // Build the touch report buffer matching HID descriptor format
    uint8_t report[APP_HID_TOUCHSCREEN_REPORT_LEN];
    memset(report, 0, sizeof(report));

    // Byte 0: Contact status (contact_id:4, tip:1, in_range:1, valid:1, padding:1)
    report[0] = (contact_id & 0x0F) |
                (is_touching ? 0x10 : 0x00) |  // tip_switch
                (is_touching ? 0x20 : 0x00) |  // in_range
                0x40;  // touch_valid always 1 when sending

    // Bytes 1-2: X coordinate (little-endian)
    report[1] = x & 0xFF;
    report[2] = (x >> 8) & 0xFF;

    // Bytes 3-4: Y coordinate (little-endian)
    report[3] = y & 0xFF;
    report[4] = (y >> 8) & 0xFF;

    // Byte 5: Pressure
    report[5] = pressure;

    // Byte 6: Width
    report[6] = 20;  // Default width

    // Byte 7: Height
    report[7] = 20;  // Default height

    // Byte 8: Contact count
    report[8] = is_touching ? 1 : 0;

    // Byte 9: Max contact count
    report[9] = 10;

    // Send the report using the HID profile
    struct hogpd_report_upd_req * req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                      prf_get_task_from_id(TASK_ID_HOGPD),
                                                      TASK_APP,
                                                      hogpd_report_upd_req,
                                                      APP_HID_TOUCHSCREEN_REPORT_LEN);

    req->conidx = app_hid_env.conidx;
    req->report.hid_idx = app_hid_env.conidx;
    req->report.type = HOGPD_REPORT;
    req->report.idx = 3;  // Report index 3 for Report ID 4
    req->report.length = APP_HID_TOUCHSCREEN_REPORT_LEN;

    memcpy(&req->report.value[0], report, APP_HID_TOUCHSCREEN_REPORT_LEN);

    NS_LOG_DEBUG("Sending touch report to HOGPD: idx=%d, len=%d\r\n",
                 req->report.idx, req->report.length);

    ke_msg_send(req);
    app_hid_env.nb_report--;
}

/**
 * @brief Simulate a simple tap
 */
void app_touchscreen_tap(uint16_t x, uint16_t y)
{
    NS_LOG_INFO("Touchscreen TAP at (%d, %d)\r\n", x, y);

    // Touch down
    app_hid_send_touchscreen(0, 1, x, y, 100);
    delay_n_ms(50);

    // Touch up
    app_hid_send_touchscreen(0, 0, x, y, 0);
}

/**
 * @brief Simulate a swipe gesture
 */
void app_touchscreen_swipe(uint16_t x_start, uint16_t y_start,
                           uint16_t x_end, uint16_t y_end,
                           uint16_t duration_ms)
{
    NS_LOG_INFO("Touchscreen SWIPE from (%d,%d) to (%d,%d)\r\n",
                x_start, y_start, x_end, y_end);

    uint8_t steps = 10;
    uint16_t delay_per_step = duration_ms / steps;

    int16_t x_step = (x_end - x_start) / steps;
    int16_t y_step = (y_end - y_start) / steps;

    uint16_t current_x = x_start;
    uint16_t current_y = y_start;

    // Touch down
    app_hid_send_touchscreen(0, 1, current_x, current_y, 100);

    // Move through intermediate points
    for (uint8_t i = 1; i < steps; i++) {
        current_x += x_step;
        current_y += y_step;
        app_hid_send_touchscreen(0, 1, current_x, current_y, 100);
        delay_n_ms(delay_per_step);
    }

    // Final position
    app_hid_send_touchscreen(0, 1, x_end, y_end, 100);
    delay_n_ms(10);

    // Touch up
    app_hid_send_touchscreen(0, 0, x_end, y_end, 0);
}
