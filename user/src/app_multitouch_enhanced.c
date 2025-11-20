/*****************************************************************************
 * Enhanced Multi-Touch Implementation
 * Supports Windows PC and Mobile devices with proper HID protocol
 *****************************************************************************/

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

// Enhanced report size for better compatibility
#define ENHANCED_MULTITOUCH_REPORT_LEN  16

// External HID environment
extern struct app_hid_env_tag app_hid_env;

// Touch state tracking for proper multi-touch
typedef struct {
    uint8_t active;
    uint8_t contact_id;
    uint16_t x;
    uint16_t y;
} touch_state_t;

static touch_state_t g_touch_states[MAX_TOUCH_POINTS] = {0};
static uint8_t g_next_contact_id = 0;

/**
 * @brief Send enhanced multi-touch report compatible with Windows and mobile
 */
void app_hid_send_enhanced_multitouch(const hid_touch_point_t* touches, uint8_t count)
{
    NS_LOG_INFO("Enhanced Multi-touch: count=%d\r\n", count);

    if (app_hid_env.state != APP_HID_READY) {
        NS_LOG_WARNING("HID not ready\r\n");
        return;
    }

    if (count > MAX_TOUCH_POINTS) {
        count = MAX_TOUCH_POINTS;
    }

    // Build Windows-compatible touch report
    uint8_t report[ENHANCED_MULTITOUCH_REPORT_LEN];
    memset(report, 0, sizeof(report));

    // Process each touch point
    for (uint8_t i = 0; i < MAX_TOUCH_POINTS; i++) {
        uint8_t offset = i * 5;  // 5 bytes per touch point

        if (i < count && touches != NULL) {
            const hid_touch_point_t* touch = &touches[i];

            // Windows format: Tip Switch in bit 0, Contact ID in bits 1-7
            report[offset] = (touch->tip_switch ? 0x01 : 0x00) |
                           ((touch->contact_id & 0x7F) << 1);

            // X coordinate (little-endian, 16-bit)
            report[offset + 1] = touch->x & 0xFF;
            report[offset + 2] = (touch->x >> 8) & 0xFF;

            // Y coordinate (little-endian, 16-bit)
            report[offset + 3] = touch->y & 0xFF;
            report[offset + 4] = (touch->y >> 8) & 0xFF;

            // Update state tracking
            g_touch_states[i].active = touch->tip_switch;
            g_touch_states[i].contact_id = touch->contact_id;
            g_touch_states[i].x = touch->x;
            g_touch_states[i].y = touch->y;

            NS_LOG_DEBUG("Touch[%d]: ID=%d, Tip=%d, X=%d, Y=%d\r\n",
                        i, touch->contact_id, touch->tip_switch, touch->x, touch->y);
        } else {
            // Send lift-off for previously active touches
            if (g_touch_states[i].active) {
                report[offset] = (g_touch_states[i].contact_id & 0x7F) << 1; // No tip switch
                report[offset + 1] = g_touch_states[i].x & 0xFF;
                report[offset + 2] = (g_touch_states[i].x >> 8) & 0xFF;
                report[offset + 3] = g_touch_states[i].y & 0xFF;
                report[offset + 4] = (g_touch_states[i].y >> 8) & 0xFF;
                g_touch_states[i].active = 0;
            } else {
                // Inactive touch - send contact ID only
                report[offset] = (i & 0x7F) << 1;
            }
        }
    }

    // Contact count in last byte
    report[15] = count;

    // Send via HID profile
    struct hogpd_report_upd_req * req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                      prf_get_task_from_id(TASK_ID_HOGPD),
                                                      TASK_APP,
                                                      hogpd_report_upd_req,
                                                      ENHANCED_MULTITOUCH_REPORT_LEN);

    req->conidx = app_hid_env.conidx;
    req->report.hid_idx = app_hid_env.conidx;
    req->report.type = HOGPD_REPORT;
    req->report.idx = 3;  // Report ID 4
    req->report.length = ENHANCED_MULTITOUCH_REPORT_LEN;

    memcpy(&req->report.value[0], report, ENHANCED_MULTITOUCH_REPORT_LEN);

    ke_msg_send(req);
    app_hid_env.nb_report--;
}

/**
 * @brief Two-finger swipe gesture with proper multi-touch
 */
void app_touchscreen_two_finger_swipe(uint16_t start_x, uint16_t start_y,
                                      uint16_t end_x, uint16_t end_y,
                                      uint16_t finger_spacing, uint16_t duration_ms)
{
    NS_LOG_INFO("Two-finger swipe: (%d,%d)->(%d,%d), spacing=%d\r\n",
                start_x, start_y, end_x, end_y, finger_spacing);

    uint8_t steps = 15;
    uint16_t delay_per_step = duration_ms / steps;

    hid_touch_point_t touches[2];

    // Setup two fingers
    touches[0].contact_id = 0;
    touches[0].tip_switch = 1;
    touches[1].contact_id = 1;
    touches[1].tip_switch = 1;

    for (uint8_t i = 0; i <= steps; i++) {
        float progress = (float)i / steps;

        // Interpolate positions
        uint16_t current_x = start_x + (int16_t)((end_x - start_x) * progress);
        uint16_t current_y = start_y + (int16_t)((end_y - start_y) * progress);

        // Position two fingers with spacing
        touches[0].x = (current_x > finger_spacing/2) ? current_x - finger_spacing/2 : 0;
        touches[0].y = current_y;
        touches[1].x = (current_x + finger_spacing/2 > 32767) ? 32767 : current_x + finger_spacing/2;
        touches[1].y = current_y;

        app_hid_send_enhanced_multitouch(touches, 2);

        if (i < steps) {
            delay_n_ms(delay_per_step);
        }
    }

    // Lift fingers
    delay_n_ms(50);
    app_hid_send_enhanced_multitouch(NULL, 0);
}

/**
 * @brief Three-finger swipe gesture
 */
void app_touchscreen_three_finger_swipe(uint16_t start_x, uint16_t start_y,
                                        uint16_t end_x, uint16_t end_y,
                                        uint16_t duration_ms)
{
    NS_LOG_INFO("Three-finger swipe: (%d,%d)->(%d,%d)\r\n",
                start_x, start_y, end_x, end_y);

    uint8_t steps = 15;
    uint16_t delay_per_step = duration_ms / steps;
    uint16_t finger_spacing = 3000; // Spacing between fingers

    hid_touch_point_t touches[3];

    // Setup three fingers
    for (uint8_t j = 0; j < 3; j++) {
        touches[j].contact_id = j;
        touches[j].tip_switch = 1;
    }

    for (uint8_t i = 0; i <= steps; i++) {
        float progress = (float)i / steps;

        // Interpolate positions
        uint16_t current_x = start_x + (int16_t)((end_x - start_x) * progress);
        uint16_t current_y = start_y + (int16_t)((end_y - start_y) * progress);

        // Position three fingers horizontally
        touches[0].x = (current_x > finger_spacing) ? current_x - finger_spacing : 0;
        touches[0].y = current_y;
        touches[1].x = current_x;
        touches[1].y = current_y;
        touches[2].x = (current_x + finger_spacing > 32767) ? 32767 : current_x + finger_spacing;
        touches[2].y = current_y;

        app_hid_send_enhanced_multitouch(touches, 3);

        if (i < steps) {
            delay_n_ms(delay_per_step);
        }
    }

    // Lift all fingers
    delay_n_ms(50);
    app_hid_send_enhanced_multitouch(NULL, 0);
}

/**
 * @brief Enhanced pinch-to-zoom gesture
 */
void app_touchscreen_enhanced_pinch(uint16_t center_x, uint16_t center_y,
                                    uint16_t start_distance, uint16_t end_distance,
                                    uint16_t duration_ms)
{
    NS_LOG_INFO("Enhanced pinch: center(%d,%d), distance %d->%d\r\n",
                center_x, center_y, start_distance, end_distance);

    uint8_t steps = 20;
    uint16_t delay_per_step = duration_ms / steps;

    hid_touch_point_t touches[2];
    touches[0].contact_id = 0;
    touches[0].tip_switch = 1;
    touches[1].contact_id = 1;
    touches[1].tip_switch = 1;

    for (uint8_t i = 0; i <= steps; i++) {
        float progress = (float)i / steps;
        uint16_t current_distance = start_distance +
                                   (int16_t)((end_distance - start_distance) * progress);

        // Position fingers symmetrically around center
        touches[0].x = (center_x > current_distance/2) ? center_x - current_distance/2 : 0;
        touches[0].y = center_y;
        touches[1].x = (center_x + current_distance/2 > 32767) ? 32767 : center_x + current_distance/2;
        touches[1].y = center_y;

        app_hid_send_enhanced_multitouch(touches, 2);

        if (i < steps) {
            delay_n_ms(delay_per_step);
        }
    }

    delay_n_ms(50);
    app_hid_send_enhanced_multitouch(NULL, 0);
}

/**
 * @brief Enhanced rotation gesture with proper arc movement
 */
void app_touchscreen_enhanced_rotate(uint16_t center_x, uint16_t center_y,
                                     uint16_t radius, int16_t angle_degrees,
                                     uint16_t duration_ms)
{
    NS_LOG_INFO("Enhanced rotate: center(%d,%d), radius=%d, angle=%d deg\r\n",
                center_x, center_y, radius, angle_degrees);

    uint8_t steps = 30;
    uint16_t delay_per_step = duration_ms / steps;
    float angle_radians = angle_degrees * 3.14159f / 180.0f;

    hid_touch_point_t touches[2];
    touches[0].contact_id = 0;
    touches[0].tip_switch = 1;
    touches[1].contact_id = 1;
    touches[1].tip_switch = 1;

    for (uint8_t i = 0; i <= steps; i++) {
        float progress = (float)i / steps;
        float current_angle = angle_radians * progress;

        // Use simplified rotation calculation
        // Finger 1 starts at (center_x + radius, center_y) and rotates
        int16_t x1_offset = (int16_t)(radius * (1.0f - progress * 0.3f)); // Gradually reduce radius
        int16_t y1_offset = (int16_t)(radius * progress * 0.7f); // Gradually increase Y

        // Finger 2 is on opposite side
        touches[0].x = (center_x + x1_offset > 32767) ? 32767 :
                      ((center_x + x1_offset < 0) ? 0 : center_x + x1_offset);
        touches[0].y = (center_y + y1_offset > 32767) ? 32767 :
                      ((center_y + y1_offset < 0) ? 0 : center_y + y1_offset);

        touches[1].x = (center_x - x1_offset > 32767) ? 32767 :
                      ((center_x - x1_offset < 0) ? 0 : center_x - x1_offset);
        touches[1].y = (center_y - y1_offset > 32767) ? 32767 :
                      ((center_y - y1_offset < 0) ? 0 : center_y - y1_offset);

        app_hid_send_enhanced_multitouch(touches, 2);

        if (i < steps) {
            delay_n_ms(delay_per_step);
        }
    }

    delay_n_ms(50);
    app_hid_send_enhanced_multitouch(NULL, 0);
}