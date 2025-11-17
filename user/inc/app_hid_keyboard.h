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

// HID Keyboard modifier bit masks
#define HID_KEYBOARD_LEFT_CTRL   0x01
#define HID_KEYBOARD_LEFT_SHIFT  0x02
#define HID_KEYBOARD_LEFT_ALT    0x04
#define HID_KEYBOARD_LEFT_GUI    0x08
#define HID_KEYBOARD_RIGHT_CTRL  0x10
#define HID_KEYBOARD_RIGHT_SHIFT 0x20
#define HID_KEYBOARD_RIGHT_ALT   0x40
#define HID_KEYBOARD_RIGHT_GUI   0x80

// HID Keyboard scan codes
#define HID_KEY_NONE             0x00
#define HID_KEY_A                0x04
#define HID_KEY_B                0x05
#define HID_KEY_C                0x06
#define HID_KEY_D                0x07
#define HID_KEY_E                0x08
#define HID_KEY_F                0x09
#define HID_KEY_G                0x0A
#define HID_KEY_H                0x0B
#define HID_KEY_I                0x0C
#define HID_KEY_J                0x0D
#define HID_KEY_K                0x0E
#define HID_KEY_L                0x0F
#define HID_KEY_M                0x10
#define HID_KEY_N                0x11
#define HID_KEY_O                0x12
#define HID_KEY_P                0x13
#define HID_KEY_Q                0x14
#define HID_KEY_R                0x15
#define HID_KEY_S                0x16
#define HID_KEY_T                0x17
#define HID_KEY_U                0x18
#define HID_KEY_V                0x19
#define HID_KEY_W                0x1A
#define HID_KEY_X                0x1B
#define HID_KEY_Y                0x1C
#define HID_KEY_Z                0x1D

#define HID_KEY_1                0x1E
#define HID_KEY_2                0x1F
#define HID_KEY_3                0x20
#define HID_KEY_4                0x21
#define HID_KEY_5                0x22
#define HID_KEY_6                0x23
#define HID_KEY_7                0x24
#define HID_KEY_8                0x25
#define HID_KEY_9                0x26
#define HID_KEY_0                0x27

#define HID_KEY_ENTER            0x28
#define HID_KEY_ESC              0x29
#define HID_KEY_BACKSPACE        0x2A
#define HID_KEY_TAB              0x2B
#define HID_KEY_SPACE            0x2C
#define HID_KEY_MINUS            0x2D
#define HID_KEY_EQUAL            0x2E
#define HID_KEY_LEFT_BRACKET     0x2F
#define HID_KEY_RIGHT_BRACKET    0x30
#define HID_KEY_BACKSLASH        0x31
#define HID_KEY_SEMICOLON        0x33
#define HID_KEY_APOSTROPHE       0x34
#define HID_KEY_GRAVE            0x35
#define HID_KEY_COMMA            0x36
#define HID_KEY_PERIOD           0x37
#define HID_KEY_SLASH            0x38
#define HID_KEY_CAPS_LOCK        0x39

#define HID_KEY_F1               0x3A
#define HID_KEY_F2               0x3B
#define HID_KEY_F3               0x3C
#define HID_KEY_F4               0x3D
#define HID_KEY_F5               0x3E
#define HID_KEY_F6               0x3F
#define HID_KEY_F7               0x40
#define HID_KEY_F8               0x41
#define HID_KEY_F9               0x42
#define HID_KEY_F10              0x43
#define HID_KEY_F11              0x44
#define HID_KEY_F12              0x45

#define HID_KEY_PRINT_SCREEN     0x46
#define HID_KEY_SCROLL_LOCK      0x47
#define HID_KEY_PAUSE            0x48
#define HID_KEY_INSERT           0x49
#define HID_KEY_HOME             0x4A
#define HID_KEY_PAGE_UP          0x4B
#define HID_KEY_DELETE           0x4C
#define HID_KEY_END              0x4D
#define HID_KEY_PAGE_DOWN        0x4E
#define HID_KEY_RIGHT            0x4F
#define HID_KEY_LEFT             0x50
#define HID_KEY_DOWN             0x51
#define HID_KEY_UP               0x52

// Keyboard report structure
typedef struct {
    uint8_t modifier;    // Modifier keys (Ctrl, Shift, Alt, GUI)
    uint8_t reserved;    // Reserved byte
    uint8_t keys[6];     // Key codes (up to 6 simultaneous keys)
} hid_keyboard_report_t;

// Function to build keyboard report
void build_keyboard_report(hid_keyboard_report_t* report, uint8_t modifier, uint8_t* keys, uint8_t key_count);

#ifdef __cplusplus
}
#endif

#endif /* __APP_HID_KEYBOARD_H__ */
