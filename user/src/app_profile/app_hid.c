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
 * @file app_hid.c
 * @author Nations Firmware Team
 * @version v1.0.2
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */


/**
 * @addtogroup APP
 * @{
 */

#include "rwip_config.h"            // SW configuration

#include <stdio.h>
#include <string.h>

#if (BLE_APP_HID)

/* Includes ------------------------------------------------------------------*/
#include "ns_ble.h"                    // Application Definitions
#include "ns_sec.h"                // Application Security Module API
#include "ns_ble_task.h"               // Application task definitions
#include "app_hid.h"                // HID Application Module Definitions
#include "hogp\hogpd\api\hogpd_task.h"             // HID Over GATT Profile Device Role Functions
#include "prf_types.h"              // Profile common types Definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include "ke_timer.h"

#if (NVDS_SUPPORT)
#include "nvds.h"                   // NVDS Definitions
#endif //(NVDS_SUPPORT)
#include "hogp\hogpd\src\hogpd.h"
#include "co_utils.h"               // Common functions

#if (KE_PROFILING)
#include "ke_mem.h"
#endif //(KE_PROFILING)
#include "app_gpio.h"
#include "app_ble.h" 
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/// HID Application Module Environment Structure
struct app_hid_env_tag app_hid_env;

/// HID Mouse Report Descriptor
static const uint8_t app_hid_mouse_report_map[] =
{
    /**
     *  --------------------------------------------------------------------------
     *  Bit      |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
     *  --------------------------------------------------------------------------
     *  Byte 0   |               Not Used                | Middle| Right | Left  |
     *  --------------------------------------------------------------------------
     *  Byte 1   |                     X Axis Relative Movement                  |
     *  --------------------------------------------------------------------------
     *  Byte 2   |                     Y Axis Relative Movement                  |
     *  --------------------------------------------------------------------------
     *  Byte 3   |                     Wheel Relative Movement                   |
     *  --------------------------------------------------------------------------
     */

    0x05, 0x01,     /// USAGE PAGE (Generic Desktop)
    0x09, 0x02,     /// USAGE (Mouse)
    
    0xA1, 0x01,     /// COLLECTION (Application)
    
    0x85, 0x01,     /// REPORT ID (1) - MANDATORY
    0x09, 0x01,     ///     USAGE (Pointer)
    0xA1, 0x00,     ///     COLLECTION (Physical)

    /**
     * ----------------------------------------------------------------------------
     * BUTTONS
     * ----------------------------------------------------------------------------
     */
    0x05, 0x09,     ///         USAGE PAGE (Buttons)
    0x19, 0x01,     ///         USAGE MINIMUM (1)
    0x29, 0x08,     ///         USAGE MAXIMUM (8)
    0x15, 0x00,     ///         LOGICAL MINIMUM (0)
    0x25, 0x01,     ///         LOGICAL MAXIMUM (1)
    0x75, 0x01,     ///         REPORT SIZE (1)
    0x95, 0x08,     ///         REPORT COUNT (8)
    0x81, 0x02,     ///         INPUT (Data, Variable, Absolute)

    /**
     * ----------------------------------------------------------------------------
     * MOVEMENT DATA
     * ----------------------------------------------------------------------------
     */
    0x05, 0x01,     ///         USAGE PAGE (Generic Desktop)
    0x16, 0x08, 0xFF, ///       LOGICAL MINIMUM (-255)
    0x26, 0xFF, 0x00, ///       LOGICAL MAXIMUM (255)
    0x75, 0x10,     ///         REPORT SIZE (16)
    0x95, 0x02,     ///         REPORT COUNT (2)
    0x09, 0x30,     ///         USAGE (X)
    0x09, 0x31,     ///         USAGE (Y)
    0x81, 0x06,     ///         INPUT (Data, Variable, Relative)

    0x15, 0x81,     ///         LOGICAL MINIMUM (-127)
    0x25, 0x7F,     ///         LOGICAL MAXIMUM (127)
    0x75, 0x08,     ///         REPORT SIZE (8)
    0x95, 0x01,     ///         REPORT COUNT (1)
    0x09, 0x38,     ///         USAGE (Wheel)
    0x81, 0x06,     ///         INPUT (Data, Variable, Relative)

    0xC0,           ///     END COLLECTION (Physical)
    0xC0,            /// END COLLECTION (Application)

    // Report ID 2: Advanced buttons
		0x05, 0x0C,        // Usage Page (Consumer Devices)
		0x09, 0x01,        // Usage (Consumer Control)
		0xA1, 0x01,        // Collection (Application)
		0x85, 0x02,        //   Report ID (2)

		// 定义32个1位开关字段，每个对应一个具体功能
		0x15, 0x00,        //   Logical Minimum (0)
		0x25, 0x01,        //   Logical Maximum (1)
		0x75, 0x01,        //   Report Size (1)
		0x95, 0x20,        //   Report Count (32) - 总共32个位

		// Consumer Page的功能 (0x0C)
		0x09, 0xCD,        //   Usage (Play/Pause)
		0x09, 0xB5,        //   Usage (Scan Next Track)
		0x09, 0xB6,        //   Usage (Scan Previous Track)
		0x09, 0xB7,        //   Usage (Stop)
		0x09, 0xB8,        //   Usage (Eject)
		0x09, 0xB9,        //   Usage (Record)
		0x09, 0xEA,        //   Usage (Fast Forward)
		0x09, 0xBB,        //   Usage (Rewind)

		0x09, 0xE9,        //   Usage (Volume Increment)
		0x09, 0xEA,        //   Usage (Volume Decrement)
		0x09, 0xE2,        //   Usage (Mute)
		0x0A, 0x2B, 0x02,  //   Usage (Equalizer)
		0x0A, 0x31, 0x02,  //   Usage (Bass Boost)
		0x0A, 0x32, 0x02,  //   Usage (Loudness)
		0x0A, 0x33, 0x02,  //   Usage (Treble Increment)
		0x0A, 0x34, 0x02,  //   Usage (Bass Increment)

		// 切换到Generic Desktop Page (0x01)
		0x09, 0x6F,        //   Usage (Brightness Decrement)
		0x09, 0x70,        //   Usage (Brightness Increment)
		0x0A, 0x92, 0x01,  //   Usage (Calculator)
		0x0A, 0x50, 0x02,  //   Usage (Search)
		0x09, 0x82,        // Usage (System Sleep) - 替代 0x0406
		0x09, 0x83,        // Usage (System Wake) - 替代 0x0407  
		0x09, 0x81,        // Usage (System Power Down) - 替代 0x0408
		0x09, 0x80,        // Usage (System Power On) - 替代 0x0409
		
		//0x0A, 0x0A, 0x04,  //   Usage (Power Toggle)
		0x0A, 0x8F, 0x05,  //   Usage (Display Invert)

		0x0A, 0x83, 0x01,  //   Usage (Voice Assistant)
		0x0A, 0x92, 0x01,  //   Usage (Media Select)
		0x0A, 0x95, 0x01,  //   Usage (Browser Home)
		0x0A, 0x92, 0x01,  //   Usage (Calculator) - 现在在正确的Page下了
		0x0A, 0x16, 0x02,  //   Usage (Email Reader)
		0x0A, 0x2A, 0x02,  //   Usage (Music Player)
		0x0A, 0x2D, 0x02,  //   Usage (Video Player)

		0x81, 0x06,        //   Input (Data,Var,Abs) - 32位位图字段
		0xC0,              // End Collection
		
    // Report ID 3: Keyboard
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        // Report ID (3)

    // Modifier keys (8 bits for Shift, Ctrl, Alt, GUI)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (Left Control)
    0x29, 0xE7,        //   Usage Maximum (Right GUI)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    // Reserved byte
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const)

    // Key codes (6 keys)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0xFF,        //   Logical Maximum (255)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0xFF,        //   Usage Maximum (255)
    0x81, 0x00,        //   Input (Data,Array)

    0xC0,              // End Collection

    // Report ID 4: Multi-Touch Screen (Windows 8+ compatible, 3 touches)
    0x05, 0x0D,        // Usage Page (Digitizers)
    0x09, 0x04,        // Usage (Touch Screen)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x04,        // Report ID (4)
		
		
		
    // Touch point 1
		0x05, 0x0D,        // Usage Page (Digitizers)
    0x09, 0x22,        //   Usage (Finger)
    0xA1, 0x02,        //   Collection (Logical)
    0x09, 0x42,        //     Usage (Tip Switch)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x09, 0x51,        //     Usage (Contact Identifier)
    0x75, 0x07,        //     Report Size (7)
    0x95, 0x01,        //     Report Count (1)
		0x25, 0x7F,        //     Logical Maximum (127)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0xC0,              //   End Collection (Logical)

    // Touch point 2
		0x05, 0x0D,        // Usage Page (Digitizers)
    0x09, 0x22,        //   Usage (Finger)
    0xA1, 0x02,        //   Collection (Logical)
    0x09, 0x42,        //     Usage (Tip Switch)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x09, 0x51,        //     Usage (Contact Identifier)
    0x75, 0x07,        //     Report Size (7)
    0x95, 0x01,        //     Report Count (1)
		0x25, 0x7F,        //     Logical Maximum (127)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0xC0,              //   End Collection (Logical)

    // Touch point 3
		0x05, 0x0D,        // Usage Page (Digitizers)
    0x09, 0x22,        //   Usage (Finger)
    0xA1, 0x02,        //   Collection (Logical)
    0x09, 0x42,        //     Usage (Tip Switch)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x09, 0x51,        //     Usage (Contact Identifier)
    
    0x75, 0x07,        //     Report Size (7)
    0x95, 0x01,        //     Report Count (1)
		0x25, 0x7F,        //     Logical Maximum (127)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)

    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0xC0,              //   End Collection (Logical)

    

    0xC0,              // End Collection
};



/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


void app_hid_init(void)
{
    NS_LOG_DEBUG("%s\r\n",__func__);
    
    // Reset the environment
    memset(&app_hid_env, 0, sizeof(app_hid_env));

    app_hid_env.nb_report = APP_HID_NB_SEND_REPORT;


    app_hid_env.timeout = APP_HID_SILENCE_DURATION_1;
    //register application subtask to app task
    struct prf_task_t prf;
    prf.prf_task_id = TASK_ID_HOGPD;
    prf.prf_task_handler = &app_hid_handlers;
    ns_ble_prf_task_register(&prf);
    
    //register get itf function to prf.c
    struct prf_get_func_t get_func;
    get_func.task_id = TASK_ID_HOGPD;
    get_func.prf_itf_get_func = hogpd_prf_itf_get;
    prf_get_itf_func_register(&get_func);
    
}


void app_hid_add_hids(void)
{
    struct hogpd_db_cfg *db_cfg;
    // Prepare the HOGPD_CREATE_DB_REQ message
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                   TASK_GAPM, TASK_APP,
                                                   gapm_profile_task_add_cmd, sizeof(struct hogpd_db_cfg));

    NS_LOG_DEBUG("%s\r\n",__func__);
    // Fill message
    req->operation   = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl     = PERM(SVC_AUTH, UNAUTH); // NO_AUTH UNAUTH AUTH
    req->prf_task_id = TASK_ID_HOGPD;
    req->app_task    = TASK_APP;
    req->start_hdl   = 0;

    // Set parameters
    db_cfg = (struct hogpd_db_cfg* ) req->param;

    // Only one HIDS instance is useful
    db_cfg->hids_nb = 1;

    // The device is a keyboard and mouse combo with touch screen
    db_cfg->cfg[0].svc_features = HOGPD_CFG_KEYBOARD | HOGPD_CFG_MOUSE; // Support both keyboard and mouse

    // Report Characteristics - Now includes touch screen with both Input and Feature reports
    db_cfg->cfg[0].report_nb    = 5;  // 5 reports: mouse, multimedia, keyboard, touchscreen input, touchscreen feature

    db_cfg->cfg[0].report_id[0] = 1;  // Mouse
    db_cfg->cfg[0].report_char_cfg[0] = HOGPD_CFG_REPORT_IN;

    db_cfg->cfg[0].report_id[1] = 2;  // Multimedia keys
    db_cfg->cfg[0].report_char_cfg[1] = HOGPD_CFG_REPORT_IN;

    db_cfg->cfg[0].report_id[2] = 3;  // Keyboard
    db_cfg->cfg[0].report_char_cfg[2] = HOGPD_CFG_REPORT_IN;

    db_cfg->cfg[0].report_id[3] = 4;  // Touch Screen (Input Report)
    db_cfg->cfg[0].report_char_cfg[3] = HOGPD_CFG_REPORT_IN;

    db_cfg->cfg[0].report_id[4] = 4;  // Touch Screen (Feature Report - same Report ID!)
    db_cfg->cfg[0].report_char_cfg[4] = HOGPD_CFG_REPORT_FEAT;

    // HID Information
    db_cfg->cfg[0].hid_info.bcdHID       = 0x0111;         // HID Version 1.11
    db_cfg->cfg[0].hid_info.bCountryCode = 0x00;
    db_cfg->cfg[0].hid_info.flags        = HIDS_REMOTE_WAKE_CAPABLE | HIDS_NORM_CONNECTABLE;

    // Send the message
    ke_msg_send(req);
    
    app_hid_init();
}



/*
 * @brief Function called when get connection complete event from the GAP
 *
 */
void app_hid_enable_prf(uint8_t conidx)
{
    uint16_t ntf_cfg;
    NS_LOG_DEBUG("%s,idx %x\r\n",__func__,conidx);

    // Store the connection handle
    app_hid_env.conidx = conidx;

    // Allocate the message
    struct hogpd_enable_req * req = KE_MSG_ALLOC(HOGPD_ENABLE_REQ,
                                                 prf_get_task_from_id(TASK_ID_HOGPD),
                                                 TASK_APP,
                                                 hogpd_enable_req);

    // Fill in the parameter structure
    req->conidx     = conidx;
    // Notifications are disabled
    ntf_cfg         = 0;

    // Go to Enabled state
    app_hid_env.state = APP_HID_ENABLED;

    // If first connection with the peer device
    if (ns_sec_get_bond_status())
    {
        app_hid_env.state = APP_HID_READY;
        app_hid_env.nb_report = APP_HID_NB_SEND_REPORT;
        // Enable notifications for all reports
        // This value enables notifications for all configured reports
        ntf_cfg = 0xFFFF;  // Enable all possible notifications
        NS_LOG_DEBUG("HID ready for bonded device, enabling all notifications\r\n");

    }

    req->ntf_cfg[conidx] = ntf_cfg;

    // Send the message
    ke_msg_send(req);
}







/*
 * @brief Function called from PS2 driver
 *
 */
void app_hid_send_mouse_report(struct ps2_mouse_msg report)
{

    NS_LOG_DEBUG("HID report, state:%d, x:%d, y:%d \r\n",app_hid_env.state, report.x, report.y);
    switch (app_hid_env.state)
    {
        case (APP_HID_READY):
        {
            // Check if the report can be sent
            if (app_hid_env.nb_report)
            {
                // Buffer used to create the Report
                uint8_t report_buff[APP_HID_MOUSE_REPORT_LEN];
                // X, Y and wheel relative movements
                int16_t x;
                int16_t y;

                // Clean the report buffer
                memset(&report_buff[0], 0, APP_HID_MOUSE_REPORT_LEN);

                // Set the button states
                report_buff[0] = (report.b & 0x07);

                // If X value is negative
                if (report.b & 0x10)
                {
                    report.x = ~report.x;
                    report.x += 1;
                    x = (int16_t)report.x;
                    x *= (-1);
                }
                else
                {
                    x = (int16_t)report.x;
                }

                // If Y value is negative
                if (report.b & 0x20)
                {
                    report.y = ~report.y;
                    report.y += 1;
                    y = (int16_t)report.y;
                }
                else
                {
                    y = (int16_t)report.y;
                    y *= (-1);
                }


                // Set the X and Y movement value in the report
                co_write16p(&report_buff[1], x);
                co_write16p(&report_buff[3], y);
                report_buff[5] =(signed char) (-1) * report.w;

                // Allocate the HOGPD_REPORT_UPD_REQ message
                struct hogpd_report_upd_req * req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                                  prf_get_task_from_id(TASK_ID_HOGPD),
                                                                  TASK_APP,
                                                                  hogpd_report_upd_req,
                                                                  APP_HID_MOUSE_REPORT_LEN);

                req->conidx  = app_hid_env.conidx;
                //now fill report
                req->report.hid_idx  = app_hid_env.conidx;
                req->report.type     = HOGPD_REPORT; //HOGPD_BOOT_MOUSE_INPUT_REPORT;//
                req->report.idx      = 0; //0 for boot reports and report map
                req->report.length   = APP_HID_MOUSE_REPORT_LEN;
                memcpy(&req->report.value[0], &report_buff[0], APP_HID_MOUSE_REPORT_LEN);

                ke_msg_send(req);

                app_hid_env.nb_report--;

                // Restart the mouse timeout timer if needed
                if (app_hid_env.timeout != 0)
                {
                    ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                    app_hid_env.timer_enabled = true;
                }
            }
        } break;

        case (APP_HID_WAIT_REP):
        {
            // Requested connection parameters
            struct gapc_conn_param conn_param;

            /*
             * Requested connection interval: 10ms
             * Latency: 25
             * Supervision Timeout: 2s
             */
            conn_param.intv_min = 8;
            conn_param.intv_max = 8;
            conn_param.latency  = 25;
            conn_param.time_out = 200;

            ns_ble_update_param(&conn_param);
            // Restart the mouse timeout timer if needed
            if (app_hid_env.timeout != 0)
            {
                ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                app_hid_env.timer_enabled = true;
            }

            // Go back to the ready state
            app_hid_env.state = APP_HID_READY;
        } break;

        case (APP_HID_IDLE):
        {
            // Try to restart advertising if needed
//            app_update_adv_state(true);
        } break;
        
                
        // DISABLE and ENABLED states
        default:
        {
            // Drop the message
        } break;
    }

}


/*
 * @brief Function  
 *
 */
void app_hid_send_consumer_report(uint8_t* report)
{
    NS_LOG_DEBUG("Consumer,state:%d ,%d\r\n",app_hid_env.state,app_hid_env.nb_report);
    switch (app_hid_env.state)
    {
        case (APP_HID_READY):
        {
            // Check if the report can be sent
            if (app_hid_env.nb_report)
            {
                // Allocate the HOGPD_REPORT_UPD_REQ message
                struct hogpd_report_upd_req * req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                                  prf_get_task_from_id(TASK_ID_HOGPD),
                                                                  TASK_APP,
                                                                  hogpd_report_upd_req,
                                                                  APP_HID_CONSUMER_REPORT_LEN);

                req->conidx  = app_hid_env.conidx;
                //now fill report
                req->report.hid_idx  = app_hid_env.conidx;
                req->report.type     = HOGPD_REPORT; //HOGPD_BOOT_MOUSE_INPUT_REPORT;//
                req->report.idx      = 1; //repoort id 2 report map
                req->report.length   = APP_HID_CONSUMER_REPORT_LEN;
                memcpy(&req->report.value[0], &report[0],APP_HID_CONSUMER_REPORT_LEN);
								NS_LOG_WARNING("\r\n%d ,%d\r\n",req->report.value[0],req->report.value[1]);
                ke_msg_send(req);

                app_hid_env.nb_report--;

                // Restart the mouse timeout timer if needed
                if (app_hid_env.timeout != 0)
                {
                    ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                    app_hid_env.timer_enabled = true;
                }
            }
        } break;

        case (APP_HID_WAIT_REP):
        {
            // Requested connection parameters
            struct gapc_conn_param conn_param;

            /*
             * Requested connection interval: 10ms
             * Latency: 25
             * Supervision Timeout: 2s
             */
            conn_param.intv_min = 8;
            conn_param.intv_max = 8;
            conn_param.latency  = 25;
            conn_param.time_out = 200;
            ns_ble_update_param(&conn_param);

            // Restart the mouse timeout timer if needed
            if (app_hid_env.timeout != 0)
            {
                ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                app_hid_env.timer_enabled = true;
            }

            // Go back to the ready state
            app_hid_env.state = APP_HID_READY;
        } break;

        case (APP_HID_IDLE):
        {
            // Try to restart advertising if needed
//            app_update_adv_state(true);
        } break;
        
                
        // DISABLE and ENABLED states
        default:
        {
            // Drop the message
        } break;
    }
#if (KE_PROFILING)
//    app_display_hdl_env_size(0xFFFF, ke_get_mem_usage(KE_MEM_ENV));
//    app_display_hdl_db_size(0xFFFF, ke_get_mem_usage(KE_MEM_ATT_DB));
//    app_display_hdl_msg_size((uint16_t)ke_get_max_mem_usage(), ke_get_mem_usage(KE_MEM_KE_MSG));
    #endif //(KE_PROFILING)
}

/*
 * @brief Function to send keyboard report
 *
 */
void app_hid_send_keyboard_report(uint8_t* report)
{
    NS_LOG_DEBUG("Keyboard report state:%d, nb_report:%d\r\n", app_hid_env.state, app_hid_env.nb_report);

    // Debug: Print report content
    NS_LOG_DEBUG("Keyboard report data: ");
    for(int i = 0; i < APP_HID_KEYBOARD_REPORT_LEN; i++) {
        NS_LOG_DEBUG("%02x ", report[i]);
    }
    NS_LOG_DEBUG("\r\n");

    switch (app_hid_env.state)
    {
        case (APP_HID_READY):
        {
            // Check if the report can be sent
            if (app_hid_env.nb_report)
            {
                // Allocate the HOGPD_REPORT_UPD_REQ message
                struct hogpd_report_upd_req * req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                                  prf_get_task_from_id(TASK_ID_HOGPD),
                                                                  TASK_APP,
                                                                  hogpd_report_upd_req,
                                                                  APP_HID_KEYBOARD_REPORT_LEN);

                req->conidx  = app_hid_env.conidx;
                //now fill report
                req->report.hid_idx  = app_hid_env.conidx;
                req->report.type     = HOGPD_REPORT;
                req->report.idx      = 2; // Report index 2 for Report ID 3 (keyboard)
                req->report.length   = APP_HID_KEYBOARD_REPORT_LEN;
                memcpy(&req->report.value[0], report, APP_HID_KEYBOARD_REPORT_LEN);

                NS_LOG_DEBUG("Sending keyboard report: idx=%d, len=%d\r\n", req->report.idx, req->report.length);

                ke_msg_send(req);
								
                app_hid_env.nb_report--;

                // Restart the mouse timeout timer if needed
                if (app_hid_env.timeout != 0)
                {
                    ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                    app_hid_env.timer_enabled = true;
                }
            }
            else
            {
                NS_LOG_WARNING("No report available to send\r\n");
            }
        } break;

        case (APP_HID_WAIT_REP):
        {
            // Requested connection parameters
            struct gapc_conn_param conn_param;

            /*
             * Requested connection interval: 10ms
             * Latency: 25
             * Supervision Timeout: 2s
             */
            conn_param.intv_min = 8;
            conn_param.intv_max = 8;
            conn_param.latency  = 25;
            conn_param.time_out = 200;
            ns_ble_update_param(&conn_param);

            // Restart the mouse timeout timer if needed
            if (app_hid_env.timeout != 0)
            {
                ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                app_hid_env.timer_enabled = true;
            }

            // Go back to the ready state
            app_hid_env.state = APP_HID_READY;
        } break;

        case (APP_HID_IDLE):
        {
            NS_LOG_WARNING("HID in IDLE state\r\n");
            // Try to restart advertising if needed
//            app_update_adv_state(true);
        } break;


        // DISABLE and ENABLED states
        default:
        {
            NS_LOG_WARNING("HID in unexpected state: %d\r\n", app_hid_env.state);
            // Drop the message
        } break;
    }
}

bool is_app_hid_ready(void)
{
    if (app_hid_env.state == APP_HID_READY)
    {
        return true;
    }

    return false;
}


/*
 * MESSAGE HANDLERS
 */


static int hogpd_ctnl_pt_ind_handler(ke_msg_id_t const msgid,
                                     struct hogpd_ctnl_pt_ind const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
   NS_LOG_DEBUG("%s\r\n",__func__);

    if (param->conidx == app_hid_env.conidx)
    {
        //make use of param->hid_ctnl_pt
        struct hogpd_report_cfm *req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
                                                        prf_get_task_from_id(TASK_ID_HOGPD),/* src_id */
                                                        TASK_APP,
                                                        hogpd_report_cfm,
                                                        0);

        req->conidx = param->conidx; ///app_hid_env.conidx; ///???
        /// Operation requested (read/write @see enum hogpd_op)
        req->operation = HOGPD_OP_REPORT_WRITE;
        /// Status of the request
        req->status = GAP_ERR_NO_ERROR;  ///???
        /// Report Info
        //req->report;
        /// HIDS Instance
        req->report.hid_idx = app_hid_env.conidx; ///???
        /// type of report (@see enum hogpd_report_type)
        req->report.type = (uint8_t)-1;//outside 
        /// Report Length (uint8_t)
        req->report.length = 0;
        /// Report Instance - 0 for boot reports and report map
        req->report.idx = 0;
        /// Report data
        

        // Send the message
        ke_msg_send(req);
    }
    return (KE_MSG_CONSUMED);
}




static int hogpd_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
                                     struct hogpd_ntf_cfg_ind const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    NS_LOG_DEBUG("%s,v_idx;%x,p_idx;%x\r\n",__func__,app_hid_env.conidx,param->conidx);
    if (app_hid_env.conidx == param->conidx)
    {
        if ((param->ntf_cfg[param->conidx] & HOGPD_CFG_REPORT_NTF_EN ) != 0)
        {
            // The device is ready to send reports to the peer device
            app_hid_env.state = APP_HID_READY;
            NS_LOG_INFO("HID Ready\r\n");
        }
        else
        {
            // Come back to the Enabled state
            if (app_hid_env.state == APP_HID_READY)
            {
                app_hid_env.state = APP_HID_ENABLED;
            }
            NS_LOG_DEBUG("HID enable\r\n");
        }
        NS_LOG_DEBUG("ntf_cfg:0x%x\r\n",param->ntf_cfg[param->conidx]);    
        #if (NVDS_SUPPORT)
        // Store the notification configuration in the database
        if (nvds_put(NVDS_TAG_MOUSE_NTF_CFG, NVDS_LEN_MOUSE_NTF_CFG,
                     (uint8_t *)&param->ntf_cfg[param->conidx]) != NVDS_OK)
        {
            // Should not happen
            ASSERT_ERR(0);
        }
        #endif //(NVDS_SUPPORT)
    }

    return (KE_MSG_CONSUMED);
}

static int hogpd_report_req_ind_handler(ke_msg_id_t const msgid,
                                    struct hogpd_report_req_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    NS_LOG_WARNING("=== REPORT REQUEST ===\r\n");
    NS_LOG_WARNING("operation: %d\r\n", param->operation);
    NS_LOG_WARNING("report.idx: %d\r\n", param->report.idx);
    NS_LOG_WARNING("report.type: %d\r\n", param->report.type);
    NS_LOG_WARNING("report.hid_idx: %d\r\n", param->report.hid_idx);
    NS_LOG_WARNING("conidx: %d\r\n", param->conidx);
    NS_LOG_WARNING("=====================\r\n");

    // 第一步：根据report.idx获取Report类型（Input/Feature）
    uint8_t report_cfg = 0;
    switch(param->report.idx)
    {
        case 0: // Report ID=1（鼠标）
        case 1: // Report ID=2（多媒体）
        case 2: // Report ID=3（键盘）
        case 3: // Report ID=4（触摸屏Input）
            report_cfg = HOGPD_CFG_REPORT_IN;
            break;
        case 4: // Report ID=4（触摸屏Feature）
            report_cfg = HOGPD_CFG_REPORT_FEAT;
            break;
        default:
            report_cfg = 0;
            break;
    }

    if ((param->operation == HOGPD_OP_REPORT_READ) && (param->report.type == HOGPD_REPORT_MAP))
    {
        // 原有逻辑：返回Report Map，无需修改
        struct hogpd_report_cfm *req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
                                                        src_id, 
                                                        dest_id, 
                                                        hogpd_report_cfm,
                                                        APP_HID_MOUSE_REPORT_MAP_LEN);
        req->conidx = param->conidx;
        req->operation = HOGPD_OP_REPORT_READ;
        req->status = GAP_ERR_NO_ERROR;
        req->report.hid_idx = param->report.hid_idx;
        req->report.type = HOGPD_REPORT_MAP;
        req->report.length = APP_HID_MOUSE_REPORT_MAP_LEN;
        memcpy(&req->report.value[0], &app_hid_mouse_report_map[0], APP_HID_MOUSE_REPORT_MAP_LEN);
        ke_msg_send(req);
    }
    else if ((param->operation == HOGPD_OP_REPORT_READ) && (param->report.type == HOGPD_REPORT))
    {
        struct hogpd_report_cfm *req = NULL;

        NS_LOG_WARNING("Read HOGPD_REPORT: idx=%d, report_cfg=%d\r\n", param->report.idx, report_cfg);

        // 特殊处理触摸屏Feature Report（idx=4, Report ID=4, Type=Feature）的读取请求
        // Windows会读取Feature Report来获取最大触摸点数
        if (param->report.idx == 4)
        {
            NS_LOG_WARNING("Touch screen FEATURE report read - returning Contact Count Maximum = 3\r\n");
            // 返回最大触摸点数（1字节，0x03）
            req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
                                   src_id,
                                   dest_id,
                                   hogpd_report_cfm,
                                   1); // Feature Report长度为1字节
            req->conidx = param->conidx;
            req->operation = HOGPD_OP_REPORT_READ;
            req->status = GAP_ERR_NO_ERROR;
            req->report.hid_idx = param->report.hid_idx;
            req->report.type = HOGPD_REPORT;
            req->report.idx = param->report.idx;
            req->report.length = 1;
            req->report.value[0] = 0x03; // 返回最大触摸点数3

            NS_LOG_WARNING("Sent Feature Report: value=0x%02x\r\n", req->report.value[0]);
        }
        else
        {
            NS_LOG_WARNING("Other report read (idx=%d) - returning zeros\r\n", param->report.idx);
            // 处理普通Input Report读取请求（鼠标/键盘/多媒体/触摸屏Input），返回默认值0即可
            req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
                                   src_id,
                                   dest_id,
                                   hogpd_report_cfm,
                                   8);
            req->conidx = param->conidx;
            req->operation = HOGPD_OP_REPORT_READ;
            req->status = GAP_ERR_NO_ERROR;
            req->report.hid_idx = param->report.hid_idx;
            req->report.type = HOGPD_REPORT;
            req->report.idx = param->report.idx;
            req->report.length = 8;
            memset(&req->report.value[0], 0, 8); // Input Report读取返回0（无实际意义）
        }
        ke_msg_send(req);
    }
    else if (param->report.type == HOGPD_BOOT_MOUSE_INPUT_REPORT)
    {
        // 原有逻辑：处理Boot鼠标Report，无需修改
        struct hogpd_report_cfm *req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
                                                        src_id,
                                                        dest_id,
                                                        hogpd_report_cfm,
                                                        0);
        req->conidx = param->conidx;
        req->operation = HOGPD_OP_REPORT_READ;
        req->status = GAP_ERR_NO_ERROR;
        req->report.hid_idx = param->report.hid_idx;
        req->report.type = param->report.type;
        req->report.length = 0;
        req->report.idx = param->report.idx;
        ke_msg_send(req);
    }
    else
    {
        // 其他情况返回无错误
        struct hogpd_report_cfm *req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
                                                        src_id,
                                                        dest_id,
                                                        hogpd_report_cfm,
                                                        0);
        req->conidx = param->conidx;
        req->operation = param->operation;
        req->status = GAP_ERR_NO_ERROR;
        req->report.hid_idx = param->report.hid_idx;
        req->report.type = param->report.type;
        req->report.length = 0;
        req->report.idx = param->report.idx;
        ke_msg_send(req);
    }

    return (KE_MSG_CONSUMED);
}

static int hogpd_proto_mode_req_ind_handler(ke_msg_id_t const msgid,
                                        struct hogpd_proto_mode_req_ind const *param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    NS_LOG_DEBUG("%s\r\n",__func__);
    if ((param->conidx == app_hid_env.conidx) && (param->operation == HOGPD_OP_PROT_UPDATE))
    {

        //make use of param->proto_mode
        struct hogpd_proto_mode_cfm *req = KE_MSG_ALLOC_DYN(HOGPD_PROTO_MODE_CFM,
                                                        prf_get_task_from_id(TASK_ID_HOGPD),/* src_id */
                                                        TASK_APP,
                                                        hogpd_proto_mode_cfm,
                                                        0);
        /// Connection Index
        req->conidx = app_hid_env.conidx; 
        /// Status of the request
        req->status = GAP_ERR_NO_ERROR;
        /// HIDS Instance
        req->hid_idx = app_hid_env.conidx;
        /// New Protocol Mode Characteristic Value
        req->proto_mode = param->proto_mode;
        

        // Send the message
        ke_msg_send(req);
    }
    else
    {
        struct hogpd_proto_mode_cfm *req = KE_MSG_ALLOC_DYN(HOGPD_PROTO_MODE_CFM,
                                                        prf_get_task_from_id(TASK_ID_HOGPD),/* src_id */
                                                        TASK_APP,
                                                        hogpd_proto_mode_cfm,
                                                        0);
        /// Status of the request
        req->status = ATT_ERR_APP_ERROR;

        /// Connection Index
        req->conidx = app_hid_env.conidx;
        /// HIDS Instance
        req->hid_idx = app_hid_env.conidx;
        /// New Protocol Mode Characteristic Value
        req->proto_mode = param->proto_mode;
        
        // Send the message
        ke_msg_send(req);
    }
    return (KE_MSG_CONSUMED);
}


static int hogpd_report_upd_handler(ke_msg_id_t const msgid,
                                   struct hogpd_report_upd_rsp const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    NS_LOG_DEBUG("%s,status:%x \r\n",__func__,param->status);
    if (app_hid_env.conidx == param->conidx)
    {
        if (GAP_ERR_NO_ERROR == param->status)
        {
            if (app_hid_env.nb_report < APP_HID_NB_SEND_REPORT)
            {
                app_hid_env.nb_report++;
            }


        }
        else
        {
            // we get this message if error occur while sending report
            // most likely - disconnect
            // Go back to the ready state
            app_hid_env.state = APP_HID_IDLE;
            // change mode
            // restart adv
            // Try to restart advertising if needed
//            app_update_adv_state(true);

            //report was not success - need to restart???
        }
    }
    return (KE_MSG_CONSUMED);
}

static int hogpd_enable_rsp_handler(ke_msg_id_t const msgid,
                                     struct hogpd_enable_rsp const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    NS_LOG_DEBUG("%s,idx %x,status %x\r\n",__func__,param->conidx,param->status);
    return (KE_MSG_CONSUMED);
}

/**
 * @brief Function called when the APP_HID_MOUSE_TIMEOUT_TIMER expires.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 */
int app_hid_mouse_timeout_timer_handler(ke_msg_id_t const msgid,void const *param)
{
    app_hid_env.timer_enabled = false;
    NS_LOG_DEBUG("%s\r\n",__func__);
    if (app_hid_env.state == APP_HID_READY)
    {
        // Requested connection parameters
        struct gapc_conn_param conn_param;
        // Timer value
        uint16_t timer_val;

        /*
         * Request an update of the connection parameters
         * Requested connection interval: 10ms
         * Latency: 200
         * Supervision Timeout: 5s
         */
        conn_param.intv_min = 8;
        conn_param.intv_max = 8;
        conn_param.latency  = 200;
        conn_param.time_out = 500;
        ns_ble_update_param(&conn_param);

        // Go to the Wait for Report state
        app_hid_env.state = APP_HID_WAIT_REP;

        timer_val = APP_HID_SILENCE_DURATION_2;

        // Relaunch the timer
        ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, timer_val);
        app_hid_env.timer_enabled = true;
    }
    else if (app_hid_env.state == APP_HID_WAIT_REP)
    {
      // Disconnect the link with the device
        ns_ble_disconnect();


        // Go back to the ready state
        app_hid_env.state = APP_HID_IDLE;
    }

    return (KE_MSG_CONSUMED);
}





/**
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 */
static int app_hid_msg_dflt_handler(ke_msg_id_t const msgid,
                                    void const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Drop the message
    NS_LOG_DEBUG("%s\r\n",__func__);
    return (KE_MSG_CONSUMED);
}

/**
 * @brief Set the value of the Report Map Characteristic in the database
 */
void app_hid_set_report_map(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 */

/// Default State handlers definition
const struct ke_msg_handler app_hid_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_hid_msg_dflt_handler},

    {HOGPD_ENABLE_RSP,              (ke_msg_func_t)hogpd_enable_rsp_handler},
    {HOGPD_NTF_CFG_IND,             (ke_msg_func_t)hogpd_ntf_cfg_ind_handler},
    {HOGPD_REPORT_REQ_IND,          (ke_msg_func_t)hogpd_report_req_ind_handler},
    {HOGPD_PROTO_MODE_REQ_IND,      (ke_msg_func_t)hogpd_proto_mode_req_ind_handler},
    {HOGPD_CTNL_PT_IND,             (ke_msg_func_t)hogpd_ctnl_pt_ind_handler},
    {HOGPD_REPORT_UPD_RSP,          (ke_msg_func_t)hogpd_report_upd_handler},

};

const struct app_subtask_handlers app_hid_handlers = APP_HANDLERS(app_hid);

/**
 * @brief Send HID report with specific report ID
 * @param report_id The report ID (1-4)
 * @param data Pointer to report data
 * @param len Length of report data
 */
void app_hid_send_report_id(uint8_t report_id, uint8_t* data, uint16_t len)
{
    NS_LOG_INFO("Send Report ID %d, state:%d, nb_report:%d, len:%d\r\n",
                report_id, app_hid_env.state, app_hid_env.nb_report, len);

    // Debug: Print first few bytes of data
    if (len > 0 && data) {
        NS_LOG_DEBUG("Data bytes: %02x %02x %02x %02x\r\n",
                     data[0], data[1], data[2], data[3]);
    }

    switch (app_hid_env.state)
    {
        case (APP_HID_READY):
        {
            // Check if the report can be sent
            if (app_hid_env.nb_report)
            {
                // Allocate the HOGPD_REPORT_UPD_REQ message
                struct hogpd_report_upd_req * req = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                                  prf_get_task_from_id(TASK_ID_HOGPD),
                                                                  TASK_APP,
                                                                  hogpd_report_upd_req,
                                                                  len);

                req->conidx  = app_hid_env.conidx;

                // Fill report info
                req->report.hid_idx  = app_hid_env.conidx;
                req->report.type     = HOGPD_REPORT;
                req->report.idx      = report_id - 1;  // Report index (0-based)
                req->report.length   = len;

                // Copy report data
                memcpy(&req->report.value[0], data, len);

                ke_msg_send(req);

                app_hid_env.nb_report--;

                // Restart the timeout timer if needed
                if (app_hid_env.timeout != 0)
                {
                    ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                    app_hid_env.timer_enabled = true;
                }
            }
            else
            {
                NS_LOG_WARNING("No reports available to send\r\n");
            }
        } break;

        case (APP_HID_WAIT_REP):
        {
            // Requested connection parameters
            struct gapc_conn_param conn_param;

            conn_param.intv_min = 8;
            conn_param.intv_max = 8;
            conn_param.latency  = 25;
            conn_param.time_out = 200;

            ns_ble_update_param(&conn_param);

            // Restart the timeout timer if needed
            if (app_hid_env.timeout != 0)
            {
                ke_timer_set(APP_HID_MOUSE_TIMEOUT_TIMER, TASK_APP, (uint16_t)(app_hid_env.timeout));
                app_hid_env.timer_enabled = true;
            }

            // Go back to the ready state
            app_hid_env.state = APP_HID_READY;
        } break;

        case (APP_HID_IDLE):
        {
            NS_LOG_DEBUG("HID in IDLE state, cannot send report\r\n");
        } break;

        default:
        {
            NS_LOG_WARNING("HID in unknown state %d\r\n", app_hid_env.state);
        } break;
    }
}

#endif //(BLE_APP_HID)

/// @} APP
