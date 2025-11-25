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
#include <math.h>
#include "app_profile/app_hid.h"
#include "ns_delay.h"
#include "ns_log.h"
#include "hogp/hogpd/api/hogpd_task.h"
#include "prf.h"
#include "ke_msg.h"
#include "co_utils.h"

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
        uint8_t offset =i * 5;  // 5 bytes per touch

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
            report[offset] = ((i) << 1);  // Contact ID without tip switch
						NS_LOG_WARNING("\r\ncontact_id:%d\r\n", report[offset]);
        }
    }

    // Last byte: Contact count
    

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
    touch.x = (x > SCREEN_WIDTH) ? SCREEN_WIDTH : x;
    touch.y = (y > SCREEN_HEIGHT) ? SCREEN_HEIGHT : y;

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
        touches[i].contact_id = i;
        touches[i].x = (x_coords[i] > SCREEN_WIDTH) ? SCREEN_WIDTH : x_coords[i];
        touches[i].y = (y_coords[i] > SCREEN_HEIGHT) ? SCREEN_HEIGHT : y_coords[i];
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

    uint8_t steps = 10;
    uint16_t delay_per_step = duration_ms / steps;
	
		int16_t x_step[10];
    int16_t y_step[10];
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
		
		for(int i=0; i<count; i++){
			app_touchscreen_multi(i+1,x_start,y_start);
			delay_n_ms(10);
		}
		
    // Move through intermediate points
    for (uint8_t i = 1; i < steps; i++) {
				for(int i=0; i<count; i++){
					current_x[i] += x_step[i];
					current_y[i] += y_step[i];
				}
        app_touchscreen_multi(count,current_x,current_y);
        delay_n_ms(delay_per_step);
    }

    // Final position
    app_touchscreen_multi(count,x_end,y_end);
    delay_n_ms(10);

    // Touch up
//		for(int i=count-1; i>=0; i--){
//			app_touchscreen_multi(i,x_end,y_end);
//			delay_n_ms(10);
//		}
		
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
        touches[i].x = (x_coords[i] > SCREEN_WIDTH) ? SCREEN_WIDTH : x_coords[i];
        touches[i].y = (y_coords[i] > SCREEN_HEIGHT) ? SCREEN_HEIGHT : y_coords[i];
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
        touches[1].x = (center_x + current_distance/2 > SCREEN_WIDTH) ? SCREEN_WIDTH : center_x + current_distance/2;
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
    float angle_step_radians = (angle_degrees * 3.14159f / 180.0f) / steps;

    hid_touch_point_t touches[2];

    // Initialize two fingers
    touches[0].tip_switch = 1;
    touches[0].contact_id = 0;
    touches[1].tip_switch = 1;
    touches[1].contact_id = 1;

    // Perform rotate gesture with real multi-touch
    for (uint8_t i = 0; i <= steps; i++) {
        // Current rotation angle for this step
        float current_angle = angle_step_radians * i;

        // Finger 1: starts at 0 degrees (right side), rotates by current_angle
        // In screen coordinates: X+ is right, Y+ is down
        // Positive angle = clockwise rotation
        float angle1 = current_angle;
        int16_t x1 = center_x + (int16_t)(radius * cosf(angle1));
        int16_t y1 = center_y + (int16_t)(radius * sinf(angle1));

        // Finger 2: starts at 180 degrees (left side), rotates by same angle
        float angle2 = 3.14159f + current_angle;
        int16_t x2 = center_x + (int16_t)(radius * cosf(angle2));
        int16_t y2 = center_y + (int16_t)(radius * sinf(angle2));

        // Clamp to valid range
        touches[0].x = (x1 < 0) ? 0 : ((x1 > SCREEN_WIDTH) ? SCREEN_WIDTH : x1);
        touches[0].y = (y1 < 0) ? 0 : ((y1 > SCREEN_HEIGHT) ? SCREEN_HEIGHT : y1);
        touches[1].x = (x2 < 0) ? 0 : ((x2 > SCREEN_WIDTH) ? SCREEN_WIDTH : x2);
        touches[1].y = (y2 < 0) ? 0 : ((y2 > SCREEN_HEIGHT) ? SCREEN_HEIGHT : y2);

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

// ============================================================================
// High-Level Gesture APIs Implementation
// ============================================================================

// Simple pseudo-random number generator for offsets
static uint32_t rand_seed = 12345;
static int16_t get_random_offset(int16_t range) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (int16_t)((rand_seed / 65536) % range) - (range / 2);
}

/**
 * @brief Perform zoom gesture (pinch in/out)
 * @param zoom_level Zoom level (1-10, higher = more zoom change)
 * @param is_zoom_in 1 = zoom in (pinch out), 0 = zoom out (pinch in)
 */
void app_gesture_zoom(uint8_t zoom_level, uint8_t is_zoom_in)
{
    // Clamp zoom level
    if (zoom_level < 1) zoom_level = 1;
    if (zoom_level > 10) zoom_level = 10;

    // Center of screen
    uint16_t center_x = SCREEN_WIDTH / 2;
    uint16_t center_y = SCREEN_HEIGHT / 2;

    // Calculate distances based on zoom level
    // Base distance: 2000-10000, scaled by zoom_level
    uint16_t base_distance = 2000 + (zoom_level * 800);
    uint16_t start_distance, end_distance;

    if (is_zoom_in) {
        // Zoom in: fingers move apart (pinch out)
        start_distance = base_distance / 2;
        end_distance = base_distance;
    } else {
        // Zoom out: fingers move together (pinch in)
        start_distance = base_distance;
        end_distance = base_distance / 2;
    }

    // Duration: 300-800ms based on zoom level
    uint16_t duration_ms = 300 + (zoom_level * 50);

    NS_LOG_INFO("Zoom %s: level=%d, distance %d->%d\r\n",
                is_zoom_in ? "IN" : "OUT", zoom_level, start_distance, end_distance);

    app_touchscreen_pinch(center_x, center_y, start_distance, end_distance, duration_ms);
}

/**
 * @brief Perform rotation gesture
 * @param angle_degrees Rotation angle in degrees (-180 to +180)
 */
void app_gesture_rotate(int16_t angle_degrees)
{
    // Clamp angle
    if (angle_degrees < -180) angle_degrees = -180;
    if (angle_degrees > 180) angle_degrees = 180;

    // Center of screen
    uint16_t center_x = SCREEN_WIDTH / 2;
    uint16_t center_y = SCREEN_HEIGHT / 2;

    // Radius: 1/4 of screen width
    uint16_t radius = SCREEN_WIDTH / 4;

    // Duration: proportional to angle (min 300ms, max 1000ms)
    uint16_t duration_ms = 300 + ((angle_degrees > 0 ? angle_degrees : -angle_degrees) * 700 / 180);

    NS_LOG_INFO("Rotate: angle=%d deg, duration=%d ms\r\n", angle_degrees, duration_ms);

    app_touchscreen_rotate(center_x, center_y, radius, angle_degrees, duration_ms);
}

/**
 * @brief Perform three-finger swipe for screenshot
 * @param start_percent Starting position as percentage of screen height (0-100)
 */
void app_gesture_screenshot(uint8_t start_percent)
{
    // Clamp percentage
    if (start_percent > 70) start_percent = 70;  // Max 70% to ensure room for swipe

    // Calculate Y positions
    uint16_t y_start = (uint32_t)SCREEN_HEIGHT * start_percent / 100;
    uint16_t y_end = y_start + (SCREEN_HEIGHT * 30 / 100);  // Swipe 30% of screen height

    // X positions: evenly spaced across screen with random offsets
    uint16_t base_spacing = SCREEN_WIDTH / 4;
    uint16_t x_start[3], y_start_arr[3];
    uint16_t x_end[3], y_end_arr[3];

    for (uint8_t i = 0; i < 3; i++) {
        // Add random offset (±500 pixels)
        int16_t x_offset = get_random_offset(1000);
        int16_t y_offset_start = get_random_offset(400);
        int16_t y_offset_end = get_random_offset(400);

        x_start[i] = base_spacing + (i * base_spacing) + x_offset;
        y_start_arr[i] = y_start + y_offset_start;
        x_end[i] = x_start[i] + get_random_offset(200);  // Small horizontal movement
        y_end_arr[i] = y_end + y_offset_end;

        // Clamp to screen bounds
        if (x_start[i] > SCREEN_WIDTH) x_start[i] = SCREEN_WIDTH;
        if (x_end[i] > SCREEN_WIDTH) x_end[i] = SCREEN_WIDTH;
        if (y_start_arr[i] > SCREEN_HEIGHT) y_start_arr[i] = SCREEN_HEIGHT;
        if (y_end_arr[i] > SCREEN_HEIGHT) y_end_arr[i] = SCREEN_HEIGHT;
    }

    NS_LOG_INFO("Screenshot: 3-finger swipe from %d%% to %d%%\r\n",
                start_percent, start_percent + 30);

    app_multi_touchscreen_swipe(3, x_start, y_start_arr, x_end, y_end_arr, 300);
}

/**
 * @brief Perform single-finger swipe
 * @param distance_pixels Distance to swipe in pixels
 * @param is_vertical 1 = vertical swipe, 0 = horizontal swipe
 */
void app_gesture_swipe(int16_t distance_pixels, uint8_t is_vertical)
{
    uint16_t x_start, y_start, x_end, y_end;

    if (is_vertical) {
        // Vertical swipe: center X, start from middle
        x_start = SCREEN_WIDTH / 2;
        x_end = x_start;
        y_start = SCREEN_HEIGHT / 2;

        // Calculate end position
        int32_t y_end_calc = (int32_t)y_start + distance_pixels;
        if (y_end_calc < 0) y_end_calc = 0;
        if (y_end_calc > SCREEN_HEIGHT) y_end_calc = SCREEN_HEIGHT;
        y_end = (uint16_t)y_end_calc;
    } else {
        // Horizontal swipe: center Y, start from middle
        y_start = SCREEN_HEIGHT / 2;
        y_end = y_start;
        x_start = SCREEN_WIDTH / 2;

        // Calculate end position
        int32_t x_end_calc = (int32_t)x_start + distance_pixels;
        if (x_end_calc < 0) x_end_calc = 0;
        if (x_end_calc > SCREEN_WIDTH) x_end_calc = SCREEN_WIDTH;
        x_end = (uint16_t)x_end_calc;
    }

    // Duration: proportional to distance (min 200ms, max 600ms)
    uint16_t abs_distance = distance_pixels > 0 ? distance_pixels : -distance_pixels;
    uint16_t duration_ms = 200 + (abs_distance * 400 / SCREEN_WIDTH);
    if (duration_ms > 600) duration_ms = 600;

    NS_LOG_INFO("Swipe %s: distance=%d pixels\r\n",
                is_vertical ? "VERTICAL" : "HORIZONTAL", distance_pixels);

    app_touchscreen_swipe(x_start, y_start, x_end, y_end, duration_ms);
}

/**
 * @brief Swipe up from bottom edge
 */
void app_gesture_swipe_up_from_bottom(void)
{
    // Start from 90% of screen height, swipe up to 30%
    uint16_t x_start = SCREEN_WIDTH / 2;
    uint16_t x_end = x_start;
    uint16_t y_start = (SCREEN_HEIGHT * 10) / 10;  // 90%
    uint16_t y_end = (SCREEN_HEIGHT * 7) / 10;    // 30%

    NS_LOG_INFO("Gesture: Swipe UP from bottom edge\r\n");
    app_touchscreen_swipe(x_start, y_start, x_end, y_end, 400);
}

/**
 * @brief Swipe down from top edge
 */
void app_gesture_swipe_down_from_top(void)
{
    // Start from 10% of screen height, swipe down to 70%
    uint16_t x_start = SCREEN_WIDTH / 2;
    uint16_t x_end = x_start;
    uint16_t y_start = 0;        // 10%
    uint16_t y_end = (SCREEN_HEIGHT * 2) / 10;    // 70%

    NS_LOG_INFO("Gesture: Swipe DOWN from top edge\r\n");
    app_touchscreen_swipe(x_start, y_start, x_end, y_end, 400);
}

/**
 * @brief Swipe right from left edge
 */
void app_gesture_swipe_right_from_left(void)
{
    // Start from 10% of screen width, swipe right to 70%
    uint16_t x_start = 0;         // 10%
    uint16_t x_end = (SCREEN_WIDTH * 5) / 10;     // 70%
    uint16_t y_start = SCREEN_HEIGHT / 2;
    uint16_t y_end = y_start;

    NS_LOG_INFO("Gesture: Swipe RIGHT from left edge\r\n");
    app_touchscreen_swipe(x_start, y_start, x_end, y_end, 400);
}

/**
 * @brief Swipe left from right edge
 */
void app_gesture_swipe_left_from_right(void)
{
    // Start from 90% of screen width, swipe left to 30%
    uint16_t x_start = SCREEN_WIDTH ;   // 90%
    uint16_t x_end = (SCREEN_WIDTH * 5) / 10;     // 30%
    uint16_t y_start = SCREEN_HEIGHT / 2;
    uint16_t y_end = y_start;

    NS_LOG_INFO("Gesture: Swipe LEFT from right edge\r\n");
    app_touchscreen_swipe(x_start, y_start, x_end, y_end, 400);
}
