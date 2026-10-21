#include "../../hal/sound_hal.h"

// The ES3C28P speaker (codec on I2S, amp enable IO1) is not wired up yet, so
// the chime is a no-op.
void sound_hal_init(void) {}
void sound_hal_tick(void) {}
void sound_hal_chime(void) {}
void sound_hal_tune(float gain, int volume) { (void)gain; (void)volume; }
