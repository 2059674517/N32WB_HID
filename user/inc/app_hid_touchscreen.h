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

// Screen dimensions (logical coordinates)
#define SCREEN_WIDTH  32767   // X axis maximum (0-32767)
#define SCREEN_HEIGHT 32767   // Y axis maximum (0-32767)

// Maximum number of simultaneous touch points
#define MAX_TOUCH_POINTS 3  // Support up to 3 fingers

// Multi-touch report size: 3 touches * 5 bytes = 15 bytes
#define APP_HID_MULTITOUCH_REPORT_LEN  (MAX_TOUCH_POINTS * 5)

// Single touch contact structure (5 bytes per contact)
typedef struct __attribute__((packed)) {
    uint8_t tip_switch : 1;     // Finger touching (1) or not (0)
    uint8_t contact_id : 7;     // Contact identifier (0-127)
    uint16_t x;                 // X coordinate (0-32767)
    uint16_t y;                 // Y coordinate (0-32767)
} hid_touch_point_t;

// Multi-touch report structure (16 bytes total)
typedef struct __attribute__((packed)) {
    hid_touch_point_t touches[MAX_TOUCH_POINTS];  // 3 * 5 = 15 bytes
    //uint8_t contact_count;                        // Number of active contacts (0-3)
} hid_multitouch_report_t;

/**
 * @brief Send multi-touch screen event with multiple touch points
 * @param touches Array of touch points
 * @param count Number of active touch points (1-5)
 */
void app_hid_send_multitouch(const hid_touch_point_t* touches, uint8_t count);

/**
 * @brief Send single touch screen event (convenience function)
 * @param contact_id Contact identifier (0-4)
 * @param is_touching 1 if touching, 0 if not
 * @param x X coordinate (0-4095)
 * @param y Y coordinate (0-4095)
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

void app_multi_touchscreen_swipe(uint16_t count,uint16_t* x_start, uint16_t* y_start,
                           uint16_t* x_end, uint16_t* y_end,
                           uint16_t duration_ms);

/**
 * @brief Simulate a two-finger pinch gesture
 * @param center_x Center X coordinate
 * @param center_y Center Y coordinate
 * @param start_distance Starting distance between fingers
 * @param end_distance Ending distance between fingers
 * @param duration_ms Duration of pinch in milliseconds
 */
void app_touchscreen_pinch(uint16_t center_x, uint16_t center_y,
                           uint16_t start_distance, uint16_t end_distance,
                           uint16_t duration_ms);

/**
 * @brief Simulate a two-finger rotate gesture
 * @param center_x Center X coordinate
 * @param center_y Center Y coordinate
 * @param radius Radius of rotation
 * @param angle_degrees Rotation angle in degrees
 * @param duration_ms Duration of rotation in milliseconds
 */
void app_touchscreen_rotate(uint16_t center_x, uint16_t center_y,
                            uint16_t radius, int16_t angle_degrees,
                            uint16_t duration_ms);

/**
 * @brief Simulate multiple simultaneous touches
 * @param finger_count Number of fingers (1-5)
 * @param x_coords Array of X coordinates for each finger
 * @param y_coords Array of Y coordinates for each finger
 */
void app_touchscreen_multi_tap(uint8_t finger_count,
                               const uint16_t* x_coords,
                               const uint16_t* y_coords);

// ============================================================================
// High-Level Gesture APIs (Easy to use)
// ============================================================================

/**
 * @brief Perform zoom gesture (pinch in/out)
 * @param zoom_level Zoom level (1-10, higher = more zoom change)
 * @param is_zoom_in 1 = zoom in (pinch out), 0 = zoom out (pinch in)
 */
void app_gesture_zoom(uint8_t zoom_level, uint8_t is_zoom_in);

/**
 * @brief Perform rotation gesture
 * @param angle_degrees Rotation angle in degrees (-180 to +180)
 *                      Positive = clockwise, Negative = counter-clockwise
 */
void app_gesture_rotate(int16_t angle_degrees);

/**
 * @brief Perform three-finger swipe for screenshot
 * @param start_percent Starting position as percentage of screen height (0-100)
 *                      Example: 20 = start at 20% of screen height
 *                      Swipe will move 30% of screen height
 */
void app_gesture_screenshot(uint8_t start_percent);

/**
 * @brief Perform single-finger swipe
 * @param distance_pixels Distance to swipe in pixels (positive = down/right, negative = up/left)
 * @param is_vertical 1 = vertical swipe, 0 = horizontal swipe
 */
void app_gesture_swipe(int16_t distance_pixels, uint8_t is_vertical);

/**
 * @brief Swipe up from bottom edge (common gesture for "show home" or "multitask")
 */
void app_gesture_swipe_up_from_bottom(void);

/**
 * @brief Swipe down from top edge (common gesture for "notification center")
 */
void app_gesture_swipe_down_from_top(void);

/**
 * @brief Swipe right from left edge (common gesture for "back" or "menu")
 */
void app_gesture_swipe_right_from_left(void);

/**
 * @brief Swipe left from right edge (common gesture for "back" or "forward")
 */
void app_gesture_swipe_left_from_right(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_HID_TOUCHSCREEN_H__ */
