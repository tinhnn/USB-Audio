
#ifndef NAMI_AUDIO_H_
#define NAMI_AUDIO_H_

/******************************************************************************
 Macro definitions
 ******************************************************************************/

/* USBx device configuration settings */
#define DEVICE_FRAME_LENGTH_HIGH_SPEED              (103U)
#define DEVICE_FRAME_LENGTH_FULL_SPEED              (93U)
#define STRING_FRAMEWORK_LENGTH                     (93U)
#define LANGUAGE_ID_FRAME_WORK_LENGTH               (2U)
#define CONFIG_NUMB                                 (1U)
#define INTERFACE_NUMB0                             (0x00)
#define INTERFACE_NUMB1                             (0x01)
#define BYTE_SIZE                                   (4U)
#define DATA_LEN                                    (2048U)
#define WRITE_DATA_LEN                              (62U)
#define MAX_PACKET_SIZE_HS                          (512U)
#define MAX_PACKET_SIZE_FS                          (64U)

/******************************************************************************
    Macro definitions
 ******************************************************************************/
#define INDEX_0                     (0)
#define INDEX_1                     (1)
#define VALUE_0                     (0)
#define VALUE_1                     (1)
#define VALUE_2                     (2)
#define VALUE_4                     (4)
#define VALUE_93                    (93)
#define VALUE_208                   (208)
#define VALUE_226                   (226)
#define VALUE_247                   (247)
#define VALUE_255                   (0xFF)
#define VALUE_275                   (275)
#define MIC_BUFF_SIZE               (1 * 192)

#define MEMPOOL_SIZE                (18432 * 2)
#define STACK_SIZE                  (1024U)
#define NUM_OF_FRAME                (8U)

#define USB_APL_ON                  (1U)
#define USB_APL_OFF                 (0U)
#define USB_MAX_PACKET_SIZE_IN      (200U)
#define USB_MAX_PACKET_SZIE_OUT     (192U)
#define USB_APL_DETACH              (0)
#define USB_APL_DEFAULT             (1)
#define USB_APL_CONFIGURED          (2)
#define USB_APL_SUSPEND             (3)
#define USB_APL_RESUME              (4)
#define USB_PLUG_IN                 (1UL << 0)
#define USB_PLUG_OUT                (1UL << 1)
#define USB_PLUG_IN_CLEAR           (~(1UL << 0))
#define USB_PLUG_OUT_CLEAR          (~(1UL << 1))

#define A20_CS_ID                       (0x10)
#define A20_SAMPLING_FREQUENCY          (48000)       //hz
#define A20_FU_ID_0                     (5)
#define A20_FU_ID_1                     (8)
#define A20_MUTE                        (0)
#define A20_VOL_MIN_0                   (0)
#define A20_VOL_MAX_0                   (100)
#define A20_SET_VOL_0                   (50)
#define A20_VOL_MIN_1                   (0)
#define A20_VOL_MAX_1                   (200)
#define A20_SET_VOL_1                   (100)

#define A10_FU_ID_0                     (5)
#define A10_FU_ID_1                     (8)
#define A10_MUTE_0                      (0)
#define A10_MUTE_1                      (0x10)
#define A10_VOL_0                       (0)
#define A10_VOL_MIN_0                   (0)
#define A10_VOL_MAX_0                   (0x80)
#define A10_VOL_RES_0                   (0x40)
#define A10_VOL_1                       (0)
#define A10_VOL_MIN_1                   (0)
#define A10_VOL_MAX_1                   (0xF0)
#define A10_VOL_RES_1                   (0x80)

/* APL_AUDIO_20
 * macro when enabled uses the HIGH SPEED Interface
 * on EK-RA6M3 and EK-RA6M5
 *
 * macro when disabled uses the FULL SPEED Interface
 * on EK-RA4M2, EK-RA4M3, EK-RA6M1, EK-RA6M2, EK-RA6M3, EK-RA6M4, EK-RA6M5 */
#define APL_AUDIO_10

void audio_init();
void audio_loop();

#endif /* NAMI_AUDIO_H_ */
