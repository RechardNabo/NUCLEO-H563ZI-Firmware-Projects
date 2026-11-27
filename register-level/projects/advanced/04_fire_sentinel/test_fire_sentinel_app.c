/* test_fire_sentinel_app.c — 3 tests pinning the correlation contract. */
#include "unity.h"
#include "xc.h"
#include "fire_sentinel_app.h"
#include "../../../drivers/devices/kit/kit.h"

static fire_sentinel_app_t app;
static uint8_t flame_pin;

static volatile uint32_t *idr_reg(unsigned port)
{
    switch (port) {
        case 0: return &GPIOA_IDR;
        case 1: return &GPIOB_IDR;
        case 2: return &GPIOC_IDR;
        case 3: return &GPIOD_IDR;
        case 4: return &GPIOE_IDR;
        case 5: return &GPIOF_IDR;
        case 6: return &GPIOG_IDR;
        default: return &GPIOH_IDR;
    }
}

static void drive_flame(bool high)
{
    volatile uint32_t *reg = idr_reg((unsigned)(flame_pin >> 4));
    uint32_t mask = 1UL << (flame_pin & 0x0FU);
    if (high) *reg |= mask; else *reg &= ~mask;
}

/* flame sensor is active-LOW with debounce: flame present = pin LOW,
 * held several samples so the integrator registers it */
static void flame_present(bool on)
{
    int i;
    drive_flame(!on);
    for (i = 0; i < 8; i++) fire_sentinel_app_step(&app);
}

void setUp(void)
{
    mock_sfr_reset();
    stm_adc_init();
    flame_pin = kit_default_pin(KIT_FLAME);
    fire_sentinel_app_init(&app, flame_pin, 0U,
                           kit_default_pin(KIT_BUZZER_ACTIVE),
                           kit_default_pin(KIT_DUAL_LED));
    drive_flame(true);   /* no flame = HIGH */
    ADC1_DR = 1000U;   /* room temperature */
}
void tearDown(void) {}

static void test_flame_alone_at_low_temp_is_warning_not_alarm(void)
{
    flame_present(true);
    TEST_ASSERT_EQUAL(FS_WARNING, fire_sentinel_level(&app));
    /* WARNING means LED only — the buzzer output must stay silent:
     * verified through the level contract since buzzer = (level == ALARM) */
}

static void test_flame_plus_heat_is_full_alarm(void)
{
    flame_present(true);
    ADC1_DR = 3500U;   /* hot */
    fire_sentinel_app_step(&app);
    TEST_ASSERT_EQUAL(FS_ALARM, fire_sentinel_level(&app));
}

static void test_heat_without_flame_is_warning_only(void)
{
    flame_present(false);
    ADC1_DR = 3500U;
    fire_sentinel_app_step(&app);
    TEST_ASSERT_EQUAL(FS_WARNING, fire_sentinel_level(&app));

    /* clearing both returns to CLEAR */
    ADC1_DR = 1000U;
    fire_sentinel_app_step(&app);
    TEST_ASSERT_EQUAL(FS_CLEAR, fire_sentinel_level(&app));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_flame_alone_at_low_temp_is_warning_not_alarm);
    RUN_TEST(test_flame_plus_heat_is_full_alarm);
    RUN_TEST(test_heat_without_flame_is_warning_only);
    return UNITY_END();
}
