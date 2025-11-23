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

// Multi-touch report size: 3 touches * 5 bytes + 1 byte count = 16 bytes
#define APP_HID_MULTITOUCH_REPORT_LEN  16

// External HID environment
extern struct app_hid_env_tag app_hid_env;

/**
 * @brief Send multi-touch screen report via HID (supports 1-3 touches)
 * @param touches Array of touch points
 * @param count Number of active touch points (0-3)
 */
void app_hid_send_multitouch(const hid_touch_point_t* touches, uint8_t count)
{
    NS_LOG_INFO("Multi-touch: count=%d\r\n", count);

    if (app_hid_env.state != APP_HID_READY) {
        NS_LOG_WARNING("HID not ready for touchscreen\r\n");
        return;
    }

    if (app_hid_env.nb_report == 0) {
        NS_LOG_WARNING("No reports available for touchscreen\r\n");
        return;
    }

    // Validate count
    if (count > MAX_TOUCH_POINTS) {
        NS_LOG_WARNING("Too many touch points: %d (max %d)\r\n", count, MAX_TOUCH_POINTS);
        count = MAX_TOUCH_POINTS;
    }

    // Build the multi-touch report buffer
    uint8_t report[APP_HID_MULTITOUCH_REPORT_LEN];
    memset(report, 0, sizeof(report));

    // Fill in touch data for each finger
    for (uint8_t i = 0; i < MAX_TOUCH_POINTS; i++) {
        uint8_t offset = i * 5;  // 5 bytes per touch

        if (i < count && touches != NULL) {
            // Active touch point
            const hid_touch_point_t* touch = &touches[i];

            // Byte 0: Tip switch (1 bit) + Contact ID (7 bits)
            report[offset] = (touch->tip_switch ? 0x01 : 0x00) |
                           ((touch->contact_id & 0x7F) << 1);

            // Bytes 1-2: X coordinate (little-endian)
            report[offset + 1] = touch->x & 0xFF;
            report[offset + 2] = (touch->x >> 8) & 0xFF;

            // Bytes 3-4: Y coordinate (little-endian)
            report[offset + 3] = touch->y & 0xFF;
            report[offset + 4] = (touch->y >> 8) & 0xFF;

            NS_LOG_WARNING("Touch %d: id=%d, tip=%d, x=%d, y=%d\r\n",
                        i, touch->contact_id, touch->tip_switch, touch->x, touch->y);
        } else {
            // No touch - set contact ID but no tip switch
            report[offset] = ((i+1) << 1);  // Contact ID without tip switch
						NS_LOG_WARNING("\r\ncontact_id:%d\r\n", report[offset]);
        }
    }

    // Last byte: Contact count
    report[15] = count;

    // Send the report using the HID profile
    struct hogpd_report_upd_req * req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                      prf_get_task_from_id(TASK_ID_HOGPD),
                                                      TASK_APP,
                                                      hogpd_report_upd_req,
                                                      APP_HID_MULTITOUCH_REPORT_LEN);

    req->conidx = app_hid_env.conidx;
    req->report.hid_idx = app_hid_env.conidx;
    req->report.type = HOGPD_REPORT;
    req->report.idx = 3;  // Report index 3 for Report ID 4
    req->report.length = APP_HID_MULTITOUCH_REPORT_LEN;

    memcpy(&req->report.value[0], report, APP_HID_MULTITOUCH_REPORT_LEN);

    NS_LOG_DEBUG("Sending multi-touch report to HOGPD: idx=%d, len=%d\r\n",
                 req->report.idx, req->report.length);

    ke_msg_send(req);
    app_hid_env.nb_report--;
}

/**
 * @brief Send single touch screen event (convenience function)
 * @param contact_id Contact identifier (0-2)
 * @param is_touching Whether the finger is touching
 * @param x X coordinate (0-32767)
 * @param y Y coordinate (0-32767)
 * @param pressure Pressure value (not used in current implementation)
 */
void app_hid_send_touchscreen(uint8_t contact_id, uint8_t is_touching,
                              uint16_t x, uint16_t y, uint8_t pressure)
{
    hid_touch_point_t touch;

    // Validate contact ID
    if (contact_id >= MAX_TOUCH_POINTS) {
        NS_LOG_WARNING("Invalid contact ID: %d (max %d)\r\n", contact_id, MAX_TOUCH_POINTS - 1);
        contact_id = 0;
    }

    // Fill touch point data
    touch.tip_switch = is_touching ? 1 : 0;
    touch.contact_id = contact_id;
    touch.x = (x > 32767) ? 32767 : x;
    touch.y = (y > 32767) ? 32767 : y;

    // Send as single touch
    if (is_touching) {
        app_hid_send_multitouch(&touch, 1);
    } else {
        app_hid_send_multitouch(NULL, 0);  // No touches
    }
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
void app_touchscreen_threefinger_swipe(uint16_t x_start, uint16_t y_start,
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
		app_hid_send_touchscreen(1, 1, current_x+1000, current_y, 100);
		app_hid_send_touchscreen(2, 1, current_x+2000, current_y, 100);

    // Move through intermediate points
    for (uint8_t i = 1; i < steps; i++) {
        current_x += x_step;
        current_y += y_step;
        app_hid_send_touchscreen(0, 1, current_x, current_y, 100);
				app_hid_send_touchscreen(1, 1, current_x+1000, current_y, 100);
				app_hid_send_touchscreen(2, 1, current_x+2000, current_y, 100);
				NS_LOG_INFO("Touchscreen SWIPE from (%d,%d) to (%d,%d)\r\n",
                current_x, current_y, current_x, current_y);
        delay_n_ms(delay_per_step);
    }

    // Final position
    app_hid_send_touchscreen(0, 1, x_end, y_end, 100);
		app_hid_send_touchscreen(1, 1, x_end+1000, y_end, 100);
		app_hid_send_touchscreen(2, 1, x_end+2000, y_end, 100);
    delay_n_ms(10);

    // Touch up
    app_hid_send_touchscreen(0, 0, x_end, y_end, 0);
		app_hid_send_touchscreen(1, 0, x_end+1000, y_end, 0);
		app_hid_send_touchscreen(2, 0, x_end+2000, y_end, 0);
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
    app_hid_send_touchscreen(0, 0, x_end, y_end, 100);
}

void app_touchscreen_multi(uint8_t finger_count,
                               const uint16_t* x_coords,
                               const uint16_t* y_coords)
{
    if (finger_count == 0 || finger_count > MAX_TOUCH_POINTS) {
        NS_LOG_WARNING("Invalid finger count: %d\r\n", finger_count);
        return;
    }

    NS_LOG_INFO("Multi-tap with %d fingers\r\n", finger_count);

    hid_touch_point_t touches[MAX_TOUCH_POINTS];

    // Prepare touch points
    for (uint8_t i = 0; i < finger_count; i++) {
        touches[i].tip_switch = 1;
        touches[i].contact_id = i+1;
        touches[i].x = (x_coords[i] > 32767) ? 32767 : x_coords[i];
        touches[i].y = (y_coords[i] > 32767) ? 32767 : y_coords[i];
    }

    // Touch down all fingers simultaneously
    app_hid_send_multitouch(touches, finger_count);
}


/**
 * @brief Simulate a swipe gesture
 */
void app_multi_touchscreen_swipe(uint16_t count,uint16_t* x_start, uint16_t* y_start,
                           uint16_t* x_end, uint16_t* y_end,
                           uint16_t duration_ms)
{
    NS_LOG_INFO("multiTouchscreen SWIPE");

    uint8_t steps = 30;
    uint16_t delay_per_step = duration_ms / steps;
	
		int16_t x_step[30];
    int16_t y_step[30];
		uint16_t current_x[3];
    uint16_t current_y[3];
		
		for(int i=0; i<count; i++){
			x_step[i] = (x_end[i] - x_start[i]) / steps;
      y_step[i] = (y_end[i] - y_start[i]) / steps;
		}

		for(int i=0; i<count; i++){
			current_x[i] = x_start[i];
			current_y[i] = y_start[i];
		}
	
    // Touch down
		app_hid_send_multitouch(NULL, 0);
		delay_n_ms(10);
		app_touchscreen_multi(1,x_start,y_start);
		delay_n_ms(10);
		app_touchscreen_multi(2,x_start,y_start);
		delay_n_ms(10);
		app_touchscreen_multi(3,x_start,y_start);
		delay_n_ms(10);

    // Move through intermediate points
    for (uint8_t i = 1; i < steps; i++) {
				for(int i=0; i<count; i++){
					current_x[i] += x_step[i];
					current_y[i] += y_step[i];
				}
        app_touchscreen_multi(3,current_x,current_y);
        delay_n_ms(delay_per_step);
    }

    // Final position
    app_touchscreen_multi(3,x_end,y_end);
    delay_n_ms(10);

    // Touch up
		app_touchscreen_multi(2,x_end,y_end);
    delay_n_ms(10);
		app_touchscreen_multi(1,x_end,y_end);
    delay_n_ms(10);
		app_touchscreen_multi(0,x_end,y_end);
    delay_n_ms(10);
		app_hid_send_multitouch(NULL, 0);
		//app_hid_send_multitouch(NULL, 0);
}

/**
 * @brief Simulate multiple simultaneous touches (true multi-touch)
 * @param finger_count Number of fingers (1-3)
 * @param x_coords Array of X coordinates for each finger
 * @param y_coords Array of Y coordinates for each finger
 */
void app_touchscreen_multi_tap(uint8_t finger_count,
                               const uint16_t* x_coords,
                               const uint16_t* y_coords)
{
    if (finger_count == 0 || finger_count > MAX_TOUCH_POINTS) {
        NS_LOG_WARNING("Invalid finger count: %d\r\n", finger_count);
        return;
    }

    NS_LOG_INFO("Multi-tap with %d fingers\r\n", finger_count);

    hid_touch_point_t touches[MAX_TOUCH_POINTS];

    // Prepare touch points
    for (uint8_t i = 0; i < finger_count; i++) {
        touches[i].tip_switch = 1;
        touches[i].contact_id = i;
        touches[i].x = (x_coords[i] > 32767) ? 32767 : x_coords[i];
        touches[i].y = (y_coords[i] > 32767) ? 32767 : y_coords[i];
    }

    // Touch down all fingers simultaneously
    app_hid_send_multitouch(touches, finger_count);
    delay_n_ms(100);

    // Touch up all fingers
    app_hid_send_multitouch(NULL, 0);
}

/**
 * @brief Simulate a two-finger pinch gesture (real multi-touch)
 * @param center_x Center X coordinate
 * @param center_y Center Y coordinate
 * @param start_distance Starting distance between fingers
 * @param end_distance Ending distance between fingers
 * @param duration_ms Duration of pinch in milliseconds
 */
void app_touchscreen_pinch(uint16_t center_x, uint16_t center_y,
                           uint16_t start_distance, uint16_t end_distance,
                           uint16_t duration_ms)
{
    NS_LOG_INFO("Pinch gesture at (%d,%d), distance %d->%d\r\n",
                center_x, center_y, start_distance, end_distance);

    uint8_t steps = 10;
    uint16_t delay_per_step = duration_ms / steps;
    int16_t distance_step = (end_distance - start_distance) / steps;

    hid_touch_point_t touches[2];

    // Initialize two fingers
    touches[0].tip_switch = 1;
    touches[0].contact_id = 0;
    touches[1].tip_switch = 1;
    touches[1].contact_id = 1;

    // Perform pinch gesture with real multi-touch
    for (uint8_t i = 0; i <= steps; i++) {
        uint16_t current_distance = start_distance + (distance_step * i);

        // Position fingers horizontally around center
        touches[0].x = (center_x > current_distance/2) ? center_x - current_distance/2 : 0;
        touches[0].y = center_y;
        touches[1].x = (center_x + current_distance/2 > 32767) ? 32767 : center_x + current_distance/2;
        touches[1].y = center_y;

        // Send both touches simultaneously
        app_hid_send_multitouch(touches, 2);

        if (i < steps) {
            delay_n_ms(delay_per_step);
        }
    }

    delay_n_ms(50);

    // Release both fingers
    app_hid_send_multitouch(NULL, 0);
}

/**
 * @brief Simulate a two-finger rotate gesture (real multi-touch)
 * @param center_x Center X coordinate
 * @param center_y Center Y coordinate
 * @param radius Radius of rotation
 * @param angle_degrees Rotation angle in degrees (positive = clockwise)
 * @param duration_ms Duration of rotation in milliseconds
 */
void app_touchscreen_rotate(uint16_t center_x, uint16_t center_y,
                            uint16_t radius, int16_t angle_degrees,
                            uint16_t duration_ms)
{
    NS_LOG_INFO("Rotate gesture at (%d,%d), radius %d, angle %d deg\r\n",
                center_x, center_y, radius, angle_degrees);

    uint8_t steps = 20;
    uint16_t delay_per_step = duration_ms / steps;
    float angle_step = (angle_degrees * 3.14159f / 180.0f) / steps;

    hid_touch_point_t touches[2];

    // Initialize two fingers
    touches[0].tip_switch = 1;
    touches[0].contact_id = 0;
    touches[1].tip_switch = 1;
    touches[1].contact_id = 1;

    // Perform rotate gesture with real multi-touch
    for (uint8_t i = 0; i <= steps; i++) {
        float current_angle = angle_step * i;

        // Simple approximation for rotation
        // Finger 1 position
        int16_t x1 = center_x + radius;
        int16_t y1 = center_y;

        // Rotate finger 1 position
        if (i > 0) {
            // Approximate rotation using linear interpolation
            float progress = (float)i / steps;
            x1 = center_x + (int16_t)(radius * (1.0f - progress * 0.5f));
            y1 = center_y + (int16_t)(radius * progress);
        }

        // Finger 2 position (opposite side)
        int16_t x2 = center_x - (x1 - center_x);
        int16_t y2 = center_y - (y1 - center_y);

        // Clamp to valid range
        touches[0].x = (x1 < 0) ? 0 : ((x1 > 32767) ? 32767 : x1);
        touches[0].y = (y1 < 0) ? 0 : ((y1 > 32767) ? 32767 : y1);
        touches[1].x = (x2 < 0) ? 0 : ((x2 > 32767) ? 32767 : x2);
        touches[1].y = (y2 < 0) ? 0 : ((y2 > 32767) ? 32767 : y2);

        // Send both touches simultaneously
        app_hid_send_multitouch(touches, 2);

        if (i < steps) {
            delay_n_ms(delay_per_step);
        }
    }

    delay_n_ms(50);

    // Release both fingers
    app_hid_send_multitouch(NULL, 0);
}
