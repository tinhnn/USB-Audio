#include "usb_audio_thread.h"
#include "nami_audio.h"

/* USB Thread entry function */
void usb_audio_thread_entry(void) {
	/* TODO: add your own code here */
	audio_init();
	while (1) {
		audio_loop();
		tx_thread_sleep(1);
	}
}
