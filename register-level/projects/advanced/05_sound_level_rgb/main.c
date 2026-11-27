/* main.c — sound level meter binding. */
#include "sound_level_rgb_app.h"
#include "../../../drivers/devices/kit/kit.h"

#define CLK_HZ     29491200UL
#define TICK_LOOPS 20000UL

int main(void)
{
    static sound_level_rgb_app_t app;
    sound_level_rgb_app_init(&app, 0U, 1U, 2U, 3U, CLK_HZ);
    for (;;) {
        sound_level_rgb_app_step(&app);
        for (volatile uint32_t i = 0; i < TICK_LOOPS; i++) { }
    }
}
