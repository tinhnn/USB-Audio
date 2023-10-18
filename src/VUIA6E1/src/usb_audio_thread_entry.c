#include <nami_audio/nami_audio.h>
#include "usb_audio_thread.h"
#include "r_usb_basic.h"
#include "r_usb_basic_cfg.h"
#include "ux_api.h"
#include "ux_system.h"
#include "ux_device_class_audio.h"
#include "ux_device_class_audio10.h"
#include "nami_audio/nami_RingBuffer.h"

#define RESET_VALUE             (0x00)
#define RX_BUFF_SIZE    20*192
#define DMIC_BUFF_SIZE  10*192
#define AMIC_BUFF_SIZE  10*192

extern uint8_t g_device_framework_full_speed[];
extern uint8_t g_device_framework_hi_speed[];
extern uint8_t g_string_framework[];
extern uint8_t g_language_id_framework[];

struct ringbuffer rb_rx;
struct ringbuffer rb_dmic;
struct ringbuffer rb_amic;
unsigned char RX_BUFF[RX_BUFF_SIZE];
unsigned char DMIC_BUFF[DMIC_BUFF_SIZE];
unsigned char AMIC_BUFF[AMIC_BUFF_SIZE];
bool isStream = false;

static volatile uint32_t g_read_alternate_setting   = USB_APL_OFF;
static volatile uint32_t g_write_alternate_setting  = USB_APL_OFF;
static volatile uint8_t  g_read_wp                  = RESET_VALUE;
static volatile uint32_t g_read_frame_num           = RESET_VALUE;
static volatile uint32_t g_read_counter             = RESET_VALUE;
static volatile uint32_t g_write_counter            = RESET_VALUE;
static volatile uint32_t g_length                   = RESET_VALUE;
static UX_DEVICE_CLASS_AUDIO * volatile g_p_audio   = UX_NULL;
static ULONG  actual_flags                          = RESET_VALUE;
volatile ULONG data_length_playback, real_byte_count= RESET_VALUE;
static UINT g_flag                                  = RESET_VALUE;
static uint32_t g_counter                           = VALUE_0;

static int16_t  g_control_volume[VALUE_2];
static uint16_t g_control_mute[VALUE_2];
static uint32_t g_ux_pool_memory[MEMPOOL_SIZE / VALUE_4];

static UX_DEVICE_CLASS_AUDIO10_CONTROL_GROUP g_audio_control_group;
static UX_DEVICE_CLASS_AUDIO10_CONTROL       g_audio_control[VALUE_2];
static UINT apl_audio10_request_process (UX_DEVICE_CLASS_AUDIO * p_audio, UX_SLAVE_TRANSFER * p_transfer);

/******************************************************************************
 * Local Function
 ******************************************************************************/
static void audio_init_params();
static UINT apl_status_change_cb (ULONG status);
static void apl_audio_read_change (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG alternate_setting);
static void apl_audio_read_done (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG actual_length);
static void apl_audio_write_change (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG alternate_setting);
static void apl_audio_write_done (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG actual_length);
static void apl_audio_instance_activate (void * p_instance);
static void apl_audio_instance_deactivate (void * p_instance);

/* USB Thread entry function */
void usb_audio_thread_entry(void)
{
    UINT status                 = UX_SUCCESS;

    audio_init_params();
    ringbuffer_init(&rb_rx, RX_BUFF, RX_BUFF_SIZE);
    ringbuffer_init(&rb_dmic, DMIC_BUFF, DMIC_BUFF_SIZE);
    ringbuffer_init(&rb_amic, AMIC_BUFF, AMIC_BUFF_SIZE);

    /* DAC Init =============================================================*/
    // Open DAC peripheral
    status = R_DAC_Open(&g_dac0_ctrl, &g_dac0_cfg);
    if(FSP_SUCCESS != status){
        __BKPT(0);
    }

    // Enable DAC output
    status = R_DAC_Start(&g_dac0_ctrl);
    if (FSP_SUCCESS != status)
    {
        __BKPT(0);
    }

    /* GPT Init =============================================================*/
    status = R_GPT_Open(&g_timer_dac_ctrl, &g_timer_dac_cfg);
    if (FSP_SUCCESS != status)
    {
        __BKPT(0);
    }
    status = R_GPT_Start(&g_timer_dac_ctrl);
    if (FSP_SUCCESS != status)
    {
        __BKPT(0);
    }

    /* USBbasic Init ======================================================= */
    // clear write buffer
    //memset(g_write_buf, VALUE_0, USB_MAX_PACKET_SIZE_IN);

    // Open usb driver
    status = R_USB_Open(&g_basic0_ctrl, &g_basic0_cfg);
    if(FSP_SUCCESS != status){
        __BKPT(0);
    }

    // Event Flags are used to register USB Events such as ATTACH and REMOVED
    // Wait until USB device cable inserted.
    status = tx_event_flags_get (&g_msc_event_flags0, USB_PLUG_IN, TX_AND_CLEAR, &actual_flags, TX_WAIT_FOREVER);

    // Reset the event flag
    actual_flags = RESET_VALUE;

    /* App */
    while (true)
    {
        status = tx_event_flags_get (&g_msc_event_flags0, USB_PLUG_OUT, TX_AND_CLEAR, &actual_flags, TX_NO_WAIT);
        if(USB_PLUG_OUT == actual_flags)
        {
            actual_flags = RESET_VALUE;

            /* Reset read flags */


            /*  Event Flags are used to register USB Events such as ATTACH and REMOVED */
            /*  Wait until USB device cable inserted.*/
            status = tx_event_flags_get (&g_msc_event_flags0, USB_PLUG_IN, TX_AND_CLEAR, &actual_flags, TX_WAIT_FOREVER);
            if(USB_PLUG_IN == actual_flags)
            {
                //
            }
            // Reset the event flag
            actual_flags = RESET_VALUE;
        }
        tx_thread_sleep (1);
    }
}

static void audio_init_params()
{
    UINT status = UX_SUCCESS;
    UX_DEVICE_CLASS_AUDIO_STREAM_PARAMETER audio_stream_parameter[VALUE_2];
    UX_DEVICE_CLASS_AUDIO_PARAMETER        audio_parameter;

    // ux_system_initialization
    status = _ux_system_initialize((CHAR *) g_ux_pool_memory, MEMPOOL_SIZE, UX_NULL, VALUE_0);
    if (UX_SUCCESS != status)
    {
        // log error
    }

    // ux_device stack initialization
    status = _ux_device_stack_initialize(g_device_framework_hi_speed,
                                    VALUE_275,
                                    g_device_framework_full_speed,
                                    VALUE_226,
                                    g_string_framework,
                                    VALUE_93,
                                    g_language_id_framework,
                                    VALUE_2,
                                    apl_status_change_cb);
    if (UX_SUCCESS != status)
    {
        // log error
    }

    // Read parameters and CB Initialization
    audio_stream_parameter[INDEX_0].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_change = apl_audio_read_change;
    audio_stream_parameter[INDEX_0].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_frame_done = apl_audio_read_done;
    audio_stream_parameter[INDEX_0].ux_device_class_audio_stream_parameter_thread_stack_size     = STACK_SIZE;
    audio_stream_parameter[INDEX_0].ux_device_class_audio_stream_parameter_max_frame_buffer_nb   = NUM_OF_FRAME;
    audio_stream_parameter[INDEX_0].ux_device_class_audio_stream_parameter_max_frame_buffer_size = USB_MAX_PACKET_SZIE_OUT;
    audio_stream_parameter[INDEX_0].ux_device_class_audio_stream_parameter_thread_entry          = ux_device_class_audio_read_thread_entry;

    // Write parameters and CB Initialization
    audio_stream_parameter[INDEX_1].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_change = apl_audio_write_change;
    audio_stream_parameter[INDEX_1].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_frame_done = apl_audio_write_done;
    audio_stream_parameter[INDEX_1].ux_device_class_audio_stream_parameter_thread_stack_size     = STACK_SIZE;
    audio_stream_parameter[INDEX_1].ux_device_class_audio_stream_parameter_max_frame_buffer_nb   = NUM_OF_FRAME;
    audio_stream_parameter[INDEX_1].ux_device_class_audio_stream_parameter_max_frame_buffer_size = USB_MAX_PACKET_SIZE_IN;
    audio_stream_parameter[INDEX_1].ux_device_class_audio_stream_parameter_thread_entry          = ux_device_class_audio_write_thread_entry;

    // Activate, deactivate CB Initialization
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_slave_class_audio_instance_activate = apl_audio_instance_activate;
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_slave_class_audio_instance_deactivate = apl_audio_instance_deactivate;
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_device_class_audio_control_process = apl_audio10_request_process;
    audio_parameter.ux_device_class_audio_parameter_streams_nb = VALUE_2;
    audio_parameter.ux_device_class_audio_parameter_streams    = &audio_stream_parameter[INDEX_0];
    // ux_device stack class registration
    status = ux_device_stack_class_register(_ux_system_slave_class_audio_name,
                                            _ux_device_class_audio_entry,
                                            VALUE_1,
                                            VALUE_0,
                                            (void *) &audio_parameter);
    // Audio Class 1.0 parameter settings
    g_audio_control[INDEX_0].ux_device_class_audio10_control_fu_id               = A10_FU_ID_0;
    g_audio_control[INDEX_0].ux_device_class_audio10_control_mute[INDEX_0]       = A10_MUTE_0;
    g_audio_control[INDEX_0].ux_device_class_audio10_control_volume[INDEX_0]     = A10_VOL_0;
    g_audio_control[INDEX_0].ux_device_class_audio10_control_volume_min[INDEX_0] = A10_VOL_MIN_0;
    g_audio_control[INDEX_0].ux_device_class_audio10_control_volume_max[INDEX_0] = A10_VOL_MAX_0;
    g_audio_control[INDEX_0].ux_device_class_audio10_control_volume_res[INDEX_0] = A10_VOL_RES_0;

    g_audio_control[INDEX_1].ux_device_class_audio10_control_fu_id                = A10_FU_ID_1;
    g_audio_control[INDEX_1].ux_device_class_audio10_control_mute[INDEX_0]        = A10_MUTE_1;
    g_audio_control[INDEX_1].ux_device_class_audio10_control_volume[INDEX_0]      = A10_VOL_1;
    g_audio_control[INDEX_1].ux_device_class_audio10_control_volume_min[INDEX_0]  = A10_VOL_MIN_1;
    g_audio_control[INDEX_1].ux_device_class_audio10_control_volume_max[INDEX_0]  = A10_VOL_MAX_1;
    g_audio_control[INDEX_1].ux_device_class_audio10_control_volume_res[INDEX_0]  = A10_VOL_RES_1;

    g_audio_control_group.ux_device_class_audio10_control_group_controls_nb       = VALUE_2;
    g_audio_control_group.ux_device_class_audio10_control_group_controls          = &g_audio_control[INDEX_0];
}

void timer_dac_callback(timer_callback_args_t *p_args)
{
    if(isStream)
    {
        uint32_t buff;
        if(ringbuffer_get(&rb_rx, (uint8_t*)&buff, 4))
        {
        	uint16_t *data = (uint16_t*)(&buff);
			R_DAC_Write(&g_dac0_ctrl, (*data)/16);
        }
    }
    FSP_PARAMETER_NOT_USED(p_args);
}

static UINT apl_status_change_cb (ULONG status)
{
    switch (status)
    {
        case UX_DEVICE_ATTACHED:
        {
            /* Set USB PLUG-IN event.*/
            tx_event_flags_set(&g_msc_event_flags0, USB_PLUG_OUT_CLEAR, TX_AND);
            tx_event_flags_set(&g_msc_event_flags0, USB_PLUG_IN, TX_OR);
            break;
        }

        case UX_DEVICE_REMOVED:
        {
            /* Set USB SUSPENDED event.*/
            tx_event_flags_set(&g_msc_event_flags0, USB_PLUG_IN_CLEAR, TX_AND);
            tx_event_flags_set(&g_msc_event_flags0, USB_PLUG_OUT, TX_OR);
            break;
        }

        default:
        {
            /* do nothing */
        }
    }
    return UX_SUCCESS;
}

/******************************************************************************
 * Function Name   : apl_audio_read_change
 * Description     : Callback function called when switching alternate setting value of OUT transfer
 * Arguments       : UX_DEVICE_CLASS_AUDIO_STREAM * : Pointer to UX_DEVICE_CLASS_AUDIO_STREAM structure
 *                 : ULONG : Alternate Setting Value
 * Return value    : UX_SUCCESS
 ******************************************************************************/
static void apl_audio_read_change (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG alternate_setting)
{
    UINT ux_err               = UX_SUCCESS;
    if (USB_APL_ON == alternate_setting)
    {
        ux_err = ux_device_class_audio_stream_get(g_p_audio, 0, &p_stream);
        if (UX_SUCCESS != ux_err)
        {
            // PRINT_ERR_STR("ux_device_class_audio_stream_get operation failed");
        }
        else
        {
            ux_err = ux_device_class_audio_reception_start(p_stream);
            if (UX_SUCCESS != ux_err)
            {
                // PRINT_ERR_STR("ux_device_class_audio_reception_start operation failed");
            }
        }
    }
    else
    {
        if (USB_APL_ON == g_read_alternate_setting)
        {
            /* Alternate Setting 1 --> 0 */
            /* Resetting counters */
            g_read_frame_num    = RESET_VALUE;
            g_read_wp           = RESET_VALUE;
            g_length            = RESET_VALUE;
            if(g_flag == VALUE_1)
            {
                // PRINT_INFO_STR("USB Read Completed");
                g_flag = RESET_VALUE;
                g_read_counter = VALUE_0;
                g_counter = VALUE_0;
            }
        }
    }

    g_read_alternate_setting = alternate_setting;
}

/******************************************************************************
 * Function Name   : apl_audio_read_done
 * Description     : Callback function called when completing of OUT transfer reception
 * Arguments       : UX_DEVICE_CLASS_AUDIO_STREAM * : Pointer to UX_DEVICE_CLASS_AUDIO_STREAM structure
 *                 : ULONG : Actual Length
 * Return value    : UX_SUCCESS
 ******************************************************************************/
static void apl_audio_read_done (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG actual_length)
{
    UINT    ux_err              = UX_SUCCESS;
    UCHAR * p_buffer            = NULL;
    ULONG   length              = VALUE_0;

    data_length_playback = actual_length;

    if (USB_APL_ON == g_read_alternate_setting)
    {
        ux_err = ux_device_class_audio_read_frame_get(p_stream, &p_buffer, &length);
        if (UX_SUCCESS == ux_err)
        {
            // Decode audio
        	ringbuffer_put_force(&rb_rx, (uint8_t*)p_buffer, (uint16_t)length);
        	isStream = true;

            // Stream to DAC


            ux_err = ux_device_class_audio_read_frame_free(p_stream);
            if (UX_SUCCESS != ux_err)
            {
                printf("ux_device_class_audio_read_frame_free failed: %d", ux_err);
            }
        }
        else
        {
            //PRINT_ERR_STR("ux_device_class_audio_read_frame_get operation failed");
        }
    }
}


/******************************************************************************
 * Function Name   : apl_audio_write_change
 * Description     : Callback function called when switching alternate setting value of IN transfer
 * Arguments       : UX_DEVICE_CLASS_AUDIO_STREAM * : Pointer to UX_DEVICE_CLASS_AUDIO_STREAM structure
 *                 : ULONG : Alternate Setting Value
 * Return value    : UX_SUCCESS
 ******************************************************************************/
static void apl_audio_write_change (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG alternate_setting)
{
#if 0
    UINT ux_err              = UX_SUCCESS;

    if (USB_APL_ON == alternate_setting)
    {
        ux_err = ux_device_class_audio_frame_write(p_stream, g_write_buf, USB_MAX_PACKET_SIZE_IN);
        if (UX_SUCCESS == ux_err)
        {
            ux_err = ux_device_class_audio_transmission_start(p_stream);
            if (UX_SUCCESS != ux_err)
            {
                // PRINT_ERR_STR("ux_device_class_audio_transmission_start operation failed");
            }
        }
        else
        {
            // PRINT_ERR_STR("ux_device_class_audio_frame_write operation failed");
        }
    }
    else
    {
        if (USB_APL_ON == g_write_alternate_setting)
        {
            if(g_flag == VALUE_1)
            {
                // PRINT_INFO_STR("USB Write Completed");
                g_flag = RESET_VALUE;
                g_write_counter = VALUE_0;
            }
        }
    }

    g_write_alternate_setting = alternate_setting;
#else
    FSP_PARAMETER_NOT_USED(p_stream);
    FSP_PARAMETER_NOT_USED(alternate_setting);
#endif
}

/******************************************************************************
 * Function Name   : apl_audio_write_done
 * Description     : Callback function called when completing of IN transfer transmission
 * Arguments       : UX_DEVICE_CLASS_AUDIO_STREAM * : Pointer to UX_DEVICE_CLASS_AUDIO_STREAM structure
 *                 : ULONG : Actual Length
 * Return value    : None
 ******************************************************************************/
static void apl_audio_write_done (UX_DEVICE_CLASS_AUDIO_STREAM * p_stream, ULONG actual_length)
{
#if 0
    static uint32_t write_counter = VALUE_0;
    FSP_PARAMETER_NOT_USED(actual_length);
    UINT ux_err                   = UX_SUCCESS;

    if (USB_APL_ON == g_write_alternate_setting)
    {
        // Send audio from AnalogMic to PC
        ux_err=ux_device_class_audio_frame_write(p_stream, AMIC_BUFF + write_counter, USB_MAX_PACKET_SIZE_IN);
        if (ux_err != UX_SUCCESS)
        {
            // PRINT_ERR_STR("ux_device_class_audio_frame_write is failed");
        }
        else
        {
            write_counter = write_counter + USB_MAX_PACKET_SIZE_IN;
            write_counter %= MIC_BUFF_SIZE ;
            g_write_counter++;
            if(g_write_counter == VALUE_1)
            {
                // PRINT_INFO_STR("USB Write Started");
                g_flag = VALUE_1;
            }
        }
    }
#else
    FSP_PARAMETER_NOT_USED(p_stream);
    FSP_PARAMETER_NOT_USED(actual_length);
#endif
}

static void apl_audio_instance_activate (void * p_instance)
{
    g_p_audio = (UX_DEVICE_CLASS_AUDIO *) p_instance;
}

static void apl_audio_instance_deactivate (void * p_instance)
{
    FSP_PARAMETER_NOT_USED(p_instance);
    g_p_audio = UX_NULL;
}


static UINT apl_audio10_request_process (UX_DEVICE_CLASS_AUDIO * p_audio, UX_SLAVE_TRANSFER * p_transfer)
{
    UINT    ux_err               = UX_SUCCESS;
    uint8_t number               = VALUE_0;

    ux_err = ux_device_class_audio10_control_process(p_audio, p_transfer, &g_audio_control_group);
    if (UX_SUCCESS == ux_err)
    {
        /* Request handled, check changes */
        number = (uint8_t) g_audio_control_group.ux_device_class_audio10_control_group_controls_nb;

        for (uint8_t index = RESET_VALUE; index < number; index++)
        {
            switch (g_audio_control[index].ux_device_class_audio10_control_changed)
            {
                case UX_DEVICE_CLASS_AUDIO10_CONTROL_MUTE_CHANGED:
                {
                    g_control_mute[index] = g_audio_control[index].ux_device_class_audio10_control_mute[INDEX_0];
                    break;
                }

                case UX_DEVICE_CLASS_AUDIO10_CONTROL_VOLUME_CHANGED:
                {
                    g_control_volume[index] = g_audio_control[index].ux_device_class_audio10_control_volume[INDEX_0];
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }
    else
    {
        //PRINT_ERR_STR("ux_device_class_audio10_control_process failed");
    }
    return UX_SUCCESS;
}
