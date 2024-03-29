/* generated thread header file - do not edit */
#ifndef USB_AUDIO_THREAD_H_
#define USB_AUDIO_THREAD_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void usb_audio_thread_entry(void);
                #else
extern void usb_audio_thread_entry(void);
#endif
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_dac.h"
#include "r_dac_api.h"
#include "r_dmac.h"
#include "r_transfer_api.h"
#include "r_usb_basic.h"
#include "r_usb_basic_api.h"
#include "ux_api.h"
#include "ux_device_class_audio.h"
#include "ux_device_class_audio10.h"
#include "ux_device_class_audio20.h"
FSP_HEADER
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer_dac;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer_dac_ctrl;
extern const timer_cfg_t g_timer_dac_cfg;

#ifndef timer_dac_callback
void timer_dac_callback(timer_callback_args_t *p_args);
#endif
/** DAC on DAC Instance. */
extern const dac_instance_t g_dac0;

/** Access the DAC instance using these structures when calling API functions directly (::p_api is not used). */
extern dac_instance_ctrl_t g_dac0_ctrl;
extern const dac_cfg_t g_dac0_cfg;
/* Transfer on DMAC Instance. */
extern const transfer_instance_t g_transfer1;

/** Access the DMAC instance using these structures when calling API functions directly (::p_api is not used). */
extern dmac_instance_ctrl_t g_transfer1_ctrl;
extern const transfer_cfg_t g_transfer1_cfg;

#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DMAC Instance. */
extern const transfer_instance_t g_transfer0;

/** Access the DMAC instance using these structures when calling API functions directly (::p_api is not used). */
extern dmac_instance_ctrl_t g_transfer0_ctrl;
extern const transfer_cfg_t g_transfer0_cfg;

#ifndef usb_ip0_d1fifo_callback
void usb_ip0_d1fifo_callback(transfer_callback_args_t *p_args);
#endif
/* Basic on USB Instance. */
extern const usb_instance_t g_basic0;

/** Access the USB instance using these structures when calling API functions directly (::p_api is not used). */
extern usb_instance_ctrl_t g_basic0_ctrl;
extern const usb_cfg_t g_basic0_cfg;

#ifndef NULL
void NULL(void*);
#endif

#if 0 == BSP_CFG_RTOS
#ifndef NULL
void NULL(usb_callback_args_t*);
#endif
#endif

#if 2 == BSP_CFG_RTOS
#ifndef NULL
void NULL(usb_event_info_t *, usb_hdl_t, usb_onoff_t);
#endif
#endif
/** AUD Driver on USB Instance. */
FSP_FOOTER
#endif /* USB_AUDIO_THREAD_H_ */
