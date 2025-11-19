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
 * @file app_hid_keyboard.h
 * @author Nations Firmware Team
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#ifndef __APP_HID_KEYBOARD_H__
#define __APP_HID_KEYBOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// ====================== ���μ���Modifier Keys��======================
// ע�����μ�ͨ�� HID ����ĵ� 0 �ֽڱ�ʾ��ÿ��λ��Ӧһ�����μ�
#define HID_MOD_LCTRL    0x01    // �� Control
#define HID_MOD_LSHIFT   0x02    // �� Shift
#define HID_MOD_LALT     0x04    // �� Alt
#define HID_MOD_LGUI     0x08    // �� GUI��Windows ��/Command ����
#define HID_MOD_RCTRL    0x10    // �� Control
#define HID_MOD_RSHIFT   0x20    // �� Shift
#define HID_MOD_RALT     0x40    // �� Alt��Alt Gr��
#define HID_MOD_RGUI     0x80    // �� GUI��Windows ��/Command ����

// ====================== ���水����Key Codes 0x04-0x31��======================
// ��ĸ����A-Z��
#define HID_KEY_A        0x04
#define HID_KEY_B        0x05
#define HID_KEY_C        0x06
#define HID_KEY_D        0x07
#define HID_KEY_E        0x08
#define HID_KEY_F        0x09
#define HID_KEY_G        0x0A
#define HID_KEY_H        0x0B
#define HID_KEY_I        0x0C
#define HID_KEY_J        0x0D
#define HID_KEY_K        0x0E
#define HID_KEY_L        0x0F
#define HID_KEY_M        0x10
#define HID_KEY_N        0x11
#define HID_KEY_O        0x12
#define HID_KEY_P        0x13
#define HID_KEY_Q        0x14
#define HID_KEY_R        0x15
#define HID_KEY_S        0x16
#define HID_KEY_T        0x17
#define HID_KEY_U        0x18
#define HID_KEY_V        0x19
#define HID_KEY_W        0x1A
#define HID_KEY_X        0x1B
#define HID_KEY_Y        0x1C
#define HID_KEY_Z        0x1D

// ���ּ���0-9������������
#define HID_KEY_1        0x1E
#define HID_KEY_2        0x1F
#define HID_KEY_3        0x20
#define HID_KEY_4        0x21
#define HID_KEY_5        0x22
#define HID_KEY_6        0x23
#define HID_KEY_7        0x24
#define HID_KEY_8        0x25
#define HID_KEY_9        0x26
#define HID_KEY_0        0x27

// ���ż�������������
#define HID_KEY_ENTER    0x28        // �س���
#define HID_KEY_ESC      0x29        // �˳���
#define HID_KEY_BACKSPACE 0x2A       // �˸��
#define HID_KEY_TAB      0x2B        // Tab ��
#define HID_KEY_SPACE    0x2C        // �ո��
#define HID_KEY_MINUS    0x2D        // ���ţ�-��
#define HID_KEY_EQUAL    0x2E        // �Ⱥţ�=��
#define HID_KEY_LEFTBRACE 0x2F       // �����ţ�[��
#define HID_KEY_RIGHTBRACE 0x30      // �����ţ�]��
#define HID_KEY_BACKSLASH 0x31       // ��б�ܣ�\��
	
// ====================== ���ܼ���༭����0x32-0x47��======================
#define HID_KEY_SEMICOLON 0x33       // �ֺţ�;��
#define HID_KEY_APOSTROPHE 0x34      // �����ţ�'��
#define HID_KEY_GRAVE     0x35       // ��������`�����Ͻǣ�
#define HID_KEY_COMMA     0x36       // ���ţ�,��
#define HID_KEY_DOT       0x37       // ��ţ�.��
#define HID_KEY_SLASH     0x38       // б�ܣ�/��
#define HID_KEY_CAPS_LOCK 0x39       // ��Сд������
#define HID_KEY_F1        0x3A       // F1
#define HID_KEY_F2        0x3B       // F2
#define HID_KEY_F3        0x3C       // F3
#define HID_KEY_F4        0x3D       // F4
#define HID_KEY_F5        0x3E       // F5
#define HID_KEY_F6        0x3F       // F6
#define HID_KEY_F7        0x40       // F7
#define HID_KEY_F8        0x41       // F8
#define HID_KEY_F9        0x42       // F9
#define HID_KEY_F10       0x43       // F10
#define HID_KEY_F11       0x44       // F11
#define HID_KEY_F12       0x45       // F12
#define HID_KEY_PRINT_SCREEN 0x46    // ��ӡ��Ļ��
#define HID_KEY_SCROLL_LOCK 0x47    // ����������

// ====================== �༭������0x48-0x53��======================
#define HID_KEY_PAUSE     0x48       // ��ͣ��
#define HID_KEY_INSERT    0x49       // Insert ��
#define HID_KEY_HOME      0x4A       // Home ��
#define HID_KEY_PAGEUP    0x4B       // Page Up ��
#define HID_KEY_DELETE    0x4C       // Delete ��
#define HID_KEY_END       0x4D       // End ��
#define HID_KEY_PAGEDOWN  0x4E       // Page Down ��
#define HID_KEY_RIGHT     0x4F       // �Ҽ�ͷ��
#define HID_KEY_LEFT      0x50       // ���ͷ��
#define HID_KEY_DOWN      0x51       // �¼�ͷ��
#define HID_KEY_UP        0x52       // �ϼ�ͷ��
#define HID_KEY_NUM_LOCK  0x53       // С����������

// ====================== С��������0x54-0x65��======================
#define HID_KEY_KP_SLASH  0x54       // С���� б�ܣ�/��
#define HID_KEY_KP_ASTERISK 0x55     // С���� �Ǻţ�*��
#define HID_KEY_KP_MINUS  0x56       // С���� ���ţ�-��
#define HID_KEY_KP_PLUS   0x57       // С���� �Ӻţ�+��
#define HID_KEY_KP_ENTER  0x58       // С���� �س���
#define HID_KEY_KP_1      0x59       // С���� 1
#define HID_KEY_KP_2      0x5A       // С���� 2
#define HID_KEY_KP_3      0x5B       // С���� 3
#define HID_KEY_KP_4      0x5C       // С���� 4
#define HID_KEY_KP_5      0x5D       // С���� 5
#define HID_KEY_KP_6      0x5E       // С���� 6
#define HID_KEY_KP_7      0x5F       // С���� 7
#define HID_KEY_KP_8      0x60       // С���� 8
#define HID_KEY_KP_9      0x61       // С���� 9
#define HID_KEY_KP_0      0x62       // С���� 0
#define HID_KEY_KP_DOT    0x63       // С���� С���㣨.��
#define HID_KEY_KP_EQUAL  0x67       // С���� �Ⱥţ�=�����ּ���֧�֣�

// ====================== ���⹦�ܼ���0x68-0x87��======================
#define HID_KEY_F13       0x68       // F13
#define HID_KEY_F14       0x69       // F14
#define HID_KEY_F15       0x6A       // F15
#define HID_KEY_F16       0x6B       // F16
#define HID_KEY_F17       0x6C       // F17
#define HID_KEY_F18       0x6D       // F18
#define HID_KEY_F19       0x6E       // F19
#define HID_KEY_F20       0x6F       // F20
#define HID_KEY_F21       0x70       // F21
#define HID_KEY_F22       0x71       // F22
#define HID_KEY_F23       0x72       // F23
#define HID_KEY_F24       0x73       // F24

// ��ý����Ƽ������ּ���֧�֣�
#define HID_KEY_MUTE      0x7F       // ����
#define HID_KEY_VOLUME_UP 0x80       // ������
#define HID_KEY_VOLUME_DOWN 0x81    // ������
#define HID_KEY_MEDIA_NEXT 0x83      // ��һ��
#define HID_KEY_MEDIA_PREV 0x84      // ��һ��
#define HID_KEY_MEDIA_PLAY_PAUSE 0x85 // ����/��ͣ

#define HID_KEY_LEFT_CONTROL     0xE0
#define HID_KEY_LEFT_SHIFT       0xE1
#define HID_KEY_LEFT_ALT         0xE2
#define HID_KEY_LEFT_GUI         0xE3    // �� Windows ��
#define HID_KEY_RIGHT_CONTROL    0xE4
#define HID_KEY_RIGHT_SHIFT      0xE5
#define HID_KEY_RIGHT_ALT        0xE6
#define HID_KEY_RIGHT_GUI        0xE7    // �� Windows ��

// ====================== �ް������������ HID ���棩======================
#define HID_KEY_NONE      0x00       // �ް�������


// Keyboard report structure
typedef struct {
    uint8_t modifier;    // Modifier keys (Ctrl, Shift, Alt, GUI)
    uint8_t reserved;    // Reserved byte
    uint8_t keys[6];     // Key codes (up to 6 simultaneous keys)
} hid_keyboard_report_t;

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

// Function to build keyboard report
void build_keyboard_report(hid_keyboard_report_t* report, uint8_t modifier, uint8_t* keys, uint8_t key_count);

// Function to build touch screen report
void build_touchscreen_report(hid_touchscreen_report_t* report,
                              uint8_t contact_id,
                              uint8_t is_touching,
                              uint16_t x, uint16_t y,
                              uint8_t pressure);

// Function to simulate touch screen tap
void touchscreen_tap(uint16_t x, uint16_t y);

// Function to simulate touch screen swipe
void touchscreen_swipe(uint16_t x_start, uint16_t y_start,
                       uint16_t x_end, uint16_t y_end,
                       uint8_t duration_ms);

// Function to send touch screen report
void app_hid_send_touchscreen_report(hid_touchscreen_report_t* report);

#ifdef __cplusplus
}
#endif

#endif /* __APP_HID_KEYBOARD_H__ */
