/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <app/icd/server/ICDServerConfig.h>
#include <cmsis_os2.h>
#include <event_groups.h>
#include <sl_cmsis_os2_common.h>
#include <wfx_host_events.h>

#ifndef RSI_BLE_ENABLE
#define RSI_BLE_ENABLE (1)
#endif

/*
 * Interface to RSI Sapis
 */

#define WFX_RSI_WLAN_TASK_SZ (1024 + 512 + 256) /* Stack for the WLAN task	 	*/ // TODO: For rs9116
#define WFX_RSI_BUF_SZ (1024 * 10)
#define WFX_RSI_DHCP_POLL_INTERVAL (250) /* Poll interval in ms for DHCP		*/
#define WFX_RSI_NUM_TIMERS (2)           /* Number of RSI timers to alloc	*/

typedef enum
{
    WFX_EVT_STA_CONN,
    WFX_EVT_STA_DISCONN,
    WFX_EVT_AP_START,
    WFX_EVT_AP_STOP,
    WFX_EVT_SCAN, /* This is used as scan result and start */
    WFX_EVT_STA_START_JOIN,
    WFX_EVT_STA_DO_DHCP,
    WFX_EVT_STA_DHCP_DONE,
    WFX_EVT_DHCP_POLL
} WfxEventType_e;

typedef enum
{
    WFX_RSI_ST_DEV_READY       = (1 << 0),
    WFX_RSI_ST_AP_READY        = (1 << 1),
    WFX_RSI_ST_STA_PROVISIONED = (1 << 2),
    WFX_RSI_ST_STA_CONNECTING  = (1 << 3),
    WFX_RSI_ST_STA_CONNECTED   = (1 << 4),
    WFX_RSI_ST_STA_DHCP_DONE   = (1 << 6), /* Requested to do DHCP after conn	*/
    WFX_RSI_ST_STA_MODE        = (1 << 7), /* Enable Station Mode			*/
    WFX_RSI_ST_AP_MODE         = (1 << 8), /* Enable AP Mode			*/
    WFX_RSI_ST_STA_READY       = (WFX_RSI_ST_STA_CONNECTED | WFX_RSI_ST_STA_DHCP_DONE),
    WFX_RSI_ST_STARTED         = (1 << 9),  /* RSI task started			*/
    WFX_RSI_ST_SCANSTARTED     = (1 << 10), /* Scan Started				*/
} WfxStateType_e;

typedef struct WfxEvent_s
{
    WfxEventType_e eventType;
    void * eventData; // event data TODO: confirm needed
} WfxEvent_t;

typedef struct wfx_rsi_s
{
    uint16_t dev_state;
    uint16_t ap_chan; /* The chan our STA is using	*/
    wfx_wifi_provision_t sec;
#ifdef SL_WFX_CONFIG_SCAN
    void (*scan_cb)(wfx_wifi_scan_result_t *);
    char * scan_ssid; /* Which one are we scanning for */
    size_t scan_ssid_length;
#endif
#ifdef SL_WFX_CONFIG_SOFTAP
    sl_wfx_mac_address_t softap_mac;
#endif
    sl_wfx_mac_address_t sta_mac;
    sl_wfx_mac_address_t ap_mac;   /* To which our STA is connected */
    sl_wfx_mac_address_t ap_bssid; /* To which our STA is connected */
    uint16_t join_retries;
    uint8_t ip4_addr[4]; /* Not sure if this is enough */
} WfxRsi_t;

extern WfxRsi_t wfx_rsi;

void sl_matter_wifi_task(void * arg);

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void wfx_ip_changed_notify(int got_ip);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_rsi_reset_count();
int32_t sl_wifi_platform_disconnect();

// TODO : this needs to be extern otherwise we get a linking error. We need to figure out why in the header clean up
// NCP files are including this while being c files
#ifdef __cplusplus
extern "C" {
#endif
sl_status_t sl_matter_wifi_platform_init(void);

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#if SLI_SI91X_MCU_INTERFACE
void sl_si91x_invoke_btn_press_event();
#endif // SLI_SI91X_MCU_INTERFACE
#if SLI_SI917
int32_t wfx_rsi_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state);
#else
int32_t wfx_rsi_power_save();
#endif /* SLI_SI917 */
#endif /* SL_ICD_ENABLED */
#ifdef __cplusplus
}
#endif

/**
 * @brief Posts an event to the Wi-Fi task
 *
 * @param[in] event Event to process. Must be valid ptr
 */
// TODO: Can't be a const & since the ncp builds are still using c files
void sl_matter_wifi_post_event(WfxEvent_t * event);
